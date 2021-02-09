// From Module
#include "DefaultParserImpl.hpp"

#include <ers/ers.h>

// From STD
#include <iomanip>
#include <chrono>

using namespace std::chrono_literals;

namespace dunedaq {
namespace flxlibs {

DefaultParserImpl::DefaultParserImpl()
{
  process_chunk_func =
    std::bind(&DefaultParserImpl::process_chunk, this, std::placeholders::_1);
  process_shortchunk_func =
    std::bind(&DefaultParserImpl::process_shortchunk, this, std::placeholders::_1);
  process_subchunk_func =
    std::bind(&DefaultParserImpl::process_subchunk, this, std::placeholders::_1);
  process_block_func =
    std::bind(&DefaultParserImpl::process_block, this, std::placeholders::_1);
  process_chunk_with_error_func =
    std::bind(&DefaultParserImpl::process_chunk_with_error, this, std::placeholders::_1);
  process_shortchunk_with_error_func =
    std::bind(&DefaultParserImpl::process_shortchunk_with_error, this, std::placeholders::_1);
  process_subchunk_with_error_func =
    std::bind(&DefaultParserImpl::process_subchunk_with_error, this, std::placeholders::_1);
  process_block_with_error_func =
    std::bind(&DefaultParserImpl::process_block_with_error, this, std::placeholders::_1);
}


DefaultParserImpl::~DefaultParserImpl()
{

}

void 
DefaultParserImpl::process_chunk(const felix::packetformat::chunk& /*chunk*/)
{

}

void 
DefaultParserImpl::process_shortchunk(const felix::packetformat::shortchunk& /*shortchunk*/)
{

}

void 
DefaultParserImpl::process_subchunk(const felix::packetformat::subchunk& /*subchunk*/)
{

}

void 
DefaultParserImpl::process_block(const felix::packetformat::block& /*block*/)
{

}

void 
DefaultParserImpl::process_chunk_with_error(const felix::packetformat::chunk& /*chunk*/)
{

}

void 
DefaultParserImpl::process_subchunk_with_error(const felix::packetformat::subchunk& /*subchunk*/)
{

}

void 
DefaultParserImpl::process_shortchunk_with_error(const felix::packetformat::shortchunk& /*shortchunk*/)
{

}

void 
DefaultParserImpl::process_block_with_error(const felix::packetformat::block& /*block*/)
{

}


} // namespace flxlibs
} // namespace dunedaq
