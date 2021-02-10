/**
 * @file ElinkHandler.hpp FELIX CR's ELink concept wrapper
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef FLXLIBS_SRC_ELINKHANDLER_HPP_
#define FLXLIBS_SRC_ELINKHANDLER_HPP_

#include "readout/ReusableThread.hpp"

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
  void init(const data_t& args) {}
  void configure(const data_t& args) {}
  void start(const data_t& args) {}
  void stop(const data_t& args) {}
  void set_running(bool should_run) {}

  void queue_in_block(uint64_t) {}


  void set_ids(unsigned id, unsigned tag) {
    m_link_id = id;
    m_link_tag = tag;
  }

private:
  unsigned m_link_id = 0;
  unsigned m_link_tag = 0;

  // blocks to process
  folly::ProducerConsumerQueue<uint64_t> m_block_addr_queue;

  // Processor
  /*
  inline static const std::string m_dma_processor_name = "flx-dma";
  std::atomic<bool> m_run_lock;
  readout::ReusableThread m_elink_processor;
  std::function<void(uint64_t)> m_handle_block_addr;
  bool m_block_addr_handler_available;
  void process_elink();
*/

};
  
} // namespace dunedaq::flxlibs

#endif // FLXLIBS_SRC_ELINKHANDLER_HPP_
