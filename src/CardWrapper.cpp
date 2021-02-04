// From Module
#include "CardWrapper.hpp"

#include <ers/ers.h>

// From STD
#include <iomanip>
#include <chrono>

namespace dunedaq {
namespace flxlibs {

CardWrapper::CardWrapper()
{

}

CardWrapper::~CardWrapper()
{

}

void 
CardWrapper::init(const data_t& args)
{  
  dma_memory_size_ = 4096*1024*1024UL;
  card_id_ = 0; //init_data.get<uint8_t>("card_id", 0);
  card_offset_ = 0; //get_config().value<uint8_t>("card_offset", 0);
  dma_id_ = 0; //get_config().value<uint8_t>("dma_id", 0);
  numa_id_ = 0; //get_config().value<uint8_t>("numa_id", 0);
  num_sources_ = 1; //get_config().value<uint8_t>("num_sources", 1);
  num_links_ = 6; //get_config().value<uint8_t>("num_links", M_MAX_LINKS_PER_CARD);
}

void 
CardWrapper::configure(const data_t& args)
{
  if (configured_) {
    ERS_INFO("Card is already configured! Won't touch it.");
  } else {
    ERS_INFO("Configuring FelixCardReader of card[" << std::to_string(card_id_) << "].");
    // Open card
    openCard();
    ERS_INFO("Card[" << card_id_ << "] opened.");
    // Allocate CMEM
    std::cout << " CMEM sizE: " << dma_memory_size_ << '\n';
    cmem_handle_ = allocateCMEM(numa_id_, dma_memory_size_, &phys_addr_, &virt_addr_);  
    ERS_INFO("Card[" << (unsigned)card_id_ << "] CMEM memory allocated with " << (unsigned)dma_memory_size_ << " Bytes.");
    // Stop currently running DMA
    stopDMA();
    ERS_INFO("Card[" << card_id_ << "] DMA interactions force stopped.");
    // Init DMA between software and card
    initDMA();
    ERS_INFO("Card[" << card_id_ << "] DMA access initialized.");
    // The rest was some CPU pinning.
    ERS_INFO("Card[" << card_id_ << "] is configured for datataking.");
    configured_=true;
  }

}

void 
CardWrapper::start(const data_t& args)
{
  ERS_INFO("Starting CardReader of card " << card_id_ << "...");
  if (!active_.load()) {
    startDMA();
    setRunning(true);
    dma_processor_.set_work(&FelixCardReader::processDMA, this);
    ERS_INFO("Started CardReader of card " << card_id_ << "...");
  } else {
    ERS_INFO("CardReader of card " << card_id_ << " is already running!");
  }
}

void 
CardWrapper::stop(const data_t& args)
{
  ERS_INFO("Stopping CardReader of card " << card_id_ << "...");
  if (active_.load()) {
    stopDMA();
    setRunning(false);
    while (!dma_processor_.get_readiness()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ERS_INFO("Stopped CardReader of card " << card_id_ << "!");
  } else {
    ERS_INFO("CardReader of card " << card_id_ << " is already stopped!");
  }
}

void
FelixCardReader::setRunning(bool shouldRun)
{
  bool wasRunning = m_active.exchange(shouldRun);
  ERS_INFO("Active state was toggled from " << wasRunning << " to " << shouldRun);
}

void 
CardWrapper::openCard()
{
  ERS_INFO("Opening FELIX card " << m_card_id);
  try {
    m_card_mutex.lock();
    m_flx_card->card_open(static_cast<int>(m_card_id), LOCK_NONE);
    m_card_mutex.unlock();
  }
  catch(FlxException& ex) {
    ers::error(flxlibs::CardError(ERS_HERE, ex.what()));
    exit(EXIT_FAILURE);
  }
}

void 
CardWrapper::closeCard()
{
  ERS_INFO("Closing FELIX card " << m_card_id);
  try {
    m_card_mutex.lock();
    m_flx_card->card_close();
    m_card_mutex.unlock();
  }
  catch(FlxException& ex) {
    ers::error(flxlibs::CardError(ERS_HERE, ex.what()));
    exit(EXIT_FAILURE);
  }
}

int 
CardWrapper::allocateCMEM(uint8_t numa, u_long bsize, u_long* paddr, u_long* vaddr)
{
  ERS_INFO("Allocating CMEM buffer " << m_card_id << " dma id:" << m_dma_id);
  int handle;
  unsigned ret = CMEM_Open();
  if (!ret) {
    ret = CMEM_NumaSegmentAllocate(bsize, numa, const_cast<char*>("FelixRO"), &handle);
    //ret = CMEM_GFPBPASegmentAllocate(bsize, const_cast<char*>("FelixRO"), &handle); 
  }
  if (!ret) {
    ret = CMEM_SegmentPhysicalAddress(handle, paddr);
  }
  if (!ret) {
    ret = CMEM_SegmentVirtualAddress(handle, vaddr);
  }
  if (ret) {
    //rcc_error_print(stdout, ret);
    m_card_mutex.lock();
    m_flx_card->card_close();
    m_card_mutex.unlock();
    ers::error(flxlibs::CardError(ERS_HERE, 
      "Not enough CMEM memory allocated or the application demands too much CMEM memory.\n"
      "Fix the CMEM memory reservation in the driver or change the module's configuration."));
    exit(EXIT_FAILURE);
  }
  return handle;
}

void 
CardWrapper::initDMA()
{
  ERS_INFO("InitDMA issued...");
  m_card_mutex.lock();

  //flx_card_->irq_disable();
  //std::cout <<"flxCard.irq_disable issued.");

  m_flx_card->dma_reset();
  ERS_INFO("flxCard.dma_reset issued.");

  m_flx_card->soft_reset();
  ERS_INFO("flxCard.soft_reset issued.");

  //flxCard.irq_disable(ALL_IRQS);
  //DEBUG("flxCard.irq_diable(ALL_IRQS) issued.");

#warning RS: Investigate when did fifo_flush vanished?
  //flx_card_->dma_fifo_flush();
  //DEBUG("flxCard.dma_fifo_flush issued.");

  //flx_card_->irq_enable(IRQ_DATA_AVAILABLE);
  //DEBUG("flxCard.irq_enable(IRQ_DATA_AVAILABLE) issued.");
  m_card_mutex.unlock();

  m_current_addr = m_phys_addr;
  m_destination = m_phys_addr;
  m_read_index = 0;
  ERS_INFO("flxCard initDMA done card[" << m_card_id << "]");
}

void 
CardWrapper::startDMA()
{
  ERS_INFO("Issuing flxCard.dma_to_host for card " << m_card_id << " dma id:" << m_dma_id);
  m_card_mutex.lock();
  //m_flx_card->dma_to_host(m_dma_id, m_phys_addr, m_dma_memory_size, FLX_DMA_WRAPAROUND); // FlxCard.h
  m_flx_card->dma_to_host(m_dma_id, m_phys_addr, m_dma_memory_size, constant::dma_wraparound); // FlxCard.h
  m_card_mutex.unlock();
}

void 
CardWrapper::stopDMA()
{
  ERS_INFO("Issuing flxCard.dma_stop for card " << m_card_id << " dma id:" << m_dma_id);
  m_card_mutex.lock();
  m_flx_card->dma_stop(m_dma_id);
  m_card_mutex.unlock();
}

inline
uint64_t 
CardWrapper::bytesAvailable()
{
  return (m_current_addr - ((m_read_index * constant::block_size) + m_phys_addr) + m_dma_memory_size)
          % m_dma_memory_size;
}

void 
CardWrapper::readCurrentAddress()
{
  m_card_mutex_.lock();
  current_addr_ = flx_card_->m_bar0->DMA_DESC_STATUS[dma_id_].current_address;
  card_mutex_.unlock();
}


} // namespace flxlibs
} // namespace dunedaq
