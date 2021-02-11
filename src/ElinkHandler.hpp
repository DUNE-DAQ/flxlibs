/**
 * @file ElinkHandler.hpp FELIX CR's ELink concept wrapper
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef FLXLIBS_SRC_ELINKHANDLER_HPP_
#define FLXLIBS_SRC_ELINKHANDLER_HPP_

#include "DefaultParserImpl.hpp"

#include "readout/ReusableThread.hpp"

#include "packetformat/detail/block_parser.hpp"

#include <nlohmann/json.hpp>
#include <folly/ProducerConsumerQueue.h>

#include <string>
#include <mutex>
#include <atomic>

namespace dunedaq::flxlibs {

class ElinkHandler 
{

public:
  /**
   * @brief ElinkHandler Constructor
   * @param name Instance name for this ElinkHandler instance
   */
  explicit ElinkHandler();
  ~ElinkHandler();
  ElinkHandler(const ElinkHandler&) =
    delete; ///< ElinkHandler is not copy-constructible
  ElinkHandler& operator=(const ElinkHandler&) =
    delete; ///< ElinkHandler is not copy-assignable
  ElinkHandler(ElinkHandler&&) =
    delete; ///< ElinkHandler is not move-constructible
  ElinkHandler& operator=(ElinkHandler&&) =
    delete; ///< ElinkHandler is not move-assignable

  using data_t = nlohmann::json;
  void init(const data_t& args);
  void configure(const data_t& args);
  void start(const data_t& args);
  void stop(const data_t& args);
  void set_running(bool should_run);

  bool queue_in_block(uint64_t block_addr);

  DefaultParserImpl& get_parser() { return std::ref(m_parser_impl); }
 
  void set_ids(unsigned id, unsigned tag) {
    m_link_id = id;
    m_link_tag = tag;
  }

private:
  // Internals
  std::atomic<bool> m_run_marker;
  bool m_configured;
  unsigned m_link_id;
  unsigned m_link_tag;

  // blocks to process
  using UniqueBlockAddrQueue = std::unique_ptr<folly::ProducerConsumerQueue<uint64_t>>;
  UniqueBlockAddrQueue m_block_addr_queue;

  // Block Parser
  DefaultParserImpl m_parser_impl;
  std::unique_ptr<felix::packetformat::BlockParser<DefaultParserImpl>> m_parser;

  // Processor
  inline static const std::string m_parser_thread_name = "elinkp";
  readout::ReusableThread m_parser_thread;
  void process_elink();

  // Statistics
  readout::ReusableThread m_stats_thread;
  void run_stats();
};
  
} // namespace dunedaq::flxlibs

#endif // FLXLIBS_SRC_ELINKHANDLER_HPP_
