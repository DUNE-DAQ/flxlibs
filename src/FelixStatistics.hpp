/**
 * @file FelixStatistics.hpp Readout system metrics
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
*/
#ifndef FLXLIBS_SRC_FELIXSTATISTICS_HPP_
#define FLXLIBS_SRC_FELIXSTATISTICS_HPP_

#include <atomic>

namespace dunedaq::flxlibs::stats {

typedef std::atomic<int> counter_t;

struct ParserStats {
  counter_t packet_ctr_{0};
  counter_t short_ctr_{0};
  counter_t chunk_ctr_{0};
  counter_t subchunk_ctr_{0};
  counter_t block_ctr_{0};
  counter_t error_short_ctr_{0};
  counter_t error_chunk_ctr_{0};
  counter_t error_subchunk_ctr_{0};
  counter_t error_block_ctr_{0};
  counter_t last_chunk_size_{0};
};

} // namespace dunedaq::readout

#endif // FLXLIBS_SRC_FELIXSTATISTICS_HPP_
