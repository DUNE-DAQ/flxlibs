// From Module
#include "ElinkHandler.hpp"
#include "FelixIssues.hpp"

#include <ers/ers.h>

// From STD
#include <iomanip>
#include <chrono>

namespace dunedaq {
namespace flxlibs {

ElinkHandler::ElinkHandler()
  : m_run_marker{false}
  , m_configured(false)
  , m_link_id(0)
  , m_link_tag(0)
  , m_parser_impl()
  , m_parser_thread(0)
  , m_stats_thread(0)
{

}

ElinkHandler::~ElinkHandler()
{

}

void 
ElinkHandler::init(const data_t& /*args*/)
{
  m_block_addr_queue = std::make_unique<folly::ProducerConsumerQueue<uint64_t>>(1000000);
  // based on queue name, modify the function members of m_parser_impl
  // create parser
  m_parser = std::make_unique<felix::packetformat::BlockParser<DefaultParserImpl>>( m_parser_impl );
}

void 
ElinkHandler::configure(const data_t& /*args*/)
{ 
  ERS_INFO("Configuring ElinkHandler...");
  if (m_configured) {
    ERS_DEBUG(2, "ElinkHandler is already configured!");
  } else {
    m_link_id = 0;
    m_link_tag = 0;
    //m_parser_thread.set_id(0);
    
    // We will need a way to figure this out from regmap.
    m_parser->configure(4096, true); // unsigned bsize, bool trailer_is_32bit

    m_configured=true;
  }
}

void 
ElinkHandler::start(const data_t& /*args*/)
{
  ERS_INFO("Starting ElinkHandler of link " << m_link_id << "...");
  if (!m_run_marker.load()) {
    set_running(true);
    m_stats_thread.set_work(&ElinkHandler::run_stats, this);
    m_parser_thread.set_work(&ElinkHandler::process_elink, this);
    ERS_DEBUG(2, "Started ElinkHandler of link " << m_link_id << "...");
  } else {
    ERS_INFO("ElinkHandler of link " << m_link_id << " is already running!");
  }
}

void 
ElinkHandler::stop(const data_t& /*args*/)
{
  ERS_INFO("Stopping ElinkHandler of link " << m_link_id << "...");
  if (m_run_marker.load()) {
    set_running(false);
    while (!m_parser_thread.get_readiness()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    while (!m_stats_thread.get_readiness()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ERS_DEBUG(2, "Stopped ElinkHandler of link " << m_link_id << "!");
  } else {
    ERS_INFO("ElinkHandler of link " << m_link_id << " is already stopped!");
  }
}

void
ElinkHandler::set_running(bool should_run)
{
  bool was_running = m_run_marker.exchange(should_run);
  ERS_DEBUG(2, "Active state was toggled from " << was_running << " to " << should_run);
}

bool
ElinkHandler::queue_in_block(uint64_t block_addr) {
  if (m_block_addr_queue->write(block_addr)) { // ok write
    return true; 
  } else { // failed write
    return false;
  }
}

void 
ElinkHandler::process_elink() 
{
  while (m_run_marker.load()) {
    uint64_t block_addr; // NOLINT
    if (m_block_addr_queue->read(block_addr)) { // read success
      const auto* block = const_cast<felix::packetformat::block*>(
        felix::packetformat::block_from_bytes(reinterpret_cast<const char*>(block_addr))
      );
      m_parser->process(block);
    } else { // couldn't read from queue
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } 
  }
}

void
ElinkHandler::run_stats()
{
  int new_short_ctr = 0;
  int new_chunk_ctr = 0;
  int new_subchunk_ctr = 0;
  int new_block_ctr = 0;
  int new_error_short_ctr =0;
  int new_error_chunk_ctr =0;
  int new_error_subchunk_ctr =0;
  int new_error_block_ctr = 0;

  auto& stats = m_parser_impl.get_stats();
  auto t0 = std::chrono::high_resolution_clock::now();
  while(m_run_marker.load()) {
    auto now = std::chrono::high_resolution_clock::now();
    new_short_ctr = stats.short_ctr.exchange(0);
    new_chunk_ctr = stats.chunk_ctr.exchange(0);
    new_subchunk_ctr = stats.subchunk_ctr.exchange(0);
    new_block_ctr = stats.block_ctr.exchange(0);
    new_error_short_ctr =stats.error_short_ctr.exchange(0);
    new_error_chunk_ctr =stats.error_chunk_ctr.exchange(0);
    new_error_subchunk_ctr =stats.error_subchunk_ctr.exchange(0);
    new_error_block_ctr = stats.error_block_ctr.exchange(0);

    double seconds =  std::chrono::duration_cast<std::chrono::microseconds>(now-t0).count()/1000000.;
    ERS_INFO("Parser stats ->"
          << " Blocks: " << new_block_ctr
          << " Block rate: " << new_block_ctr/seconds/1000. << " [kHz]"
          << " Chunks: " << new_chunk_ctr
          << " Chunk rate: " << new_chunk_ctr/seconds/1000. << " [kHz]"
          << " Shorts: " << new_short_ctr
          << " Subchunks:" << new_subchunk_ctr
          << " Error Chunks: " << new_error_chunk_ctr
          << " Error Shorts: " << new_error_short_ctr
          << " Error Subchunks: " << new_error_subchunk_ctr
          << " Error Block: " << new_error_block_ctr);

    for (int i=0; i<50 && m_run_marker.load(); ++i) { // 100 x 100ms = 10s sleeps
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    t0 = now;
  }
}

} // namespace flxlibs
} // namespace dunedaq
