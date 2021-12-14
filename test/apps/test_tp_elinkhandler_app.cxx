/**
 * @file test_elinkhandler_app.cxx Test application for
 * ElinkConcept and ElinkModel. Inits, starts, stops block parsers.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include "CardWrapper.hpp"
#include "CreateElink.hpp"
#include "ElinkConcept.hpp"
#include "flxlibs/AvailableParserOperations.hpp"

#include "logging/Logging.hpp"
#include "fdreadoutlibs/FDReadoutTypes.hpp"

#include "packetformat/block_format.hpp"
#include <nlohmann/json.hpp>

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <sstream>

using namespace dunedaq::flxlibs;
using namespace dunedaq::fdreadoutlibs;

int
main(int /*argc*/, char** /*argv[]*/)
{
  // Run marker
  std::atomic<bool> marker{ true };

  // Killswitch that flips the run marker
  auto killswitch = std::thread([&]() {
    TLOG() << "Application will terminate in 10s...";
    std::this_thread::sleep_for(std::chrono::seconds(10));
    marker.store(false);
  });

  // Dummy command
  nlohmann::json cmd_params = "{}"_json;

  // Counter
  std::atomic<int> block_counter{ 0 };

  // CardWrapper
  TLOG() << "Creating CardWrapper...";
  CardWrapper flx;
  std::map<int, std::unique_ptr<ElinkConcept>> elinks;

  // 5 elink handlers
  for (int i = 0; i < 5; ++i) {
    elinks[i * 64] = createElinkModel("dummy");
    auto& handler = elinks[i * 64];
    handler->init(cmd_params, 100000);
    handler->conf(cmd_params, 4096, true);
    handler->start(cmd_params);
  }

  // Modify a specific elink handler
  bool first = true;
  auto& parser0 = elinks[0]->get_parser();
  parser0.process_subchunk_with_error_func = [&](const felix::packetformat::subchunk& subchunk) {
    // This specific implementation prints the first occurence of a subchunk with error on elink-0.
    if (first) {
      TLOG() << "First subchunk with error:"
             << " Length=" << subchunk.length << " ErrFlag=" << subchunk.err_flag
             << " TrunFlag=" << subchunk.trunc_flag;
      first = false;
    }
  };

  // Add TP link
  elinks[5 * 64] = createElinkModel("dummy");
  auto& tphandler = elinks[5 * 64];
  tphandler->init(cmd_params, 100000);
  tphandler->conf(cmd_params, 4096, true);

  // Modify TP link handler
  /*bool firstTPshort = true;
  auto& tpparser = elinks[5 * 64]->get_parser();
  tpparser.process_shortchunk_func = [&](const felix::packetformat::shortchunk& shortchunk) {
    if (firstTPshort) {
      TLOG() << "SC Lenght: " << shortchunk.length;
      firstTPshort = false;
    }
  };
  */

  bool firstTPchunk = true;
  int amount = 0;
  auto& tpparser = elinks[5 * 64]->get_parser();
  uint64_t good_counter = 0;
  uint64_t total_counter = 0;
  tpparser.process_chunk_func = [&](const felix::packetformat::chunk& chunk) {
    ++total_counter;
    if (firstTPchunk) {
      //TLOG() << "C Lenght: " << chunk.length();

      auto subchunk_data = chunk.subchunks();
      auto subchunk_sizes = chunk.subchunk_lengths();
      auto n_subchunks = chunk.subchunk_number();

      uint32_t bytes_copied_chunk = 0;
      RawWIBTp* rwtpp = static_cast<RawWIBTp*>(std::malloc(chunk.length())); //+ sizeof(int)));
      //auto* rwtpip = reinterpret_cast<uint8_t*>(rwtpp);
      for (unsigned i = 0; i < n_subchunks; i++) {
        parsers::dump_to_buffer(
          subchunk_data[i], subchunk_sizes[i], static_cast<void*>(rwtpp), bytes_copied_chunk, chunk.length());
        bytes_copied_chunk += subchunk_sizes[i];
      }

      if ((uint32_t)(rwtpp->m_head.m_crate_no) == 21) { // RS FIXME -> read from cmdline the list of signatures loaded to EMU
        ++good_counter;
        std::ostringstream oss;
        rwtpp->m_head.print(oss);
        TLOG() << oss.str();
      }

      types::RAW_WIB_TRIGGERPRIMITIVE_STRUCT rwtps;
      rwtps.rwtp.reset(rwtpp);

      if (amount > 1000) {
        firstTPchunk = false;
      } else {
        amount++;
      }
    }
  };
  tphandler->start(cmd_params);

  // Implement how block addresses should be handled
  std::function<void(uint64_t)> count_block_addr = [&](uint64_t block_addr) { // NOLINT
    ++block_counter;
    const auto* block = const_cast<felix::packetformat::block*>(
      felix::packetformat::block_from_bytes(reinterpret_cast<const char*>(block_addr)) // NOLINT
    );
    auto elink = block->elink;
    if (elinks.count(elink) != 0) {
      if (elinks[elink]->queue_in_block_address(block_addr)) {
        // queued block
      } else {
        // couldn't queue block
      }
    }
  };

  // Set this function as the handler of blocks.
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

  TLOG() << "Stop ElinkHandlers...";
  for (auto const& [tag, handler] : elinks) {
    handler->stop(cmd_params);
  }

  TLOG() << "GOOD counter: " << good_counter;
  TLOG() << "Total counter" << total_counter;

  TLOG() << "Number of blocks DMA-d: " << block_counter;

  TLOG() << "Exiting.";
  return 0;
}
