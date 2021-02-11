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
#include "ElinkHandler.hpp"

#include "packetformat/block_format.hpp"

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

  // Dummy command
  nlohmann::json cmd_params = "{}"_json;

  // Counter
  std::atomic<int> block_counter{0};

  // CardWrapper
  ERS_INFO("Creating CardWrapper...");
  CardWrapper flx;
  std::map<int, std::unique_ptr<ElinkHandler>> elink_handlers;

  // 5 elink handlers
  for (int i=0; i<5; ++i) {
    elink_handlers[i*64] = std::make_unique<ElinkHandler>();
    auto& handler = elink_handlers[i*64];
    handler->init(cmd_params);
    handler->configure(cmd_params);
    handler->start(cmd_params);
  }

  // Implement how block addresses should be handled
  std::function<void(uint64_t)> count_block_addr = [&](uint64_t block_addr) {
    ++block_counter;
    const auto* block = const_cast<felix::packetformat::block*>(
      felix::packetformat::block_from_bytes(reinterpret_cast<const char*>(block_addr))
    );
    auto elink = block->elink;
    if (elink_handlers.count(elink) != 0) {
      if (elink_handlers[elink]->queue_in_block(block_addr)) {
        // queued block
      } else {
        // couldn't queue block
      }
    } 
  };

  // Set this function as the handler of blocks.
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

  ERS_INFO("Stop ElinkHandlers...");
  for (auto const& [tag, handler] : elink_handlers) {
    handler->stop(cmd_params);
  }

  ERS_INFO("Number of blocks DMA-d: " << block_counter);

  ERS_INFO("Exiting.");
  return 0;
}
