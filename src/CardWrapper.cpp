// From Module
#include "CardWrapper.hpp"
#include "CardConstants.hpp"
#include "FelixIssues.hpp"

#include "flxcard/FlxException.h"
#include "packetformat/block_format.hpp"

#include <ers/ers.h>

// From STD
#include <iomanip>
#include <chrono>

namespace dunedaq {
namespace flxlibs {

CardWrapper::CardWrapper()
  : m_active{false}
  , m_configured(false)
  , m_run_lock{false}
  , m_dma_processor(0)
{
  m_flx_card = std::make_unique<FlxCard>();
}

CardWrapper::~CardWrapper()
{

}

void 
CardWrapper::init(const data_t& /*args*/)
{  
  m_dma_memory_size = 4096*1024*1024UL;
  m_card_id = 0; //init_data.get<uint8_t>("card_id", 0);
  m_card_id_str = std::to_string(m_card_id);
  m_card_offset = 0; //get_config().value<uint8_t>("card_offset", 0);
  m_dma_id = 0; //get_config().value<uint8_t>("dma_id", 0);
  m_numa_id = 0; //get_config().value<uint8_t>("numa_id", 0);
  m_num_sources = 1; //get_config().value<uint8_t>("num_sources", 1);
  m_num_links = 6; //get_config().value<uint8_t>("num_links", M_MAX_LINKS_PER_CARD);
}

void 
CardWrapper::configure(const data_t& /*args*/)
{
  ERS_INFO("Configuring CardWrapper of card " << m_card_id_str);
  if (m_configured) {
    ERS_DEBUG(2, "Card is already configured! Won't touch it.");
  } else {
    // Open card
    open_card();
    ERS_DEBUG(2, "Card[" << m_card_id_str << "] opened.");
    // Allocate CMEM
    m_cmem_handle = allocate_CMEM(m_numa_id, m_dma_memory_size, &m_phys_addr, &m_virt_addr);  
    ERS_DEBUG(2, "Card[" << m_card_id_str << "] CMEM memory allocated with " 
              << std::to_string(m_dma_memory_size) << " Bytes.");
    // Stop currently running DMA
    stop_DMA();
    ERS_DEBUG(2, "Card[" << m_card_id_str << "] DMA interactions force stopped.");
    // Init DMA between software and card
    init_DMA();
    ERS_DEBUG(2, "Card[" << m_card_id_str << "] DMA access initialized.");
    // The rest was some CPU pinning.
    ERS_DEBUG(2, "Card[" << m_card_id_str << "] is configured for datataking.");
    m_configured=true;
  }

}

void 
CardWrapper::start(const data_t& /*args*/)
{
  ERS_INFO("Starting CardWrapper of card " << m_card_id_str << "...");
  if (!m_active.load()) {
    start_DMA();
    set_running(true);
    m_dma_processor.set_work(&CardWrapper::process_DMA, this);
    ERS_DEBUG(2, "Started CardWrapper of card " << m_card_id_str << "...");
  } else {
    ERS_INFO("CardWrapper of card " << m_card_id_str << " is already running!");
  }
}

void 
CardWrapper::stop(const data_t& /*args*/)
{
  ERS_INFO("Stopping CardWrapper of card " << m_card_id_str << "...");
  if (m_active.load()) {
    stop_DMA();
    set_running(false);
    while (!m_dma_processor.get_readiness()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ERS_DEBUG(2, "Stopped CardWrapper of card " << m_card_id_str << "!");
  } else {
    ERS_INFO("CardWrapper of card " << m_card_id_str << " is already stopped!");
  }
}

void
CardWrapper::set_running(bool should_run)
{
  bool was_running = m_active.exchange(should_run);
  ERS_DEBUG(2, "Active state was toggled from " << was_running << " to " << should_run);
}

void 
CardWrapper::open_card()
{
  ERS_DEBUG(2, "Opening FELIX card " << m_card_id_str);
  try {
    m_card_mutex.lock();
    m_flx_card->card_open(static_cast<int>(m_card_id), LOCK_NONE); // FlxCard.h
    m_card_mutex.unlock();
  }
  catch(FlxException& ex) {
    ers::error(flxlibs::CardError(ERS_HERE, ex.what()));
    exit(EXIT_FAILURE);
  }
}

void 
CardWrapper::close_card()
{
  ERS_DEBUG(2, "Closing FELIX card " << m_card_id_str);
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
CardWrapper::allocate_CMEM(uint8_t numa, u_long bsize, u_long* paddr, u_long* vaddr)
{
  ERS_DEBUG(2, "Allocating CMEM buffer " << m_card_id_str << " dma id:" << m_dma_id);
  int handle;
  unsigned ret = CMEM_Open(); // cmem_rcc.h
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
    ers::fatal(flxlibs::CardError(ERS_HERE, 
      "Not enough CMEM memory allocated or the application demands too much CMEM memory.\n"
      "Fix the CMEM memory reservation in the driver or change the module's configuration."));
    exit(EXIT_FAILURE);
  }
  return handle;
}

void 
CardWrapper::init_DMA()
{
  ERS_DEBUG(2, "InitDMA issued...");
  m_card_mutex.lock();

  //m_flx_card->irq_disable();
  //std::cout <<"flxCard.irq_disable issued.");

  m_flx_card->dma_reset();
  ERS_DEBUG(2, "flxCard.dma_reset issued.");

  m_flx_card->soft_reset();
  ERS_DEBUG(2, "flxCard.soft_reset issued.");

  //flxCard.irq_disable(ALL_IRQS);
  //DEBUG("flxCard.irq_diable(ALL_IRQS) issued.");

  // RS TODO: Investigate when did fifo_flush vanish?
  //m_flx_card->dma_fifo_flush();
  //DEBUG("flxCard.dma_fifo_flush issued.");

  //m_flx_card->irq_enable(IRQ_DATA_AVAILABLE);
  //DEBUG("flxCard.irq_enable(IRQ_DATA_AVAILABLE) issued.");
  m_card_mutex.unlock();

  m_current_addr = m_phys_addr;
  m_destination = m_phys_addr;
  m_read_index = 0;
  ERS_DEBUG(2, "flxCard initDMA done card[" << m_card_id_str << "]");
}

void 
CardWrapper::start_DMA()
{
  ERS_DEBUG(2, "Issuing flxCard.dma_to_host for card " << m_card_id_str << " dma id:" << m_dma_id);
  m_card_mutex.lock();
  //m_flx_card->dma_to_host(m_dma_id, m_phys_addr, m_dma_memory_size, FLX_DMA_WRAPAROUND); // FlxCard.h
  m_flx_card->dma_to_host(m_dma_id, m_phys_addr, m_dma_memory_size, constant::dma_wraparound); // FlxCard.h
  m_card_mutex.unlock();
}

void 
CardWrapper::stop_DMA()
{
  ERS_DEBUG(2, "Issuing flxCard.dma_stop for card " << m_card_id_str << " dma id:" << m_dma_id);
  m_card_mutex.lock();
  m_flx_card->dma_stop(m_dma_id);
  m_card_mutex.unlock();
}

inline
uint64_t 
CardWrapper::bytes_available()
{
  return (m_current_addr - ((m_read_index * constant::block_size) + m_phys_addr) + m_dma_memory_size)
          % m_dma_memory_size;
}

void 
CardWrapper::read_current_address()
{
  m_card_mutex.lock();
  m_current_addr = m_flx_card->m_bar0->DMA_DESC_STATUS[m_dma_id].current_address;
  m_card_mutex.unlock();
}

void
CardWrapper::process_DMA()
{
  ERS_DEBUG(2, "CardWrapper starts processing blocks...");
  while (m_active) {

    // Loop until read address makes sense
    while((m_current_addr < m_phys_addr) || (m_phys_addr+m_dma_memory_size < m_current_addr))
    {
      if (m_active.load()) {
        read_current_address();
        std::this_thread::sleep_for(std::chrono::microseconds(5000)); //cfg.poll_time = 5000
      } else {
        ERS_DEBUG(2, "Stop issued during poll! Returning...");
        return;
      }
    }
    // Loop while there are not enough data
    while (bytes_available() < constant::block_threshold * constant::block_size)
    {
      if (m_active.load()) {
        std::this_thread::sleep_for(std::chrono::microseconds(5000)); // cfg.poll_time = 5000
        read_current_address();
      } else {
        ERS_DEBUG(2, "Stop issued during poll! Returning...");
        return;
      }
    }

    // Set write index and start DMA advancing
    u_long write_index = (m_current_addr - m_phys_addr) / constant::block_size;
    uint64_t bytes = 0;
    while (m_read_index != write_index) {
      uint64_t from_address = m_virt_addr + (m_read_index * constant::block_size);

      // Interpret block
      const felix::packetformat::block* block = const_cast<felix::packetformat::block*>(
        felix::packetformat::block_from_bytes(reinterpret_cast<const char*>(from_address))
      );

      // Get ELink ID
      unsigned block_elink_to_id = static_cast<unsigned>(block->elink)/64;
      //ERS_INFO("BLOCK ELINK: " << block_elink);

#warning RS: Add parser implementation
      // Queue block pointer for processing
      //block_ptr_sinks_[block_elink_to_id]->push(from_address);

      //if ( !block_ptr_queues_[block_elink_to_id]->push(from_address) ) {
      //  elink_metrics_[block_elink]++;
      //  ERROR("Could not queue in block for elink[" << block_elink << "]");
      //}

      // Advance
      m_read_index = (m_read_index + 1) % (m_dma_memory_size / constant::block_size);
      bytes += constant::block_size; 
    }      

    // here check if we can move the read pointer in the circular buffer
    m_destination = m_phys_addr + (write_index * constant::block_size) - (constant::margin_blocks * constant::block_size);
    if (m_destination < m_phys_addr) {
      m_destination += m_dma_memory_size;
    }

    // Finally, set new pointer
    m_card_mutex.lock();
    m_flx_card->dma_set_ptr(m_dma_id, m_destination);
    m_card_mutex.unlock();

  }
  ERS_DEBUG(2, "CardWrapper processor thread finished.");
}

} // namespace flxlibs
} // namespace dunedaq
