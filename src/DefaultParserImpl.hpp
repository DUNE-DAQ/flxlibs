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

  stats::ParserStats& get_stats();

  // Public functions for re-bind
  std::function<void(const felix::packetformat::chunk& chunk)> process_chunk_func;
  std::function<void(const felix::packetformat::shortchunk& shortchunk)> process_shortchunk_func;
  std::function<void(const felix::packetformat::subchunk& subchunk)> process_subchunk_func;
  std::function<void(const felix::packetformat::block& block)> process_block_func;
  std::function<void(const felix::packetformat::chunk& chunk)> process_chunk_with_error_func;
  std::function<void(const felix::packetformat::subchunk& subchunk)> process_subchunk_with_error_func;
  std::function<void(const felix::packetformat::shortchunk& shortchunk)> process_shortchunk_with_error_func;
  std::function<void(const felix::packetformat::block& block)> process_block_with_error_func;

  // Implementation of ParserOperations: They invoke the functions above
  void chunk_processed(const felix::packetformat::chunk& chunk);
  void shortchunk_processed(const felix::packetformat::shortchunk& shortchunk);
  void subchunk_processed(const felix::packetformat::subchunk& subchunk);
  void block_processed(const felix::packetformat::block& block);
  void chunk_processed_with_error(const felix::packetformat::chunk& chunk);
  void subchunk_processed_with_error(const felix::packetformat::subchunk& subchunk);
  void shortchunk_process_with_error(const felix::packetformat::shortchunk& shortchunk);
  void block_processed_with_error(const felix::packetformat::block& block); 

private:
  // Default/empty implementations: No-op "processing"
  void process_chunk(const felix::packetformat::chunk& /*chunk*/) {}
  void process_shortchunk(const felix::packetformat::shortchunk& /*shortchunk*/) {}
  void process_subchunk(const felix::packetformat::subchunk& /*subchunk*/) {}
  void process_block(const felix::packetformat::block& /*block*/) {}
  void process_chunk_with_error(const felix::packetformat::chunk& /*chunk*/) {}
  void process_subchunk_with_error(const felix::packetformat::subchunk& /*subchunk*/) {}
  void process_shortchunk_with_error(const felix::packetformat::shortchunk& /*shortchunk*/) {}
  void process_block_with_error(const felix::packetformat::block& /*block*/) {} 

  // Statistics
  stats::ParserStats m_stats;

};

} // namespace dunedaq::readout

#endif // FLXLIBS_SRC_LINKPARSERIMPL_HPP_
