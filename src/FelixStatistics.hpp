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

using counter_t = std::atomic<int>;

struct ParserStats {
  counter_t packet_ctr{0};
  counter_t short_ctr{0};
  counter_t chunk_ctr{0};
  counter_t subchunk_ctr{0};
  counter_t block_ctr{0};
  counter_t error_short_ctr{0};
  counter_t error_chunk_ctr{0};
  counter_t error_subchunk_ctr{0};
  counter_t error_block_ctr{0};
};

} // namespace dunedaq::flxlibs::stats

#endif // FLXLIBS_SRC_FELIXSTATISTICS_HPP_
