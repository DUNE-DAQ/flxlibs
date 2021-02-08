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

#include "CardConstants.hpp"

#include <string>

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

  void init(const data_t& args);
  void configure(const data_t& args);
  void start(const data_t& args);
  void stop(const data_t& args);
  void setRunning(bool shouldRun);

  // Card
  void openCard();
  void closeCard();

  // DMA
  int allocateCMEM(uint8_t numa, u_long bsize, u_long* paddr, u_long* vaddr);
  void initDMA();
  void startDMA();
  void stopDMA();
  uint64_t bytesAvailable();
  void readCurrentAddress();

private:

  // Card object
  using UniqueFlxCard = std::unique_ptr<FlxCard>;
  UniqueFlxCard m_flx_card;
  std::mutex m_card_mutex;
  std::atomic<bool> m_active;

  // Internals
  uint8_t m_card_id;
  uint8_t m_card_offset;
  uint8_t m_dma_id;
  uint8_t m_numa_id;
  uint8_t m_num_sources;
  uint8_t m_num_links;
  std::string m_info_str;

  // DMA: CMEM
  std::size_t m_dma_memory_size; // size of CMEM (driver) memory to allocate
  int m_cmem_handle;         // handle to the DMA memory block
  uint64_t m_virt_addr;      // virtual address of the DMA memory block
  uint64_t m_phys_addr;      // physical address of the DMA memory block
  uint64_t m_current_addr;   // pointer to the current write position for the card
  unsigned m_read_index;
  u_long m_destination;      // FlxCard.h

};
  
} // namespace dunedaq::flxlibs

#endif // FLXLIBS_SRC_CARDWRAPPER_HPP_
