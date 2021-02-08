/**
 * @file FelixCardReader.hpp
 */
#ifndef APPFWK_UDAQ_READOUT_FELIXCARDREADER_HPP_
#define APPFWK_UDAQ_READOUT_FELIXCARDREADER_HPP_

#include "appfwk/cmd/Structs.hpp"
#include "appfwk/cmd/Nljs.hpp"

#include "flxlibs/felixcardreader/Structs.hpp"

// From appfwk
#include "appfwk/DAQModule.hpp"
#include "appfwk/DAQSink.hpp"
#include "appfwk/ThreadHelper.hpp"

// From readout
#include "readout/ReusableThread.hpp"
#include "readout/ReadoutTypes.hpp"

// FELIX Software Suite provided
#include "flxcard/FlxCard.h"
#include "packetformat/block_format.hpp"

#include <future>
#include <memory>
#include <string>
#include <vector>

namespace dunedaq::flxlibs {

/**
 * @brief FelixCardReader reads FELIX DMA block pointers
 *
 * @author Roland Sipos
 * @date   2020-2021
 *
 */
class FelixCardReader : public dunedaq::appfwk::DAQModule
{
public:
  /**
   * @brief FelixCardReader Constructor
   * @param name Instance name for this FelixCardReader instance
   */
  explicit FelixCardReader(const std::string& name);

  FelixCardReader(const FelixCardReader&) =
    delete; ///< FelixCardReader is not copy-constructible
  FelixCardReader& operator=(const FelixCardReader&) =
    delete; ///< FelixCardReader is not copy-assignable
  FelixCardReader(FelixCardReader&&) =
    delete; ///< FelixCardReader is not move-constructible
  FelixCardReader& operator=(FelixCardReader&&) =
    delete; ///< FelixCardReader is not move-assignable

  void init(const data_t& args) override;

private:
  // Commands
  void do_configure(const data_t& args);
  void do_start(const data_t& args);
  void do_stop(const data_t& args);

  // Configuration
  bool configured_;
  using conf_count_t = dunedaq::flxlibs::felixcardreader::Count;
  using module_conf_t = dunedaq::flxlibs::felixcardreader::Conf;
  module_conf_t cfg_;    
  std::map<uint8_t, readout::types::UniqueBlockPtrSink> block_ptr_sinks_;

  // Card control
  typedef std::unique_ptr<FlxCard> UniqueFlxCard;
  UniqueFlxCard flx_card_;
  std::mutex card_mutex_;

  // Constants
  static constexpr size_t M_MAX_LINKS_PER_CARD=6;
  static constexpr size_t M_MARGIN_BLOCKS=4;
  static constexpr size_t M_BLOCK_THRESHOLD=256;
  static constexpr size_t M_BLOCK_SIZE=felix::packetformat::BLOCKSIZE; 

  // Internals
  uint8_t card_id_;
  uint8_t card_offset_;
  uint8_t dma_id_;
  uint8_t numa_id_;
  uint8_t num_sources_;
  uint8_t num_links_;
  std::string info_str_;

  // CMEM
  std::size_t dma_memory_size_; // size of CMEM (driver) memory to allocate
  int cmem_handle_;         // handle to the DMA memory block
  uint64_t virt_addr_;      // virtual address of the DMA memory block
  uint64_t phys_addr_;      // physical address of the DMA memory block
  uint64_t current_addr_;   // pointer to the current write position for the card
  unsigned read_index_;
  u_long destination_;      // FlxCard.h

  // Processor
  inline static const std::string dma_processor_name_ = "flx-dma";
  std::atomic<bool> run_lock_{false};
  std::atomic<bool> active_{false};
  readout::ReusableThread dma_processor_;
  void processDMA();

  // Functionalities
  void setRunning(bool running);
  void openCard();
  void closeCard();
  int allocateCMEM(uint8_t numa, u_long bsize, u_long* paddr, u_long* vaddr);
  void initDMA();
  void startDMA();
  void stopDMA();
  uint64_t bytesAvailable();  
  void readCurrentAddress();

};

} // namespace readout

#endif // APPFWK_UDAQ_READOUT_FELIXCARDREADER_HPP_
