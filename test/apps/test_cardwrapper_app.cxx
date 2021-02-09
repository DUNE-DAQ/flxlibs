/**
 * @file test_cardwrapper_app.cxx Test application for 
 * CardWrapper. Inits, starts, stops the DMA transfer.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include "CardWrapper.hpp"

#include <ers/ers.h>
#include <nlohmann/json.hpp>

#include <atomic>
#include <string>
#include <chrono>
#include <memory>

using namespace dunedaq::flxlibs;

int
main(int /*argc*/, char** /*argv[]*/)
{
  // Run marker
  std::atomic<bool> marker{true};

  // Killswitch that flips the run marker
  auto killswitch = std::thread([&]() {
    ERS_INFO("Application will terminate in 10s...");
    std::this_thread::sleep_for(std::chrono::seconds(10));
    marker.store(false);
  });

  nlohmann::json cmd_params = "{}"_json;

  // CardWrapper
  ERS_INFO("Creating CardWrapper...");
  CardWrapper flx;

  ERS_INFO("Init CardWrapper...");
  flx.init(cmd_params);

  ERS_INFO("Configure CardWrapper...");
  flx.configure(cmd_params);

  ERS_INFO("Start CardWrapper...");
  flx.start(cmd_params);

  ERS_INFO("Flipping killswitch in order to stop...");
  if (killswitch.joinable()) {
    killswitch.join();
  }

  ERS_INFO("Stop CardWrapper...");
  flx.stop(cmd_params); 

  ERS_INFO("Exiting.");
  return 0;
}
