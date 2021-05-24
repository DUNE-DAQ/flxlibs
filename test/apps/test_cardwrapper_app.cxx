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

#include "logging/Logging.hpp"

#include "packetformat/block_format.hpp"

#include <nlohmann/json.hpp>

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <string>

using namespace dunedaq::flxlibs;

int
main(int /*argc*/, char** /*argv[]*/)
{
  // Run marker
  std::atomic<bool> marker{ true };

  // Killswitch that flips the run marker
  auto killswitch = std::thread([&]() {
    TLOG() << "Application will terminate in 5s and show encountered ELink IDs in BLOCK headers...";
    std::this_thread::sleep_for(std::chrono::seconds(5));
    marker.store(false);
  });

  nlohmann::json cmd_params = "{}"_json;

  // CardWrapper
  TLOG() << "Creating CardWrapper...";
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
    if (elink_block_counters.count(elink) == 0) {
      elink_block_counters[elink] = 0;
    }
    elink_block_counters[elink]++;
  };

  flx.set_block_addr_handler(count_block_addr);

  TLOG() << "Init CardWrapper...";
  flx.init(cmd_params);

  TLOG() << "Configure CardWrapper...";
  flx.configure(cmd_params);

  TLOG() << "Start CardWrapper...";
  flx.start(cmd_params);

  TLOG() << "Flipping killswitch in order to stop...";
  if (killswitch.joinable()) {
    killswitch.join();
  }

  TLOG() << "Stop CardWrapper...";
  flx.stop(cmd_params);

  TLOG() << "Number of blocks DMA-d: " << block_counter << "-> Per elink: ";
  for (const auto& [elinkid, count] : elink_block_counters) {
    TLOG() << "  elink(" << std::to_string(elinkid) << "): " << std::to_string(count);
  }

  TLOG() << "Exiting.";
  return 0;
}
