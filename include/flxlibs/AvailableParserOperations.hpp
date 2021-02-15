/**
* @file AvailableParserOperations.hpp Implementations of FELIX data format
* parsers into user types.
*
* This is part of the DUNE DAQ , copyright 2020.
* Licensing/copyright details are in the COPYING file that you should have
* received with this code.
*/
#ifndef FLXLIBS_SRC_AVAILABLEPARSEROPERATIONS_HPP_
#define FLXLIBS_SRC_AVAILABLEPARSEROPERATIONS_HPP_

#include "readout/ReadoutTypes.hpp"

namespace dunedaq {
namespace flxlibs {
namespace parsers {

inline void
dump_to_buffer(const char* data, long unsigned size,
               void* buffer, long unsigned buffer_pos,
               const long unsigned& buffer_size)
{
  long unsigned bytes_to_copy = size;
  while(bytes_to_copy > 0) {
    unsigned int n = std::min(bytes_to_copy, buffer_size-buffer_pos);
    std::memcpy(static_cast<char*>(buffer)+buffer_pos, data, n);
    buffer_pos += n;
    bytes_to_copy -= n;
    if(buffer_pos == buffer_size) {
      buffer_pos = 0;
    }
  }
}

template<class TargetStruct>
inline std::function<void(const felix::packetformat::chunk& chunk)>
fixsizedChunkInto(std::unique_ptr<appfwk::DAQSink<std::unique_ptr<TargetStruct>>>& sink, 
                  std::chrono::milliseconds timeout = std::chrono::milliseconds(100))
{
  return [&](const felix::packetformat::chunk& chunk) { 
    long unsigned bytes_copied_chunk = 0;
    auto subchunk_data = chunk.subchunks();
    auto subchunk_sizes = chunk.subchunk_lengths();
    auto n_subchunks = chunk.subchunk_number();
    std::unique_ptr<TargetStruct> payload_ptr = std::make_unique<TargetStruct>();
    auto target_size = sizeof(*payload_ptr);
    for(unsigned i=0; i<n_subchunks; i++)
    {
      dump_to_buffer(subchunk_data[i], subchunk_sizes[i],
                     static_cast<void*>(&payload_ptr),
                     bytes_copied_chunk, 
                     target_size);
      bytes_copied_chunk += subchunk_sizes[i];
    }
    try {
      sink->push(std::move(payload_ptr), timeout);
    } catch (...) {
      
    }
  };
}

//// Implement here any other DUNE specific FELIX chunk/block to User payload parsers

} // namespace parsers
} // namespace flxlibs
} // namespace dunedaq

#endif // FLXLIBS_SRC_AVAILABLEPARSEROPERATIONS_HPP_
