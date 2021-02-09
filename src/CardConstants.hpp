/**
 * @file CardConstants.hpp FELIX Card related constants
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
*/
#ifndef FLXLIBS_SRC_CARDCONSTANTS_HPP_
#define FLXLIBS_SRC_CARDCONSTANTS_HPP_

#include "packetformat/block_format.hpp"

namespace dunedaq::flxlibs::constant {

  static constexpr size_t max_links_per_card = 6;
  static constexpr size_t margin_blocks = 4;
  static constexpr size_t block_threshold = 256;
  static constexpr size_t block_size = felix::packetformat::BLOCKSIZE; 
  static constexpr size_t dma_wraparound = FLX_DMA_WRAPAROUND;

} // namespace dunedaq::flxlibs::constant

#endif // FLXLIBS_SRC_CARDCONSTANTS_HPP_
