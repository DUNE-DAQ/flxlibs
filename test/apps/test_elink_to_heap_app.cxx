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

struct PayloadWrapper {
  PayloadWrapper() {}
  PayloadWrapper(size_t size, char* data) : size(size), data(data) {}

  size_t size = 0;
  std::unique_ptr<char> data = nullptr;
};

using LatencyBuffer = folly::ProducerConsumerQueue<PayloadWrapper>;

inline std::function<void(const felix::packetformat::chunk& chunk)>
payloadToBuffer(std::unique_ptr<LatencyBuffer>& buffer)
{
  return [&](const felix::packetformat::chunk& chunk) {
    auto subchunk_data = chunk.subchunks();
    auto subchunk_sizes = chunk.subchunk_lengths();
    auto n_subchunks = chunk.subchunk_number();
    auto chunk_length = chunk.length();

    char* payload = static_cast<char*>( malloc(chunk_length * sizeof(char)) );
    uint32_t bytes_copied_chunk = 0;
    for (unsigned i=0; i<n_subchunks; ++i) {
      parsers::dump_to_buffer(subchunk_data[i], subchunk_sizes[i],
                     static_cast<void*>(payload),
                     bytes_copied_chunk,
                     chunk_length);
      bytes_copied_chunk += subchunk_sizes[i];
    }
    PayloadWrapper payload_wrapper(chunk_length, payload);
    buffer->write(std::move(payload_wrapper));
  };  
}

struct BlockRouter
{
  std::map<int, std::unique_ptr<ElinkModel<PayloadWrapper>>> elinks;
  std::map<int, std::unique_ptr<folly::ProducerConsumerQueue<PayloadWrapper>>> lbuffers;

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
    std::this_thread::sleep_for(std::chrono::seconds(15));
    marker.store(false);
  });

  nlohmann::json def_params = "{}"_json;

  nlohmann::json conf_params_slr1 = "{\"card_id\":0,\"chunk_trailer_size\":32,\"dma_block_size_kb\":4,"
                                    "\"dma_id\":0,\"dma_memory_size_gb\":4,\"logical_unit\":0,"
                                    "\"num_links\":5,\"num_sources\":1,\"numa_id\":0}"_json;

  // CardWrapper
  TLOG() << "Creating CardWrappers...";
  CardWrapper flx;
 
  BlockRouter slr1_router;
  for (unsigned i=0; i<5; ++i) {
    auto tag = i * 64;
    slr1_router.elinks[tag] = std::make_unique<ElinkModel<PayloadWrapper>>();
    slr1_router.lbuffers[tag] = std::make_unique<LatencyBuffer>(1000000);
    auto& parser1 = slr1_router.elinks[tag]->get_parser();
    parser1.process_chunk_func = payloadToBuffer(std::ref(slr1_router.lbuffers[tag]));
    slr1_router.elinks[tag]->init(def_params, 1000000);
    slr1_router.elinks[tag]->set_ids(0, 0, i, tag);
    slr1_router.elinks[tag]->conf(def_params, 4096, true);
  }

  flx.set_block_addr_handler(slr1_router.count_block_addr);

  TLOG() << "Init CardWrappers...";
  flx.init(def_params);


  TLOG() << "Configure CardWrappers...";
  flx.configure(conf_params_slr1);


  TLOG() << "Start CardWrappers...";
  flx.start(def_params);

  TLOG() << "Start ElinkModels...";
  for (const auto& [id, elink]: slr1_router.elinks) {
    elink->start(def_params);
  }

  // Consumer
  std::function<std::map<unsigned, size_t>(std::unique_ptr<LatencyBuffer>&)> endless_pop = 
    [&](std::unique_ptr<LatencyBuffer>& buffer) {
      std::map<unsigned, size_t> enc_sizes;
      size_t popped = 0;
      while (marker.load()) {
        if (!buffer->isEmpty()) {
          PayloadWrapper pw;
          buffer->read(pw);
          popped++;
          if (enc_sizes.count(pw.size) == 0) {
            enc_sizes[pw.size] = 0;
          }
          enc_sizes[pw.size]++;
        }
      }
      return enc_sizes;
  };
  
  TLOG() << "Time to spawn consumers...";
  std::map<int, std::future<std::map<unsigned, size_t>>> done_futures;
  for (auto& [id, buffer]: slr1_router.lbuffers) {
    done_futures[id] = std::async(std::launch::async, endless_pop, std::ref(buffer));
  }

  TLOG() << "Flipping killswitch in order to stop...";
  if (killswitch.joinable()) {
    killswitch.join();
  }

  TLOG() << "Stop CardWrapper...";
  flx.stop(def_params); 

  TLOG() << "Stop ElinkModels...";
  for (const auto& [id, elink]: slr1_router.elinks) {
    elink->stop(def_params);
  }

  TLOG() << "Number of SLR1 blocks DMA-d: " << slr1_router.block_counter
         << "-> Per elink: ";
  for (const auto& [elinkid, count]: slr1_router.elink_block_counters) {
    TLOG() << "  elink(" << std::to_string(elinkid) << "): " << std::to_string(count);
  }

  TLOG() << "Wait for them. This might take a while...";
  for (auto& [id, fut] : done_futures) {
    TLOG() << "[" << id << "] encountered sizes: ";
    std::map<unsigned, size_t> bw = fut.get();
    for (auto& [s, c] : bw) {
      TLOG() << " size: " << s << " times: " << c;
    }
  }

  TLOG() << "Exiting.";
  return 0;
}
