/**
 * @file FelixCardController.hpp FELIX card controller DAQ Module
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef FLXLIBS_PLUGINS_FELIXCARDCONTROLLER_HPP
#define FLXLIBS_PLUGINS_FELIXCARDCONTROLLER_HPP

// From appfwk
#include "appfwk/DAQModule.hpp"

#include <string>

namespace dunedaq {
namespace flxlibs {

class FelixCardController : public dunedaq::appfwk::DAQModule
{
public:
  explicit FelixCardController(const std::string& name);

  FelixCardController(const FelixCardController&) = delete;            ///< FelixCardController is not copy-constructible
  FelixCardController& operator=(const FelixCardController&) = delete; ///< FelixCardController is not copy-assignable
  FelixCardController(FelixCardController&&) = delete;                 ///< FelixCardController is not move-constructible
  FelixCardController& operator=(FelixCardController&&) = delete;      ///< FelixCardController is not move-assignable

  void init(const data_t&) override;

private:
  void set_register();
  void read_register();
};

} // namespace flxlibs
} // namespace dunedaq

#endif // FLXLIBS_PLUGINS_FELIXCARDCONTROLLER_HPP
