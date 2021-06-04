/**
 * @file ElinkModel.hpp FELIX CR's ELink concept wrapper
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef FLXLIBS_SRC_ELINKMODEL_HPP_
#define FLXLIBS_SRC_ELINKMODEL_HPP_

#include "ElinkConcept.hpp"

#include "appfwk/DAQSink.hpp"
#include "logging/Logging.hpp"
#include "readout/utils/ReusableThread.hpp"

#include <folly/ProducerConsumerQueue.h>
#include <nlohmann/json.hpp>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

namespace dunedaq::flxlibs {

template<class TargetPayloadType>
class ElinkModel : public ElinkConcept
{
public:
  // using sink_t = appfwk::DAQSink<std::unique_ptr<TargetPayloadType>>;
  // using sink_t = appfwk::DAQSink<TargetPayloadType*>;
  using sink_t = appfwk::DAQSink<TargetPayloadType>;
  using inherited = ElinkConcept;
  using data_t = nlohmann::json;

  /**
   * @brief ElinkModel Constructor
   * @param name Instance name for this ElinkModel instance
   */
  ElinkModel()
    : ElinkConcept()
    , m_run_marker{ false }
    , m_parser_thread(0)
    , m_stats_thread(0)
  {}
  ~ElinkModel() {}

  void set_sink(const std::string& sink_name) override
  {
    if (m_sink_is_set) {
      TLOG_DEBUG(5) << "ElinkModel sink is already set in initialized!";
    } else {
      m_sink_queue = std::make_unique<sink_t>(sink_name);
      m_sink_is_set = true;
    }
  }

  std::unique_ptr<sink_t>& get_sink() { return m_sink_queue; }

  void init(const data_t& /*args*/, const size_t block_queue_capacity)
  {
    m_block_addr_queue = std::make_unique<folly::ProducerConsumerQueue<uint64_t>>(block_queue_capacity); // NOLINT
  }

  void conf(const data_t& /*args*/, size_t block_size, bool is_32b_trailers)
  {
    if (m_configured) {
      TLOG_DEBUG(5) << "ElinkModel is already configured!";
    } else {
      m_parser_thread.set_name(inherited::m_elink_source_tid, inherited::m_link_tag);
      // if (inconsistency)
      // ers::fatal(ElinkConfigurationInconsistency(ERS_HERE, m_num_links));

      m_parser->configure(block_size, is_32b_trailers); // unsigned bsize, bool trailer_is_32bit
      m_configured = true;
    }
  }

  void start(const data_t& /*args*/)
  {
    if (!m_run_marker.load()) {
      set_running(true);
      m_stats_thread.set_work(&ElinkModel::run_stats, this);
      m_parser_thread.set_work(&ElinkModel::process_elink, this);
      TLOG_DEBUG(5) << "Started ElinkModel of link " << inherited::m_link_id << "...";
    } else {
      TLOG_DEBUG(5) << "ElinkModel of link " << inherited::m_link_id << " is already running!";
    }
  }

  void stop(const data_t& /*args*/)
  {
    if (m_run_marker.load()) {
      set_running(false);
      while (!m_parser_thread.get_readiness()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      while (!m_stats_thread.get_readiness()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      TLOG_DEBUG(5) << "Stopped ElinkModel of link " << m_link_id << "!";
    } else {
      TLOG_DEBUG(5) << "ElinkModel of link " << m_link_id << " is already stopped!";
    }
  }

  void set_running(bool should_run)
  {
    bool was_running = m_run_marker.exchange(should_run);
    TLOG_DEBUG(5) << "Active state was toggled from " << was_running << " to " << should_run;
  }

  bool queue_in_block_address(uint64_t block_addr) // NOLINT(build/unsigned)
  {
    if (m_block_addr_queue->write(block_addr)) { // ok write
      return true;
    } else { // failed write
      return false;
    }
  }

private:
  // Types
  using UniqueBlockAddrQueue = std::unique_ptr<folly::ProducerConsumerQueue<uint64_t>>; // NOLINT(build/unsigned)

  // Internals
  std::atomic<bool> m_run_marker;
  bool m_configured{ false };

  // Sink
  bool m_sink_is_set{ false };
  std::unique_ptr<sink_t> m_sink_queue;

  // blocks to process
  UniqueBlockAddrQueue m_block_addr_queue;

  // Processor
  inline static const std::string m_parser_thread_name = "elinkp";
  readout::ReusableThread m_parser_thread;
  void process_elink()
  {
    while (m_run_marker.load()) {
      uint64_t block_addr;                        // NOLINT
      if (m_block_addr_queue->read(block_addr)) { // read success
        const auto* block = const_cast<felix::packetformat::block*>(
          felix::packetformat::block_from_bytes(reinterpret_cast<const char*>(block_addr)) // NOLINT
        );
        m_parser->process(block);
      } else { // couldn't read from queue
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    }
  }

  // Statistics
  readout::ReusableThread m_stats_thread;
  void run_stats()
  {
    int new_short_ctr = 0;
    int new_chunk_ctr = 0;
    int new_subchunk_ctr = 0;
    int new_block_ctr = 0;
    int new_error_short_ctr = 0;
    int new_error_chunk_ctr = 0;
    int new_error_subchunk_ctr = 0;
    int new_error_block_ctr = 0;

    auto& stats = m_parser_impl.get_stats();
    auto t0 = std::chrono::high_resolution_clock::now();
    while (m_run_marker.load()) {
      auto now = std::chrono::high_resolution_clock::now();
      new_short_ctr = stats.short_ctr.exchange(0);
      new_chunk_ctr = stats.chunk_ctr.exchange(0);
      new_subchunk_ctr = stats.subchunk_ctr.exchange(0);
      new_block_ctr = stats.block_ctr.exchange(0);
      new_error_short_ctr = stats.error_short_ctr.exchange(0);
      new_error_chunk_ctr = stats.error_chunk_ctr.exchange(0);
      new_error_subchunk_ctr = stats.error_subchunk_ctr.exchange(0);
      new_error_block_ctr = stats.error_block_ctr.exchange(0);

      double seconds = std::chrono::duration_cast<std::chrono::microseconds>(now - t0).count() / 1000000.;
      TLOG_DEBUG(2) << inherited::m_elink_str // Move to TLVL_TAKE_NOTE from readout
                    << " Parser stats ->"
                    << " Blocks: " << new_block_ctr << " Block rate: " << new_block_ctr / seconds / 1000. << " [kHz]"
                    << " Chunks: " << new_chunk_ctr << " Chunk rate: " << new_chunk_ctr / seconds / 1000. << " [kHz]"
                    << " Shorts: " << new_short_ctr << " Subchunks:" << new_subchunk_ctr
                    << " Error Chunks: " << new_error_chunk_ctr << " Error Shorts: " << new_error_short_ctr
                    << " Error Subchunks: " << new_error_subchunk_ctr << " Error Block: " << new_error_block_ctr;

      for (int i = 0; i < 50 && m_run_marker.load(); ++i) { // 100 x 100ms = 10s sleeps
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
      t0 = now;
    }
  }
};

} // namespace dunedaq::flxlibs

#endif // FLXLIBS_SRC_ELINKMODEL_HPP_
