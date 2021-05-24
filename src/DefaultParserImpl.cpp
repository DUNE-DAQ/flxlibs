/**
 * @file DefaultParserImpl.cpp FELIX's packetformat default block/chunk parser
 * implementation.
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
// From Module
#include "DefaultParserImpl.hpp"

// From STD
#include <chrono>
#include <iomanip>

using namespace std::chrono_literals;

namespace dunedaq {
namespace flxlibs {

DefaultParserImpl::DefaultParserImpl()
{
  process_chunk_func = std::bind(&DefaultParserImpl::process_chunk, this, std::placeholders::_1);
  process_shortchunk_func = std::bind(&DefaultParserImpl::process_shortchunk, this, std::placeholders::_1);
  process_subchunk_func = std::bind(&DefaultParserImpl::process_subchunk, this, std::placeholders::_1);
  process_block_func = std::bind(&DefaultParserImpl::process_block, this, std::placeholders::_1);
  process_chunk_with_error_func = std::bind(&DefaultParserImpl::process_chunk_with_error, this, std::placeholders::_1);
  process_shortchunk_with_error_func =
    std::bind(&DefaultParserImpl::process_shortchunk_with_error, this, std::placeholders::_1);
  process_subchunk_with_error_func =
    std::bind(&DefaultParserImpl::process_subchunk_with_error, this, std::placeholders::_1);
  process_block_with_error_func = std::bind(&DefaultParserImpl::process_block_with_error, this, std::placeholders::_1);
}

DefaultParserImpl::~DefaultParserImpl() {}

stats::ParserStats&
DefaultParserImpl::get_stats()
{
  return std::ref(m_stats);
}

void
DefaultParserImpl::chunk_processed(const felix::packetformat::chunk& chunk)
{
  process_chunk_func(chunk);
  m_stats.chunk_ctr++;
}

void
DefaultParserImpl::shortchunk_processed(const felix::packetformat::shortchunk& shortchunk)
{
  process_shortchunk_func(shortchunk);
  m_stats.short_ctr++;
}

void
DefaultParserImpl::subchunk_processed(const felix::packetformat::subchunk& subchunk)
{
  process_subchunk_func(subchunk);
  m_stats.subchunk_ctr++;
}

void
DefaultParserImpl::block_processed(const felix::packetformat::block& block)
{
  process_block_func(block);
  m_stats.block_ctr++;
}

void
DefaultParserImpl::chunk_processed_with_error(const felix::packetformat::chunk& chunk)
{
  process_chunk_with_error_func(chunk);
  m_stats.error_chunk_ctr++;
}

void
DefaultParserImpl::subchunk_processed_with_error(const felix::packetformat::subchunk& subchunk)
{
  process_subchunk_with_error_func(subchunk);
  m_stats.error_subchunk_ctr++;
}

void
DefaultParserImpl::shortchunk_process_with_error(const felix::packetformat::shortchunk& shortchunk)
{
  process_shortchunk_with_error_func(shortchunk);
  m_stats.error_short_ctr++;
}

void
DefaultParserImpl::block_processed_with_error(const felix::packetformat::block& block)
{
  process_block_with_error_func(block);
  m_stats.error_block_ctr++;
}

} // namespace flxlibs
} // namespace dunedaq
