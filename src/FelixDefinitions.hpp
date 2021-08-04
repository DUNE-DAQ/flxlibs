/**
 * @file FelixDefinitions.hpp FELIX related constants and defines
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef FLXLIBS_SRC_FELIXDEFINITIONS_HPP_
#define FLXLIBS_SRC_FELIXDEFINITIONS_HPP_

#include "regmap/regmap.h"

namespace dunedaq {
namespace flxlibs {

// define not bound by namespace
#define MAX_IRQ 8 // NOLINT(build/define_used)

#if REGMAP_VERSION < 0x500
#define IRQ_WRAP_AROUND_FROM_HOST 0 // NOLINT(build/define_used)
#define IRQ_WRAP_AROUND_TO_HOST 1   // NOLINT(build/define_used)
#define IRQ_DATA_AVAILABLE 2        // NOLINT(build/define_used)
#define IRQ_FIFO_FULL_FROM_HOST 3   // NOLINT(build/define_used)
#define IRQ_PROG_FULL_TO_HOST 6     // NOLINT(build/define_used)
#define IRQ_FIFO_FULL_TO_HOST 7     // NOLINT(build/define_used)
#else
#define IRQ_DATA_AVAILABLE 0 // NOLINT(build/define_used)
#endif                       // REGMAP_VERSION

} // namespace flxlibs
} // namespace dunedaq

#endif // FLXLIBS_SRC_FELIXDEFINITIONS_HPP_
