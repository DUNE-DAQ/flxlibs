/**
 * @file test_cardwrapper_app.cxx Test application for 
 * CardWrapper. Inits, starts, stops the DMA transfer. 
 * Also demonstrates the most basic block interpretation/handler callback.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include "flxlibs/AvailableParserOperations.hpp"
#include "CardWrapper.hpp"
#include "ElinkModel.hpp"

#include "logging/Logging.hpp"

#include "packetformat/block_format.hpp"

#include <nlohmann/json.hpp>
#include <folly/ProducerConsumerQueue.h>

#include <atomic>
#include <string>
#include <chrono>
#include <memory>
#include <map>
#include <utility>
#include <future>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace dunedaq::flxlibs;

const constexpr std::size_t USER_PAYLOAD_SIZE = 5568; // for 12: 5568
struct USER_PAYLOAD_STRUCT {
  char data[USER_PAYLOAD_SIZE];
};

using LatencyBuffer = folly::ProducerConsumerQueue<USER_PAYLOAD_STRUCT>;

inline std::function<void(const felix::packetformat::chunk& chunk)>
payloadToBuffer(std::unique_ptr<LatencyBuffer>& buffer)
{
  return [&](const felix::packetformat::chunk& chunk) {
    // Chunk info
    auto subchunk_data = chunk.subchunks();
    auto subchunk_sizes = chunk.subchunk_lengths();
    auto n_subchunks = chunk.subchunk_number();

    // Only dump if possible
    if (chunk.length() != USER_PAYLOAD_SIZE) {
      // scream?
    } else {
      USER_PAYLOAD_STRUCT payload;
      uint32_t bytes_copied_chunk = 0; // NOLINT
      for(unsigned i=0; i<n_subchunks; i++) {
        parsers::dump_to_buffer(subchunk_data[i], subchunk_sizes[i],
                               static_cast<void*>(&payload.data),
                               bytes_copied_chunk,
                               USER_PAYLOAD_SIZE);
        bytes_copied_chunk += subchunk_sizes[i];
      }
      if (!buffer->write(std::move(payload))) {
        // Buffer full
      }
    }
  };  
}

struct BlockRouter
{
  std::map<int, std::unique_ptr<ElinkModel<USER_PAYLOAD_STRUCT>>> elinks;
  std::map<int, std::unique_ptr<folly::ProducerConsumerQueue<USER_PAYLOAD_STRUCT>>> lbuffers;

  std::map<unsigned, size_t> elink_block_counters;
  size_t block_counter = 0;

  std::function<void(uint64_t)> count_block_addr = [&, this](uint64_t block_addr) { // NOLINT 
    block_counter++;
    const auto* block = const_cast<felix::packetformat::block*>(
      felix::packetformat::block_from_bytes(reinterpret_cast<const char*>(block_addr)) // NOLINT
    );
    auto elink = block->elink;
    if(this->elink_block_counters.count(elink) == 0) {
       this->elink_block_counters[elink] = 0;
    }
    this->elink_block_counters[elink]++;
    if (this->elinks.count(elink) == 0) {
      // unexpected elink
    } else {
      this->elinks[elink]->queue_in_block_address(block_addr);
    }
  };
};

int
main(int /*argc*/, char** /*argv[]*/)
{
  // Run marker
  std::atomic<bool> marker{true};

  // Killswitch that flips the run marker
  auto killswitch = std::thread([&]() {
    TLOG() << "Application will terminate in 5s and show encountered ELink IDs in BLOCK headers...";
    std::this_thread::sleep_for(std::chrono::seconds(5));
    marker.store(false);
  });

  nlohmann::json def_params = "{}"_json;

  nlohmann::json conf_params_slr1 = "{\"card_id\":0,\"chunk_trailer_size\":32,\"dma_block_size_kb\":4,"
                                    "\"dma_id\":0,\"dma_memory_size_gb\":4,\"logical_unit\":0,"
                                    "\"num_links\":5,\"num_sources\":1,\"numa_id\":0}"_json;

  nlohmann::json conf_params_slr2 = "{\"card_id\":0,\"chunk_trailer_size\":32,\"dma_block_size_kb\":4,"
                                    "\"dma_id\":0,\"dma_memory_size_gb\":4,\"logical_unit\":1,"
                                    "\"num_links\":5,\"num_sources\":1,\"numa_id\":0}"_json;

  // CardWrapper
  TLOG() << "Creating CardWrappers...";
  std::pair<CardWrapper, CardWrapper> flxs; // 2 SLRS
  flxs.first.init(def_params);
  flxs.second.init(def_params);
 
  BlockRouter slr1_router;
  BlockRouter slr2_router;
  for (unsigned i=0; i<5; ++i) {
    auto tag = i * 64;
    slr1_router.elinks[tag] = std::make_unique<ElinkModel<USER_PAYLOAD_STRUCT>>();
    slr2_router.elinks[tag] = std::make_unique<ElinkModel<USER_PAYLOAD_STRUCT>>();
    slr1_router.lbuffers[tag] = std::make_unique<LatencyBuffer>(1000000);
    slr2_router.lbuffers[tag] = std::make_unique<LatencyBuffer>(1000000);
    auto& parser1 = slr1_router.elinks[tag]->get_parser();
    auto& parser2 = slr2_router.elinks[tag]->get_parser();
    parser1.process_chunk_func = payloadToBuffer(std::ref(slr1_router.lbuffers[tag]));
    parser2.process_chunk_func = payloadToBuffer(std::ref(slr2_router.lbuffers[tag]));
    slr1_router.elinks[tag]->init(def_params, 1000000);
    slr2_router.elinks[tag]->init(def_params, 1000000);
    slr1_router.elinks[tag]->set_ids(0, 0, i, tag);
    slr2_router.elinks[tag]->set_ids(0, 1, i, tag);
    slr1_router.elinks[tag]->conf(def_params, 4096, true);
    slr2_router.elinks[tag]->conf(def_params, 4096, true);
  }

  flxs.first.set_block_addr_handler(slr1_router.count_block_addr);
  flxs.second.set_block_addr_handler(slr2_router.count_block_addr);

  TLOG() << "Init CardWrappers...";
  flxs.first.init(def_params);
  flxs.second.init(def_params);

  TLOG() << "Configure CardWrappers...";
  flxs.first.configure(conf_params_slr1);
  flxs.second.configure(conf_params_slr2);

  TLOG() << "Start CardWrappers...";
  flxs.first.start(def_params);
  flxs.second.start(def_params);
  TLOG() << "Start ElinkModels...";
  for (const auto& [id, elink]: slr1_router.elinks) {
    elink->start(def_params);
  }
  for (const auto& [id, elink]: slr2_router.elinks) {
    elink->start(def_params);
  }

  TLOG() << "Flipping killswitch in order to stop...";
  if (killswitch.joinable()) {
    killswitch.join();
  }

  TLOG() << "Stop CardWrapper...";
  flxs.first.stop(def_params); 
  flxs.second.stop(def_params); 
  TLOG() << "Stop ElinkModels...";
  for (const auto& [id, elink]: slr1_router.elinks) {
    elink->stop(def_params);
  }
  for (const auto& [id, elink]: slr2_router.elinks) {
    elink->stop(def_params);
  }

  TLOG() << "Number of SLR1 blocks DMA-d: " << slr2_router.block_counter
         << "-> Per elink: ";
  for (const auto& [elinkid, count]: slr1_router.elink_block_counters) {
    TLOG() << "  elink(" << std::to_string(elinkid) << "): " << std::to_string(count);
  }

  TLOG() << "Number of SLR2 blocks DMA-d: " << slr2_router.block_counter
         << "-> Per elink: ";
  for (const auto& [elinkid, count]: slr2_router.elink_block_counters) {
    TLOG() << "  elink(" << std::to_string(elinkid) << "): " << std::to_string(count);
  }

  // Filewriter
  std::function<size_t(std::string, std::unique_ptr<LatencyBuffer>&)> write_to_file = 
    [&](std::string filename, std::unique_ptr<LatencyBuffer>& buffer) {
      std::ofstream linkfile(filename, std::ios::out | std::ios::binary);
      size_t bytes_written = 0;
      USER_PAYLOAD_STRUCT upc;
      while (!buffer->isEmpty()) {
        buffer->read(upc);
        linkfile.write(upc.data, USER_PAYLOAD_SIZE);
        bytes_written += USER_PAYLOAD_SIZE;
      }
      return bytes_written;
  };

  TLOG() << "Time to write out the data...";
  std::map<int, std::future<size_t>> done_futures;
  for (auto& [id, buffer]: slr1_router.lbuffers) {
    std::ostringstream fnamestr;
    fnamestr << "slr1-" << id << "-data.bin";
    TLOG() << "  -> Dropping data to file: " << fnamestr.str();
    std::string fname = fnamestr.str();
    done_futures[id] = std::async(std::launch::async, write_to_file, fname, std::ref(buffer));
  }
  for (auto& [id, buffer]: slr2_router.lbuffers) {
    std::ostringstream fnamestr;
    fnamestr << "slr2-" << id << "-data.bin";
    TLOG() << "  -> Dropping data to file: " << fnamestr.str();
    std::string fname = fnamestr.str();
    done_futures[id+2048] = std::async(std::launch::async, write_to_file, fname, std::ref(buffer));
  }

  TLOG() << "Wait for them. This might take a while...";
  for (auto& [id, fut] : done_futures) {
    size_t bw = fut.get();
    TLOG() << "[" << id << "] Bytes written: " << bw;
  }

  TLOG() << "Exiting.";
  return 0;
}
