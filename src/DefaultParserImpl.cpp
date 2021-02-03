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
  pre_process_chunk_func =
    std::bind(&DefaultParserImpl::pre_process_chunk, this, std::placeholders::_1);
  pre_process_shortchunk_func =
    std::bind(&DefaultParserImpl::pre_process_shortchunk, this, std::placeholders::_1);
  pre_process_subchunk_func =
    std::bind(&DefaultParserImpl::pre_process_subchunk, this, std::placeholders::_1);
  pre_process_block_func =
    std::bind(&DefaultParserImpl::pre_process_block, this, std::placeholders::_1);
  pre_process_chunk_with_error_func =
    std::bind(&DefaultParserImpl::pre_process_chunk_with_error, this, std::placeholders::_1);
  pre_process_shortchunk_with_error_func =
    std::bind(&DefaultParserImpl::pre_process_shortchunk_with_error, this, std::placeholders::_1);
  pre_process_subchunk_with_error_func =
    std::bind(&DefaultParserImpl::pre_process_subchunk_with_error, this, std::placeholders::_1);
  pre_process_block_with_error_func =
    std::bind(&DefaultParserImpl::pre_process_block_with_error, this, std::placeholders::_1);

  post_process_chunk_func =
    std::bind(&DefaultParserImpl::post_process_chunk, this, std::placeholders::_1);
  post_process_shortchunk_func =
    std::bind(&DefaultParserImpl::post_process_shortchunk, this, std::placeholders::_1);
  post_process_subchunk_func =
    std::bind(&DefaultParserImpl::post_process_subchunk, this, std::placeholders::_1);
  post_process_block_func =
    std::bind(&DefaultParserImpl::post_process_block, this, std::placeholders::_1);
  post_process_chunk_with_error_func =
    std::bind(&DefaultParserImpl::post_process_chunk_with_error, this, std::placeholders::_1);
  post_process_shortchunk_with_error_func =
    std::bind(&DefaultParserImpl::post_process_shortchunk_with_error, this, std::placeholders::_1);
  post_process_subchunk_with_error_func =
    std::bind(&DefaultParserImpl::post_process_subchunk_with_error, this, std::placeholders::_1);
  post_process_block_with_error_func =
    std::bind(&DefaultParserImpl::post_process_block_with_error, this, std::placeholders::_1);

  std::cout << " A DEFAULT PARSER IMPLEMENTATION WAS CREATED \n";
}


DefaultParserImpl::~DefaultParserImpl()
{

}

void 
DefaultParserImpl::pre_process_chunk(const felix::packetformat::chunk& /*chunk*/)
{

}

void 
DefaultParserImpl::pre_process_shortchunk(const felix::packetformat::shortchunk& /*shortchunk*/)
{

}

void 
DefaultParserImpl::pre_process_subchunk(const felix::packetformat::subchunk& /*subchunk*/)
{

}

void 
DefaultParserImpl::pre_process_block(const felix::packetformat::block& /*block*/)
{

}

void 
DefaultParserImpl::pre_process_chunk_with_error(const felix::packetformat::chunk& /*chunk*/)
{

}

void 
DefaultParserImpl::pre_process_subchunk_with_error(const felix::packetformat::subchunk& /*subchunk*/)
{

}

void 
DefaultParserImpl::pre_process_shortchunk_with_error(const felix::packetformat::shortchunk& /*shortchunk*/)
{

}

void 
DefaultParserImpl::pre_process_block_with_error(const felix::packetformat::block& /*block*/)
{

}

//// POST PROCESS

void 
DefaultParserImpl::post_process_chunk(const felix::packetformat::chunk& /*chunk*/)
{

}

void 
DefaultParserImpl::post_process_shortchunk(const felix::packetformat::shortchunk& /*shortchunk*/)
{

}

void 
DefaultParserImpl::post_process_subchunk(const felix::packetformat::subchunk& /*subchunk*/)
{

}

void 
DefaultParserImpl::post_process_block(const felix::packetformat::block& /*block*/)
{

}

void 
DefaultParserImpl::post_process_chunk_with_error(const felix::packetformat::chunk& /*chunk*/)
{

}

void 
DefaultParserImpl::post_process_subchunk_with_error(const felix::packetformat::subchunk& /*subchunk*/)
{

}

void 
DefaultParserImpl::post_process_shortchunk_with_error(const felix::packetformat::shortchunk& /*shortchunk*/)
{

}

void 
DefaultParserImpl::post_process_block_with_error(const felix::packetformat::block& /*block*/)
{

}


} // namespace flxlibs
} // namespace dunedaq
