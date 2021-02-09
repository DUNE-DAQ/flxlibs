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

// FELIX Software Suite provided
#include "flxcard/FlxCard.h"
#include "packetformat/block_format.hpp"

#include <future>
#include <memory>
#include <string>
#include <vector>

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
  // Commands
  void do_configure(const data_t& args);
  void do_start(const data_t& args);
  void do_stop(const data_t& args);

  // Configuration
  bool m_configured;
  using module_conf_t = dunedaq::flxlibs::felixcardreader::Conf;
  module_conf_t m_cfg;
  
  //std::map<uint8_t, readout::types::UniqueBlockPtrSink> m_block_ptr_sinks;

};

} // namespace dunedaq::flxlibs

#endif // APPFWK_UDAQ_READOUT_FELIXCARDREADER_HPP_
