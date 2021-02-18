/**
 * @file FelixCardReader.hpp FELIX card reader DAQ Module.
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef FLXLIBS_PLUGINS_FELIXCARDREADER_HPP_
#define FLXLIBS_PLUGINS_FELIXCARDREADER_HPP_

#include "appfwk/cmd/Structs.hpp"
#include "appfwk/cmd/Nljs.hpp"

#include "flxlibs/felixcardreader/Structs.hpp"

// From appfwk
#include "appfwk/DAQModule.hpp"
#include "appfwk/DAQSink.hpp"
#include "appfwk/ThreadHelper.hpp"

// From readout
#include "readout/ReusableThread.hpp"

// FELIX Software Suite provided
#include "packetformat/block_format.hpp"

#include "CardWrapper.hpp"
#include "ElinkConcept.hpp"

#include <future>
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace dunedaq::flxlibs {

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
  // Types
  using module_conf_t = dunedaq::flxlibs::felixcardreader::Conf;

  // Constants
  static constexpr int m_elink_multiplier = 64;
  static constexpr size_t m_block_queue_capacity = 1000000;
  static constexpr size_t m_1kb_block_size = 1024;
  static constexpr int m_32b_trailer_size = 32; 

  // Commands
  void do_configure(const data_t& args);
  void do_start(const data_t& args);
  void do_stop(const data_t& args);

  // Configuration
  bool m_configured;
  module_conf_t m_cfg;

  int m_card_id;
  int m_num_links;
  std::size_t m_block_size;
  int m_chunk_trailer_size;

  // FELIX Cards
  std::unique_ptr<CardWrapper> m_card_wrapper;

  // ElinkConcept
  std::map<int, std::unique_ptr<ElinkConcept>> m_elinks;

  // Function for routing block addresses from card to elink handler
  std::function<void(uint64_t)> m_block_router; // NOLINT

};

} // namespace dunedaq::flxlibs

#endif // FLXLIBS_PLUGINS_FELIXCARDREADER_HPP_
