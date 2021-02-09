/**
 * @file FelixCardReader.cc FelixCardReader class
 * implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include "FelixCardReader.hpp"
#include "FelixIssues.hpp"

#include "flxcard/FlxException.h"

#include <chrono>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <TRACE/trace.h>
/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "FelixCardReader" // NOLINT

namespace dunedaq {
namespace flxlibs {

FelixCardReader::FelixCardReader(const std::string& name)
  : DAQModule(name)
  , m_configured(false)
  //, block_ptr_sinks_{ } 

{

  register_command("conf", &FelixCardReader::do_configure);
  register_command("start", &FelixCardReader::do_start);
  register_command("stop", &FelixCardReader::do_stop);
}

void
FelixCardReader::init(const data_t& args)
{
/*
  auto ini = args.get<appfwk::cmd::ModInit>();
  for (const auto& qi : ini.qinfos) {
    if (qi.name == "output") {
      ERS_INFO("CardReader output queue is " << qi.inst);
    } else {
      ers::error(InitializationError(ERS_HERE, "Only output queues are supported in this module!"));
    }
  }
  std::vector<std::string> queue_names; // = get_config()["outputs"].get<std::vector<std::string>>();

  dma_memory_size_ = 4096*1024*1024UL;
  card_id_ = 0; //init_data.get<uint8_t>("card_id", 0);
  card_offset_ = 0; //get_config().value<uint8_t>("card_offset", 0);
  dma_id_ = 0; //get_config().value<uint8_t>("dma_id", 0);
  numa_id_ = 0; //get_config().value<uint8_t>("numa_id", 0);
  num_sources_ = 1; //get_config().value<uint8_t>("num_sources", 1);
  num_links_ = 6; //get_config().value<uint8_t>("num_links", M_MAX_LINKS_PER_CARD);

  //ERS_INFO("Base parameters initialized: " << get_config().dump()); 
*/


/*
  if (queue_names.size() != num_links_) {
    ers::error(readout::ConfigurationError(ERS_HERE, "Number of links does not match number of output queues."));
  } else {
    for (unsigned i=0; i<queue_names.size(); ++i){
     block_ptr_sinks_[i] = std::make_unique<BlockPtrSink>(queue_names[i]);
     ERS_INFO("Added BlockPtr DAQSink for link[" << i << "].");
    }
  }
*/
}

void
FelixCardReader::do_configure(const data_t& /*args*/)
{

}

void
FelixCardReader::do_start(const data_t& /*args*/)
{

}

void
FelixCardReader::do_stop(const data_t& /*args*/)
{

}

} // namespace flxlibs
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::flxlibs::FelixCardReader)
