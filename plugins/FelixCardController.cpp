/**
 * @file FelixCardController.cpp FelixCardController DAQModule implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "FelixCardController.hpp"

namespace dunedaq {
namespace flxlibs {

FelixCardController::FelixCardController(const std::string &name)
  : DAQModule(name)
{}

void set_register()
{}

void read_register()
{}

} // namespace flxlibs
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::flxlibs::FelixCardController)
