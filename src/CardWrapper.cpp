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

#include "flxcard/FlxException.h"
#include "packetformat/block_format.hpp"

#include <ers/ers.h>

// From STD
#include <iomanip>
#include <chrono>
#include <memory>

namespace dunedaq {
namespace flxlibs {

CardWrapper::CardWrapper()
  : m_run_marker{false}
  , m_run_lock{false}
  , m_dma_processor(0)
  , m_handle_block_addr(nullptr)
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
  if (!m_run_marker.load()) {
    if (!m_block_addr_handler_available) {
      ERS_INFO("Block Address handler is not set! Is it intentional?");
    }
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
  if (m_run_marker.load()) {
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
  bool was_running = m_run_marker.exchange(should_run);
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
CardWrapper::allocate_CMEM(uint8_t numa, u_long bsize, u_long* paddr, u_long* vaddr) // NOLINT
{
  ERS_DEBUG(2, "Allocating CMEM buffer " << m_card_id_str << " dma id:" << std::to_string(m_dma_id));
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
  ERS_DEBUG(2, "InitDMA issued...");
  m_card_mutex.lock();
  m_flx_card->dma_reset();
  ERS_DEBUG(2, "flxCard.dma_reset issued.");
  m_flx_card->soft_reset();
  ERS_DEBUG(2, "flxCard.soft_reset issued.");
  m_card_mutex.unlock();

  m_current_addr = m_phys_addr;
  m_destination = m_phys_addr;
  m_read_index = 0;
  ERS_DEBUG(2, "flxCard initDMA done card[" << m_card_id_str << "]");
}

void 
CardWrapper::start_DMA()
{
  ERS_DEBUG(2, "Issuing flxCard.dma_to_host for card " << m_card_id_str << " dma id:" << std::to_string(m_dma_id));
  m_card_mutex.lock();
  m_flx_card->dma_to_host(m_dma_id, m_phys_addr, m_dma_memory_size, m_dma_wraparound); // FlxCard.h
  m_card_mutex.unlock();
}

void 
CardWrapper::stop_DMA()
{
  ERS_DEBUG(2, "Issuing flxCard.dma_stop for card " << m_card_id_str << " dma id:" << std::to_string(m_dma_id));
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
  ERS_DEBUG(2, "CardWrapper starts processing blocks...");
  while (m_run_marker.load()) {

    // Loop until read address makes sense
    while((m_current_addr < m_phys_addr) || (m_phys_addr + m_dma_memory_size < m_current_addr))
    {
      if (m_run_marker.load()) {
        read_current_address();
        std::this_thread::sleep_for(std::chrono::microseconds(5000)); //cfg.poll_time = 5000
      } else {
        ERS_DEBUG(2, "Stop issued during poll! Returning...");
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
        ERS_DEBUG(2, "Stop issued during poll! Returning...");
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
  ERS_DEBUG(2, "CardWrapper processor thread finished.");
}

} // namespace flxlibs
} // namespace dunedaq
