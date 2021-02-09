/**
 * @file DefaultParserImpl.hpp FELIX's packetformat default block/chunk parser
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef FLXLIBS_SRC_DEFAULTPARSERIMPL_HPP_
#define FLXLIBS_SRC_DEFAULTPARSERIMPL_HPP_

// 3rdparty, external
#include "packetformat/block_format.hpp"
#include "packetformat/block_parser.hpp"

// From Module
//#include "appfwk/DAQSource.hpp"

#include "FelixIssues.hpp"
//#include "ReadoutTypes.hpp"
#include "FelixStatistics.hpp"

// From STD
#include <iomanip>
#include <functional>

namespace dunedaq::flxlibs {

class DefaultParserImpl : public felix::packetformat::ParserOperations
{
public:
  explicit DefaultParserImpl();
  ~DefaultParserImpl();
  DefaultParserImpl(const DefaultParserImpl&) =
    delete; ///< DefaultParserImpl is not copy-constructible
  DefaultParserImpl& operator=(const DefaultParserImpl&) =
    delete; ///< DefaultParserImpl is not copy-assignable
  DefaultParserImpl(DefaultParserImpl&&) =
    delete; ///< DefaultParserImpl is not move-constructible
  DefaultParserImpl& operator=(DefaultParserImpl&&) =
    delete; ///< DefaultParserImpl is not move-assignable

  void set_ids(unsigned id, unsigned tag) { 
    m_link_id = id; 
    m_link_tag = tag; 
  }

  //void start() {}
  //void stop() {}
  void log_packet(bool /*is_short*/) { m_stats.packet_ctr++; }

  std::function<void(const felix::packetformat::chunk& chunk)> process_chunk_func = nullptr;
  void process_chunk(const felix::packetformat::chunk& chunk);
  void chunk_processed(const felix::packetformat::chunk& chunk) { 
    process_chunk_func(chunk);
    m_stats.chunk_ctr++;
  }

  std::function<void(const felix::packetformat::shortchunk& shortchunk)> process_shortchunk_func = nullptr;
  void process_shortchunk(const felix::packetformat::shortchunk& shortchunk);
  void shortchunk_processed(const felix::packetformat::shortchunk& shortchunk) { 
    process_shortchunk_func(shortchunk);
    m_stats.short_ctr++;
  }

  std::function<void(const felix::packetformat::subchunk& subchunk)> process_subchunk_func = nullptr;
  void process_subchunk(const felix::packetformat::subchunk& subchunk);
  void subchunk_processed(const felix::packetformat::subchunk& subchunk) {
    process_subchunk_func(subchunk);
    m_stats.subchunk_ctr++;
  }

  std::function<void(const felix::packetformat::block& block)> process_block_func = nullptr;
  void process_block(const felix::packetformat::block& block);
  void block_processed(const felix::packetformat::block& block) {
    process_block_func(block);
    m_stats.block_ctr++;
  }

  std::function<void(const felix::packetformat::chunk& chunk)> process_chunk_with_error_func = nullptr;
  void process_chunk_with_error(const felix::packetformat::chunk& chunk);
  void chunk_processed_with_error(const felix::packetformat::chunk& chunk) {
    process_chunk_with_error_func(chunk);
    m_stats.error_chunk_ctr++;
  }

  std::function<void(const felix::packetformat::subchunk& subchunk)> process_subchunk_with_error_func = nullptr;
  void process_subchunk_with_error(const felix::packetformat::subchunk& subchunk);
  void subchunk_processed_with_error(const felix::packetformat::subchunk& subchunk) {
    process_subchunk_with_error_func(subchunk);
    m_stats.error_subchunk_ctr++;
  }

  std::function<void(const felix::packetformat::shortchunk& shortchunk)> process_shortchunk_with_error_func = nullptr;
  void process_shortchunk_with_error(const felix::packetformat::shortchunk& shortchunk);
  void shortchunk_process_with_error(const felix::packetformat::shortchunk& shortchunk) {
    process_shortchunk_with_error_func(shortchunk);
    m_stats.error_short_ctr++;
  }

  std::function<void(const felix::packetformat::block& block)> process_block_with_error_func = nullptr;
  void process_block_with_error(const felix::packetformat::block& block);
  void block_processed_with_error(const felix::packetformat::block& block) {
    process_block_with_error_func(block);
    m_stats.error_block_ctr++;
  }

private:
  unsigned m_link_id = 0;
  unsigned m_link_tag = 0;

  // Statistics
  stats::ParserStats m_stats;

};

} // namespace dunedaq::readout

#endif // FLXLIBS_SRC_LINKPARSERIMPL_HPP_
