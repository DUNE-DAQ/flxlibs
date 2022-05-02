/**
 * @file AvailableParserOperations.hpp Implementations of FELIX data format
 * parsers into user types.
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef FLXLIBS_INCLUDE_FLXLIBS_AVAILABLEPARSEROPERATIONS_HPP_
#define FLXLIBS_INCLUDE_FLXLIBS_AVAILABLEPARSEROPERATIONS_HPP_

#include "FelixIssues.hpp"

#include "iomanager/Sender.hpp"
#include "fdreadoutlibs/FDReadoutTypes.hpp"

#include "packetformat/block_format.hpp"

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <sstream>
#include <utility>

namespace dunedaq {
namespace flxlibs {
namespace parsers {

inline void
print_bytes(std::ostream& ostr, const char* title, const unsigned char* data, std::size_t length, bool format = true)
{
  ostr << title << std::endl;
  ostr << std::setfill('0');
  for (size_t i = 0; i < length; ++i) {
    ostr << std::hex << std::setw(2) << static_cast<int>(data[i]);
    if (format) {
      ostr << (((i + 1) % 16 == 0) ? "\n" : " ");
    }
  }
  ostr << std::endl;
}

inline void
dump_to_buffer(const char* data,
               std::size_t size,
               void* buffer,
               uint32_t buffer_pos, // NOLINT
               const std::size_t& buffer_size)
{
  auto bytes_to_copy = size; // NOLINT
  while (bytes_to_copy > 0) {
    auto n = std::min(bytes_to_copy, buffer_size - buffer_pos); // NOLINT
    std::memcpy(static_cast<char*>(buffer) + buffer_pos, data, n);
    buffer_pos += n;
    bytes_to_copy -= n;
    if (buffer_pos == buffer_size) {
      buffer_pos = 0;
    }
  }
}

template<class TargetStruct>
inline std::function<void(const felix::packetformat::chunk& chunk)>
fixsizedChunkInto(std::shared_ptr<iomanager::SenderConcept<TargetStruct>>& sink,
                  std::chrono::milliseconds timeout = std::chrono::milliseconds(100))
{
  return [&](const felix::packetformat::chunk& chunk) {
    // Chunk info
    auto subchunk_data = chunk.subchunks();
    auto subchunk_sizes = chunk.subchunk_lengths();
    auto n_subchunks = chunk.subchunk_number();
    std::size_t target_size = sizeof(TargetStruct);

    // Only dump to buffer if possible
    if (chunk.length() != target_size) {
      ers::error(UnexpectedChunk(ERS_HERE, chunk.length()));
    } else {
      TargetStruct payload;
      uint32_t bytes_copied_chunk = 0; // NOLINT
      for (unsigned i = 0; i < n_subchunks; i++) {
        dump_to_buffer(
          subchunk_data[i], subchunk_sizes[i], static_cast<void*>(&payload.data), bytes_copied_chunk, target_size);
        bytes_copied_chunk += subchunk_sizes[i];
      }
      try {
        // finally, push to sink
        sink->send(payload, timeout);
      } catch (const dunedaq::iomanager::TimeoutExpired& excpt) {
        // ers::error(ParserOperationQueuePushFailure(ERS_HERE, " "));
      }
    }
  };
}

template<class TargetStruct>
inline std::function<void(const felix::packetformat::shortchunk& shortchunk)>
fixsizedShortchunkInto(std::shared_ptr<iomanager::SenderConcept<TargetStruct>>& sink,
                       std::chrono::milliseconds timeout = std::chrono::milliseconds(100))
{
  return [&](const felix::packetformat::shortchunk& shortchunk) {
    // Only dump to buffer if possible
    std::size_t target_size = sizeof(TargetStruct);
    if (shortchunk.length != target_size) {
      // report? Add custom way of handling unexpected user payloads.
      //  In this case -> not fixed size shortchunk -> shortchunk-to-userbuff not possible
      // Can't throw, and can't print as it may flood output
    } else {
      TargetStruct payload;
      std::memcpy(static_cast<char*>(payload), shortchunk.data, target_size);
      try {
        // finally, push to sink
        sink->send(payload, timeout);
      } catch (const dunedaq::iomanager::TimeoutExpired& excpt) {
        // ers::error(ParserOperationQueuePushFailure(ERS_HERE, " "));
      }
    }
  };
}

template<class TargetStruct>
inline std::function<void(const felix::packetformat::chunk& chunk)>
fixsizedChunkViaHeap(std::shared_ptr<iomanager::SenderConcept<TargetStruct*>>& sink,
                     // std::shared_ptr<iomanager::SenderConcept<std::unique_ptr<TargetStruct>>>& sink,
                     std::chrono::milliseconds timeout = std::chrono::milliseconds(100))
{
  return [&](const felix::packetformat::chunk& chunk) {
    // Chunk info
    auto subchunk_data = chunk.subchunks();
    auto subchunk_sizes = chunk.subchunk_lengths();
    auto n_subchunks = chunk.subchunk_number();
    auto target_size = sizeof(TargetStruct);

    // Only dump to buffer if possible
    if (chunk.length() != target_size) {
      // report? Add custom way of handling unexpected user payloads.
      //   In this case -> not fixed size chunk -> chunk-to-userbuff not possible
    } else {
      TargetStruct* payload = new TargetStruct[sizeof(TargetStruct)];
      // std::unique_ptr<TargetStruct> payload = std::make_unique<TargetStruct>();
      uint_fast32_t bytes_copied_chunk = 0; // NOLINT
      for (unsigned i = 0; i < n_subchunks; i++) {
        dump_to_buffer(subchunk_data[i],
                       subchunk_sizes[i],
                       static_cast<void*>(payload), // static_cast<void*>(&payload_ptr->data),
                       bytes_copied_chunk,
                       target_size);
        bytes_copied_chunk += subchunk_sizes[i];
      }
      try {
        // finally, push to sink
        sink->send(payload, timeout); // std::move(std::make_unique<TargetStruct>(payload)), timeout);
      } catch (const dunedaq::iomanager::TimeoutExpired& excpt) {
        // ers::error(ParserOperationQueuePushFailure(ERS_HERE, " "));
      }
    }
  };
}

template<class TargetWithDatafield>
inline std::function<void(const felix::packetformat::chunk&)>
varsizedChunkIntoWithDatafield(std::shared_ptr<iomanager::SenderConcept<TargetWithDatafield>>& sink,
                               std::chrono::milliseconds timeout = std::chrono::milliseconds(100))
{
  return [&](const felix::packetformat::chunk& chunk) {
    auto subchunk_data = chunk.subchunks();
    auto subchunk_sizes = chunk.subchunk_lengths();
    auto n_subchunks = chunk.subchunk_number();
    TargetWithDatafield twd;
    twd.get_data().reserve(chunk.length());
    uint32_t bytes_copied_chunk = 0;
    for (unsigned i = 0; i< n_subchunks; ++i) {
      dump_to_buffer(subchunk_data[i],
                     subchunk_sizes[i],
                     static_cast<void*>(twd.get_data().data()),
                     bytes_copied_chunk,
                     chunk.length());
      bytes_copied_chunk += subchunk_sizes[i];
    }
    twd.set_data_size(bytes_copied_chunk);
    try {
      sink->send(twd, timeout);
    } catch (const dunedaq::iomanager::TimeoutExpired& excpt) {
      // ers::error
    }
  };
}

inline std::function<void(const felix::packetformat::chunk& chunk)>
varsizedChunkIntoWrapper(std::shared_ptr<iomanager::SenderConcept<fdreadoutlibs::types::VariableSizePayloadWrapper>>& sink,
                         std::chrono::milliseconds timeout = std::chrono::milliseconds(100))
{
  return [&](const felix::packetformat::chunk& chunk) {
    auto subchunk_data = chunk.subchunks();
    auto subchunk_sizes = chunk.subchunk_lengths();
    auto n_subchunks = chunk.subchunk_number();
    auto chunk_length = chunk.length();

    char* payload = static_cast<char*>(malloc(chunk_length * sizeof(char)));
    uint32_t bytes_copied_chunk = 0; // NOLINT(build/unsigned)
    for (unsigned i = 0; i < n_subchunks; ++i) {
      dump_to_buffer(
        subchunk_data[i], subchunk_sizes[i], static_cast<void*>(payload), bytes_copied_chunk, chunk_length);
      bytes_copied_chunk += subchunk_sizes[i];
    }
    fdreadoutlibs::types::VariableSizePayloadWrapper payload_wrapper(chunk_length, payload);
    try {
      sink->send(payload_wrapper, timeout);
    } catch (const dunedaq::iomanager::TimeoutExpired& excpt) {
      // ers
    }
  };
}

inline std::function<void(const felix::packetformat::shortchunk& shortchunk)>
varsizedShortchunkIntoWrapper(std::shared_ptr<iomanager::SenderConcept<fdreadoutlibs::types::VariableSizePayloadWrapper>>& sink,
                              std::chrono::milliseconds timeout = std::chrono::milliseconds(100))
{
  return [&](const felix::packetformat::shortchunk& shortchunk) {
    auto shortchunk_length = shortchunk.length;
    char* payload = static_cast<char*>(malloc(shortchunk_length * sizeof(char)));
    std::memcpy(payload, shortchunk.data, shortchunk_length);
    fdreadoutlibs::types::VariableSizePayloadWrapper payload_wrapper(shortchunk_length, payload);
    try {
      sink->send(payload_wrapper, timeout);
    } catch (const dunedaq::iomanager::TimeoutExpired& excpt) {
      // ers
    }
  };
}

inline std::function<void(const felix::packetformat::chunk& chunk)>
errorChunkIntoSink(std::shared_ptr<iomanager::SenderConcept<felix::packetformat::chunk>>& sink,
                   std::chrono::milliseconds timeout = std::chrono::milliseconds(100))
{
  return [&](const felix::packetformat::chunk& chunk) {
    try {
#warning GLM -> This is not safe since chunk may not be valid after send!
      //sink->send(chunk, timeout);
    } catch (const dunedaq::iomanager::TimeoutExpired& excpt) {
      // ers
    }
  };
}

//// Implement here any other DUNE specific FELIX chunk/block to User payload parsers

} // namespace parsers
} // namespace flxlibs
} // namespace dunedaq

#endif // FLXLIBS_INCLUDE_FLXLIBS_AVAILABLEPARSEROPERATIONS_HPP_
