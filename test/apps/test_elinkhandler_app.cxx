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
#include "ElinkConcept.hpp"
#include "CreateElink.hpp"

#include "flxlibs/AvailableParserOperations.hpp"

#include "readout/ReadoutTypes.hpp"

#include "packetformat/block_format.hpp"

#include <ers/ers.h>
#include <nlohmann/json.hpp>

#include <atomic>
#include <string>
#include <chrono>
#include <memory>

using namespace dunedaq::flxlibs;
using namespace dunedaq::readout;

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
  std::map<int, std::unique_ptr<ElinkConcept>> elinks;

  // 5 elink handlers
  for (int i=0; i<5; ++i) {
    elinks[i*64] = createElinkModel("wib");
    auto& handler = elinks[i*64];
    handler->init(cmd_params);
    handler->conf(cmd_params);
    handler->start(cmd_params);
  }

  // Modify a specific elink handler
  bool first = true;
  auto& parser0 = elinks[0]->get_parser();
  parser0.process_subchunk_with_error_func = [&](const felix::packetformat::subchunk& subchunk) {
    // This specific implementation prints the first occurence of a subchunk with error on elink-0.
    if (first) {
      ERS_INFO("First subchunk with error:"
        << " Length=" << subchunk.length
        << " ErrFlag=" << subchunk.err_flag
        << " TrunFlag=" << subchunk.trunc_flag);
      first = false;
    }
  };

  // Sink
  //using FrameSink = dunedaq::readout::types::WIBFramePtrSink;
  //FrameSink wibsink("wibsink0");
  //parser0.process_chunk_func = parsers::fixedsizeChunkInto<types::WIB_SUPERCHUNK_STRUCT>(wibsink);

  // Implement how block addresses should be handled
  std::function<void(uint64_t)> count_block_addr = [&](uint64_t block_addr) {
    ++block_counter;
    const auto* block = const_cast<felix::packetformat::block*>(
      felix::packetformat::block_from_bytes(reinterpret_cast<const char*>(block_addr))
    );
    auto elink = block->elink;
    if (elinks.count(elink) != 0) {
      if (elinks[elink]->queue_in_block(block_addr)) {
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
  for (auto const& [tag, handler] : elinks) {
    handler->stop(cmd_params);
  }

  ERS_INFO("Number of blocks DMA-d: " << block_counter);

  ERS_INFO("Exiting.");
  return 0;
}
