/**
 * @file CardWrapper.hpp FELIX's FlxCard library wrapper
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef FLXLIBS_SRC_CARDWRAPPER_HPP_
#define FLXLIBS_SRC_CARDWRAPPER_HPP_

#include "flxcard/FlxCard.h"

#include "readout/ReusableThread.hpp"

#include <nlohmann/json.hpp>

#include <string>
#include <mutex>
#include <atomic>

namespace dunedaq::flxlibs {

class CardWrapper 
{

public:
  /**
   * @brief CardWrapper Constructor
   * @param name Instance name for this CardWrapper instance
   */
  explicit CardWrapper();
  ~CardWrapper();
  CardWrapper(const CardWrapper&) =
    delete; ///< CardWrapper is not copy-constructible
  CardWrapper& operator=(const CardWrapper&) =
    delete; ///< CardWrapper is not copy-assignable
  CardWrapper(CardWrapper&&) =
    delete; ///< CardWrapper is not move-constructible
  CardWrapper& operator=(CardWrapper&&) =
    delete; ///< CardWrapper is not move-assignable

  using data_t = nlohmann::json;
  void init(const data_t& args);
  void configure(const data_t& args);
  void start(const data_t& args);
  void stop(const data_t& args);
  void set_running(bool should_run);

  void set_block_addr_handler(std::function<void(uint64_t)>& handle) {
    m_handle_block_addr = std::bind(handle, std::placeholders::_1);
    m_block_addr_handler_available = true;
  }

private:
  // Card
  void open_card();
  void close_card();

  // DMA
  int allocate_CMEM(uint8_t numa, u_long bsize, u_long* paddr, u_long* vaddr);
  void init_DMA();
  void start_DMA();
  void stop_DMA();
  uint64_t bytes_available();
  void read_current_address();

  // Internals
  std::atomic<bool> m_run_marker;
  bool m_configured;
  uint8_t m_card_id;
  std::string m_card_id_str;
  uint8_t m_card_offset;
  uint8_t m_dma_id;
  uint8_t m_numa_id;
  uint8_t m_num_sources;
  uint8_t m_num_links;
  std::string m_info_str;

  // Card object
  using UniqueFlxCard = std::unique_ptr<FlxCard>;
  UniqueFlxCard m_flx_card;
  std::mutex m_card_mutex;

  // DMA: CMEM
  std::size_t m_dma_memory_size; // size of CMEM (driver) memory to allocate
  int m_cmem_handle;             // handle to the DMA memory block
  uint64_t m_virt_addr;          // virtual address of the DMA memory block
  uint64_t m_phys_addr;          // physical address of the DMA memory block
  uint64_t m_current_addr;       // pointer to the current write position for the card
  unsigned m_read_index;
  u_long m_destination;          // u_long -> FlxCard.h

  // Processor
  inline static const std::string m_dma_processor_name = "flx-dma";
  std::atomic<bool> m_run_lock;
  readout::ReusableThread m_dma_processor;
  std::function<void(uint64_t)> m_handle_block_addr;
  bool m_block_addr_handler_available;
  void process_DMA();

};
  
} // namespace dunedaq::flxlibs

#endif // FLXLIBS_SRC_CARDWRAPPER_HPP_
