/**
* @file CreateRedout.hpp Specific readout creator
* Thanks for Brett and Phil for the idea
*
* This is part of the DUNE DAQ , copyright 2020.
* Licensing/copyright details are in the COPYING file that you should have
* received with this code.
*/
#ifndef FLXLIBS_SRC_CREATEELINK_HPP_
#define FLXLIBS_SRC_CREATEELINK_HPP_

#include "readout/ReadoutTypes.hpp"
#include "ElinkConcept.hpp"
#include "ElinkModel.hpp"

namespace dunedaq {
namespace flxlibs {

std::unique_ptr<ElinkConcept> 
createElinkModel(const std::string& target)
{   
  if (target.find("wib") != std::string::npos) {
    return std::make_unique<ElinkModel<readout::types::WIB_SUPERCHUNK_STRUCT>>();
  }

  return nullptr;
}

} // namespace flxlibs
} // namespace dunedaq

#endif // FLXLIBS_SRC_CREATEELINK_HPP_
