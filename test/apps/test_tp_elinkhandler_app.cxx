/**
 * @file test_tp_elinkhandler_app.cxx Test application for
 * ElinkConcept and ElinkModel for handling Raw WIB TPs.
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
#include "readoutlibs/ReadoutTypes.hpp"
#include "detdataformats/wib/RawWIBTp.hpp"

#include "packetformat/block_format.hpp"
#include <nlohmann/json.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <fstream>
#include <future>
#include <map>
#include <memory>
#include <string>
#include <sstream>

using namespace dunedaq::flxlibs;
using namespace dunedaq::fdreadoutlibs;


struct TP_SUPERCHUNK_STRUCT
{
  TP_SUPERCHUNK_STRUCT() {}
  TP_SUPERCHUNK_STRUCT(size_t size, char* data)
    : size(size)
    , data(data)
  {}

  size_t size = 0;
  std::unique_ptr<char> data = nullptr;
};

using LatencyBuffer = folly::ProducerConsumerQueue<TP_SUPERCHUNK_STRUCT>;

int
main(int argc, char* argv[])
{
  // Run marker
  std::atomic<bool> marker{ true };

  // Killswitch that flips the run marker
  auto killswitch = std::thread([&]() {
    TLOG() << "Application will terminate in 60s...";
    std::this_thread::sleep_for(std::chrono::seconds(60));
    marker.store(false);
  });

  // Dummy command
  nlohmann::json cmd_params = "{}"_json;

  // config command to specify card ID
  nlohmann::json conf_params_slr1 = "{\"card_id\":0,\"logical_unit\":0}"_json;
  if(argc > 2)
  {
    std::string card_id = argv[1];
    std::string logical_unit = argv[2];
    if(std::all_of(card_id.begin(), card_id.end(), ::isdigit) && std::all_of(logical_unit.begin(), logical_unit.end(), ::isdigit))
    {
      conf_params_slr1 = {{"card_id", std::stoi(card_id)},{"logical_unit", std::stoi(logical_unit)}};
    }
    else
    {
      TLOG() << "invalid card id/logical unit specified, setting id to 0 and logical unit to 0.";
    }
  }


  // Counter
  std::atomic<int> block_counter{ 0 };

  // CardWrapper
  TLOG() << "Creating CardWrapper...";
  CardWrapper flx;
  std::map<int, std::unique_ptr<ElinkConcept>> elinks;

  TLOG() << "Creating Elink models...";
  // 5 elink handlers
  for (int i = 0; i < 5; ++i) {
    TLOG() << "Elink " << i << "...";
    //elinks[i * 64] = createElinkModel("wib");
    elinks[i * 64] = std::make_unique<ElinkModel<types::ProtoWIBSuperChunkTypeAdapter>>();
    auto& handler = elinks[i * 64];
    handler->init(cmd_params, 100000);
    handler->conf(cmd_params, 4096, true);
    handler->start(cmd_params);
  }

  // Add TP link
  TLOG() << "Creating TP link...";
  //elinks[5 * 64] = createElinkModel("raw_tp");
  elinks[5 * 64] = std::make_unique<ElinkModel<TP_SUPERCHUNK_STRUCT>>();
  auto& tphandler = elinks[5 * 64];
  tphandler->init(cmd_params, 100000);
  tphandler->conf(cmd_params, 4096, true);
  std::unique_ptr<folly::ProducerConsumerQueue<TP_SUPERCHUNK_STRUCT>> tpbuffer = std::make_unique<LatencyBuffer>(1000000);


  // Modify a specific elink handler
  bool first = true;
  int firstWIBframes = 0;
  auto& parser0 = elinks[0]->get_parser();
  parser0.process_chunk_func = [&](const felix::packetformat::chunk& chunk) {
    auto subchunk_data = chunk.subchunks();
    auto subchunk_sizes = chunk.subchunk_lengths();
    auto n_subchunks = chunk.subchunk_number();
    types::ProtoWIBSuperChunkTypeAdapter wss;
    uint32_t bytes_copied_chunk = 0; // NOLINT 
    for (unsigned i = 0; i < n_subchunks; i++) {
      parsers::dump_to_buffer(
        subchunk_data[i], subchunk_sizes[i], static_cast<void*>(&wss.data), bytes_copied_chunk, sizeof(types::ProtoWIBSuperChunkTypeAdapter));
      bytes_copied_chunk += subchunk_sizes[i];
    }

    if (first) {
      TLOG() << "Chunk with length: " << chunk.length();
      ++firstWIBframes;
      if (firstWIBframes > 100) {
        first = false;
      }
    }
  };

  bool firstTPchunk = true;
  int amount = 0;
  auto& tpparser = elinks[5 * 64]->get_parser();
  uint64_t good_counter = 0;
  uint64_t total_counter = 0;
  tpparser.process_chunk_func = [&](const felix::packetformat::chunk& chunk) {
    ++total_counter;
    if (firstTPchunk) {
      auto subchunk_data = chunk.subchunks();
      auto subchunk_sizes = chunk.subchunk_lengths();
      auto n_subchunks = chunk.subchunk_number();
      auto chunk_length = chunk.length();
      
      TLOG() << "TP subchunk number: " << n_subchunks; 
      TLOG() << "TP chunk length: " << chunk_length;

      uint32_t bytes_copied_chunk = 0;
      dunedaq::detdataformats::wib::RawWIBTp* rwtpp = static_cast<dunedaq::detdataformats::wib::RawWIBTp*>(std::malloc(chunk_length)); //+ sizeof(int)));
      //auto* rwtpip = reinterpret_cast<uint8_t*>(rwtpp);
      
      char* payload = static_cast<char*>(malloc(chunk_length * sizeof(char)));
      TP_SUPERCHUNK_STRUCT payload_struct(chunk_length, payload);
      for (unsigned i = 0; i < n_subchunks; i++) {
        TLOG() << "TP subchunk " << i << " length: " << subchunk_sizes[i];
        parsers::dump_to_buffer(
          subchunk_data[i], subchunk_sizes[i], static_cast<void*>(payload), bytes_copied_chunk, chunk_length);
        bytes_copied_chunk += subchunk_sizes[i];
      }
      if (!tpbuffer->write(std::move(payload_struct))) {
        // Buffer full
      }

      if ((uint32_t)(rwtpp->m_head.m_crate_no) == 21) { // RS FIXME -> read from cmdline the list of signatures loaded to EMU
        ++good_counter;
      }

      std::ostringstream oss;
      rwtpp->m_head.print(oss);
      //types::DUNEWIBFirmwareTriggerPrimitiveSuperChunkTypeAdapter rwtps; // TODO 2022-04-21 ivana.hristova@stfc.ac.uk: currently this pointer has been removed  
      //rwtps.rwtp.reset(rwtpp);
      TLOG() << oss.str();

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
  flx.configure(conf_params_slr1);

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

  // Filewriter
  std::function<size_t(std::string, std::unique_ptr<LatencyBuffer>&)> write_to_file =
    [&](std::string filename, std::unique_ptr<LatencyBuffer>& buffer) {
      std::ofstream linkfile(filename, std::ios::out | std::ios::binary);
      size_t bytes_written = 0;
      TP_SUPERCHUNK_STRUCT spc;
      while (!buffer->isEmpty()) {
        TLOG() << "chunk length: " << spc.size;
        buffer->read(spc);
        linkfile.write(spc.data.get(), spc.size);
        bytes_written += spc.size;
      }
      return bytes_written;
    };

  TLOG() << "Time to write out the data...";
  std::map<int, std::future<size_t>> done_futures;

  std::ostringstream fnamestr;
  fnamestr << "slr1-" << 5 << "-data.bin";
  TLOG() << "  -> Dropping data to file: " << fnamestr.str();
  std::string fname = fnamestr.str();
  done_futures[5] = std::async(std::launch::async, write_to_file, fname, std::ref(tpbuffer));


  TLOG() << "Wait for them. This might take a while...";
  for (auto& [id, fut] : done_futures) {
    size_t bw = fut.get();
    TLOG() << "[" << id << "] Bytes written: " << bw;
  }

  TLOG() << "GOOD counter: " << good_counter;
  TLOG() << "Total counter: " << total_counter;

  TLOG() << "Number of blocks DMA-d: " << block_counter;

  TLOG() << "Exiting.";
  return 0;
}
