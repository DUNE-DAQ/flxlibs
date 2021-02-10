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
#include "packetformat/block_format.hpp"

#include "CardWrapper.hpp"
#include "ElinkHandler.hpp"

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

  int m_card_id = 0;
  int m_num_links = 0;

  // FELIX Cards
  std::unique_ptr<CardWrapper> m_card_wrapper;

  // ElinkHandler
  std::map<int, std::unique_ptr<ElinkHandler>> m_elink_handlers;

  // Function for routing block addresses from card to elink handler
  std::function<void(uint64_t)> m_block_router;

  // Appfwk sinks


};

} // namespace dunedaq::flxlibs

#endif // APPFWK_UDAQ_READOUT_FELIXCARDREADER_HPP_
