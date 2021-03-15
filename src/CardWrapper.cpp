/**
 * @file CardWrapper.cpp FELIX's FlxCard library wrapper implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
// From Module
#include "CardWrapper.hpp"
#include "FelixIssues.hpp"

#include "logging/Logging.hpp"

#include "flxcard/FlxException.h"
#include "packetformat/block_format.hpp"

// From STD
#include <iomanip>
#include <chrono>
#include <memory>

/**
 * @brief TRACE debug levels used in this source file
 */
enum
{
  TLVL_ENTER_EXIT_METHODS = 5,
  TLVL_WORK_STEPS = 10,
  TLVL_BOOKKEEPING = 15
};

namespace dunedaq {
namespace flxlibs {

CardWrapper::CardWrapper()
  : m_run_marker{false}
  , m_card_id(0)
  , m_logical_unit(0)
  , m_card_id_str("")
  , m_dma_id(0)
  , m_numa_id(0)
  , m_num_links(0)
  , m_info_str("")
  , m_run_lock{false}
  , m_dma_processor(0)
  , m_handle_block_addr(nullptr)
{

}

CardWrapper::~CardWrapper()
{

}

void 
CardWrapper::init(const data_t& /*args*/)
{ 
  m_flx_card = std::make_unique<FlxCard>();
  if (m_flx_card == nullptr) {
    ers::fatal(flxlibs::CardError(ERS_HERE, "Couldn't create FlxCard object."));
  }
}

void 
CardWrapper::configure(const data_t& args)
{
  if (m_configured) {
    TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << "Card is already configured! Won't touch it.";
  } else {
    // Load config
    m_cfg = args.get<felixcardreader::Conf>();
    m_card_id = m_cfg.card_id;
    m_logical_unit = m_cfg.logical_unit;
    m_dma_id = m_cfg.dma_id;
    m_dma_memory_size = m_cfg.dma_memory_size_gb * 1024*1024*1024UL;
    m_numa_id = m_cfg.numa_id;
    m_dma_processor.set_name(m_dma_processor_name, m_card_id);

    std::ostringstream cardoss;
    cardoss << "[id:" << std::to_string(m_card_id) << " slr:" << std::to_string(m_logical_unit) << "]";
    m_card_id_str = cardoss.str();
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Configuring CardWrapper of card " << m_card_id_str;
    // Open card
    open_card();
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Card[" << m_card_id_str << "] opened.";
    // Allocate CMEM
    m_cmem_handle = allocate_CMEM(m_numa_id, m_dma_memory_size, &m_phys_addr, &m_virt_addr);  
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Card[" << m_card_id_str << "] CMEM memory allocated with " 
                                << std::to_string(m_dma_memory_size) << " Bytes.";
    // Stop currently running DMA
    stop_DMA();
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Card[" << m_card_id_str << "] DMA interactions force stopped.";
    // Init DMA between software and card
    init_DMA();
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Card[" << m_card_id_str << "] DMA access initialized.";
    // The rest was some CPU pinning.
    TLOG_DEBUG(TLVL_WORK_STEPS) << m_card_id_str << "] is configured for datataking.";
    m_configured=true;
  }
}

void 
CardWrapper::start(const data_t& /*args*/)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << "Starting CardWrapper of card " << m_card_id_str << "...";
  if (!m_run_marker.load()) {
    if (!m_block_addr_handler_available) {
      TLOG() << "Block Address handler is not set! Is it intentional?";
    }
    start_DMA();
    set_running(true);
    m_dma_processor.set_work(&CardWrapper::process_DMA, this);
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Started CardWrapper of card " << m_card_id_str << "...";
  } else {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "CardWrapper of card " << m_card_id_str << " is already running!";
  }
}

void 
CardWrapper::stop(const data_t& /*args*/)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << "Stopping CardWrapper of card " << m_card_id_str << "...";
  if (m_run_marker.load()) {
    stop_DMA();
    set_running(false);
    while (!m_dma_processor.get_readiness()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    init_DMA();
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Stopped CardWrapper of card " << m_card_id_str << "!";
  } else {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "CardWrapper of card " << m_card_id_str << " is already stopped!";
  }
}

void
CardWrapper::set_running(bool should_run)
{
  bool was_running = m_run_marker.exchange(should_run);
  TLOG_DEBUG(TLVL_WORK_STEPS) << "Active state was toggled from " << was_running << " to " << should_run;
}

void 
CardWrapper::open_card()
{
  TLOG_DEBUG(TLVL_WORK_STEPS) << "Opening FELIX card " << m_card_id_str;
  try {
    m_card_mutex.lock();
    auto absolute_card_id = m_card_id + m_logical_unit;
    m_flx_card->card_open(static_cast<int>(absolute_card_id), LOCK_NONE); // FlxCard.h
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
  TLOG_DEBUG(TLVL_WORK_STEPS) << "Closing FELIX card " << m_card_id_str;
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
CardWrapper::allocate_CMEM(uint8_t numa, u_long bsize, u_long* paddr, u_long* vaddr) // NOLINT
{
  TLOG_DEBUG(TLVL_WORK_STEPS) << "Allocating CMEM buffer " << m_card_id_str << " dma id:" << std::to_string(m_dma_id);
  int handle;
  unsigned ret = CMEM_Open(); // cmem_rcc.h
  if (!ret) {
    ret = CMEM_NumaSegmentAllocate(bsize, numa, const_cast<char*>("FelixRO"), &handle); // NUMA aware
    //ret = CMEM_GFPBPASegmentAllocate(bsize, const_cast<char*>("FelixRO"), &handle); // non NUMA aware
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
  TLOG_DEBUG(TLVL_WORK_STEPS) << "InitDMA issued...";
  m_card_mutex.lock();
  m_flx_card->dma_reset();
  TLOG_DEBUG(TLVL_WORK_STEPS) << "flxCard.dma_reset issued.";
  m_flx_card->soft_reset();
  TLOG_DEBUG(TLVL_WORK_STEPS) << "flxCard.soft_reset issued.";
  m_card_mutex.unlock();

  m_current_addr = m_phys_addr;
  m_destination = m_phys_addr;
  m_read_index = 0;
  TLOG_DEBUG(TLVL_WORK_STEPS) << "flxCard initDMA done card[" << m_card_id_str << "]";
}

void 
CardWrapper::start_DMA()
{
  TLOG_DEBUG(TLVL_WORK_STEPS) << "Issuing flxCard.dma_to_host for card " << m_card_id_str << " dma id:" << std::to_string(m_dma_id);
  m_card_mutex.lock();
  m_flx_card->dma_to_host(m_dma_id, m_phys_addr, m_dma_memory_size, m_dma_wraparound); // FlxCard.h
  m_card_mutex.unlock();
}

void 
CardWrapper::stop_DMA()
{
  TLOG_DEBUG(TLVL_WORK_STEPS) << "Issuing flxCard.dma_stop for card " << m_card_id_str << " dma id:" << std::to_string(m_dma_id);
  m_card_mutex.lock();
  m_flx_card->dma_stop(m_dma_id);
  m_card_mutex.unlock();
}

inline uint64_t // NOLINT
CardWrapper::bytes_available()
{
  return (m_current_addr - ((m_read_index * m_block_size) + m_phys_addr) + m_dma_memory_size)
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
  TLOG_DEBUG(TLVL_WORK_STEPS) << "CardWrapper starts processing blocks...";
  while (m_run_marker.load()) {

    // Loop until read address makes sense
    while((m_current_addr < m_phys_addr) || (m_phys_addr + m_dma_memory_size < m_current_addr))
    {
      if (m_run_marker.load()) {
        read_current_address();
        std::this_thread::sleep_for(std::chrono::microseconds(5000)); //cfg.poll_time = 5000
      } else {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Stop issued during poll! Returning...";
        return;
      }
    }
    // Loop while there are not enough data
    while (bytes_available() < m_block_threshold * m_block_size)
    {
      if (m_run_marker.load()) {
        std::this_thread::sleep_for(std::chrono::microseconds(5000)); // cfg.poll_time = 5000
        read_current_address();
      } else {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Stop issued during poll! Returning...";
        return;
      }
    }

    // Set write index and start DMA advancing
    u_long write_index = (m_current_addr - m_phys_addr) / m_block_size;
    uint64_t bytes = 0; // NOLINT
    while (m_read_index != write_index) {
      uint64_t from_address = m_virt_addr + (m_read_index * m_block_size); // NOLINT

      // Handle block address
      if (m_block_addr_handler_available) {
        m_handle_block_addr(from_address);
      }

      // Advance
      m_read_index = (m_read_index + 1) % (m_dma_memory_size / m_block_size);
      bytes += m_block_size; 
    }      

    // here check if we can move the read pointer in the circular buffer
    m_destination = m_phys_addr + (write_index * m_block_size) - (m_margin_blocks * m_block_size);
    if (m_destination < m_phys_addr) {
      m_destination += m_dma_memory_size;
    }

    // Finally, set new pointer
    m_card_mutex.lock();
    m_flx_card->dma_set_ptr(m_dma_id, m_destination);
    m_card_mutex.unlock();

  }
  TLOG_DEBUG(TLVL_WORK_STEPS) << "CardWrapper processor thread finished.";
}

} // namespace flxlibs
} // namespace dunedaq
