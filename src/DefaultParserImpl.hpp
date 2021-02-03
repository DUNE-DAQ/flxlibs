/**
 * @file DefaultParserImpl.hpp
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
/*
 * DefaultParserImpl
 * Author: Roland.Sipos@cern.ch
 * Description: Parser callbacks for FELIX related structures
 *   Implements ParserOperations from felix::packetformat
 * Date: May 2020
 * */
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
    link_id_ = id; 
    link_tag_ = tag; 
  }

  //void start() {}
  //void stop() {}
  void log_packet(bool /*is_short*/) { stats_.packet_ctr_++; }

  std::function<void(const felix::packetformat::chunk& chunk)> pre_process_chunk_func = nullptr;
  std::function<void(const felix::packetformat::chunk& chunk)> post_process_chunk_func = nullptr;
  void pre_process_chunk(const felix::packetformat::chunk& chunk);
  void post_process_chunk(const felix::packetformat::chunk& chunk);
  void chunk_processed(const felix::packetformat::chunk& chunk) { 
    pre_process_chunk_func(chunk);
    post_process_chunk_func(chunk);
    stats_.chunk_ctr_++;
  }

  std::function<void(const felix::packetformat::shortchunk& shortchunk)> pre_process_shortchunk_func = nullptr;
  std::function<void(const felix::packetformat::shortchunk& shortchunk)> post_process_shortchunk_func = nullptr;
  void pre_process_shortchunk(const felix::packetformat::shortchunk& shortchunk);
  void post_process_shortchunk(const felix::packetformat::shortchunk& shortchunk);
  void shortchunk_processed(const felix::packetformat::shortchunk& shortchunk) { 
    pre_process_shortchunk_func(shortchunk); 
    post_process_shortchunk_func(shortchunk);
    stats_.short_ctr_++;
  }

  std::function<void(const felix::packetformat::subchunk& subchunk)> pre_process_subchunk_func = nullptr;
  std::function<void(const felix::packetformat::subchunk& subchunk)> post_process_subchunk_func = nullptr;
  void pre_process_subchunk(const felix::packetformat::subchunk& subchunk);
  void post_process_subchunk(const felix::packetformat::subchunk& subchunk);
  void subchunk_processed(const felix::packetformat::subchunk& subchunk) {
    pre_process_subchunk_func(subchunk);
    post_process_subchunk_func(subchunk);
    stats_.subchunk_ctr_++;
  }

  std::function<void(const felix::packetformat::block& block)> pre_process_block_func = nullptr;
  std::function<void(const felix::packetformat::block& block)> post_process_block_func = nullptr;
  void pre_process_block(const felix::packetformat::block& block);
  void post_process_block(const felix::packetformat::block& block);
  void block_processed(const felix::packetformat::block& block) {
    pre_process_block_func(block);
    post_process_block_func(block);
    stats_.block_ctr_++;
  }

  std::function<void(const felix::packetformat::chunk& chunk)> pre_process_chunk_with_error_func = nullptr;
  std::function<void(const felix::packetformat::chunk& chunk)> post_process_chunk_with_error_func = nullptr;
  void pre_process_chunk_with_error(const felix::packetformat::chunk& chunk);
  void post_process_chunk_with_error(const felix::packetformat::chunk& chunk);
  void chunk_processed_with_error(const felix::packetformat::chunk& chunk) {
    pre_process_chunk_with_error_func(chunk);
    post_process_chunk_with_error_func(chunk);
    stats_.error_chunk_ctr_++;
  }

  std::function<void(const felix::packetformat::subchunk& subchunk)> pre_process_subchunk_with_error_func = nullptr;
  std::function<void(const felix::packetformat::subchunk& subchunk)> post_process_subchunk_with_error_func = nullptr;
  void pre_process_subchunk_with_error(const felix::packetformat::subchunk& subchunk);
  void post_process_subchunk_with_error(const felix::packetformat::subchunk& subchunk);
  void subchunk_processed_with_error(const felix::packetformat::subchunk& subchunk) {
    pre_process_subchunk_with_error_func(subchunk);
    post_process_subchunk_with_error_func(subchunk);
    stats_.error_subchunk_ctr_++;
  }

  std::function<void(const felix::packetformat::shortchunk& shortchunk)> pre_process_shortchunk_with_error_func = nullptr;
  std::function<void(const felix::packetformat::shortchunk& shortchunk)> post_process_shortchunk_with_error_func = nullptr;
  void pre_process_shortchunk_with_error(const felix::packetformat::shortchunk& shortchunk);
  void post_process_shortchunk_with_error(const felix::packetformat::shortchunk& shortchunk);
  void shortchunk_process_with_error(const felix::packetformat::shortchunk& shortchunk) {
    pre_process_shortchunk_with_error_func(shortchunk);
    post_process_shortchunk_with_error_func(shortchunk);
    stats_.error_short_ctr_++;
  }

  std::function<void(const felix::packetformat::block& block)> pre_process_block_with_error_func = nullptr;
  std::function<void(const felix::packetformat::block& block)> post_process_block_with_error_func = nullptr;
  void pre_process_block_with_error(const felix::packetformat::block& block);
  void post_process_block_with_error(const felix::packetformat::block& block);
  void block_processed_with_error(const felix::packetformat::block& block) {
    pre_process_block_with_error_func(block);
    post_process_block_with_error_func(block);
    stats_.error_block_ctr_++;
  }

private:
  unsigned link_id_ = 0;
  unsigned link_tag_ = 0;

  // Statistics
  stats::ParserStats stats_;

};

} // namespace dunedaq::readout

#endif // FLXLIBS_SRC_LINKPARSERIMPL_HPP_
