/**
 * @file test_cardwrapper_app.cxx Test application for 
 * CardWrapper. Inits, starts, stops the DMA transfer. 
 * Also demonstrates the most basic block interpretation/handler callback.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include "CardWrapper.hpp"

#include "packetformat/block_format.hpp"

#include <ers/ers.h>
#include <nlohmann/json.hpp>

#include <atomic>
#include <string>
#include <chrono>
#include <memory>
#include <map>

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

  // Set how block addresses should be handled
  std::map<unsigned, size_t> elink_block_counters;
  size_t block_counter = 0;
  std::function<void(uint64_t)> count_block_addr = [&](uint64_t block_addr) { // NOLINT 
    block_counter++;
    const auto* block = const_cast<felix::packetformat::block*>(
      felix::packetformat::block_from_bytes(reinterpret_cast<const char*>(block_addr)) // NOLINT
    );
    auto elink = block->elink;
    if(elink_block_counters.count(elink) == 0) {
       elink_block_counters[elink] = 0;
    }
    elink_block_counters[elink]++;
  };

  flx.set_block_addr_handler(count_block_addr);

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

  ERS_INFO("Number of blocks DMA-d: " << block_counter);
  ERS_INFO("-> Per elink: ");
  for (const auto& [elinkid, count]: elink_block_counters) {
    ERS_INFO("  elink(" << std::to_string(elinkid) << "): " << std::to_string(count));
  }

  ERS_INFO("Exiting.");
  return 0;
}
