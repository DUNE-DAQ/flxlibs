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
#include "CreateElink.hpp"

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
  , m_block_router(nullptr)
  //, block_ptr_sinks_{ } 

{
  m_card_wrapper = std::make_unique<CardWrapper>();

  register_command("conf", &FelixCardReader::do_configure);
  register_command("start", &FelixCardReader::do_start);
  register_command("stop", &FelixCardReader::do_stop);
}

inline void
tokenize(std::string const &str, const char delim,
         std::vector<std::string>& out)
{
  size_t start;
  size_t end = 0;
  while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
    end = str.find(delim, start);
    out.push_back(str.substr(start, end - start));
  }
}

void
FelixCardReader::init(const data_t& args)
{
  auto ini = args.get<appfwk::cmd::ModInit>();
  for (const auto& qi : ini.qinfos) {
    if (qi.dir != "output") {
      //ers::error(InitializationError(ERS_HERE, "Only output queues are supported in this module!"));
      continue;
    } else {
      ERS_INFO("CardReader output queue is " << qi.inst);
      const char delim = '-';
      std::string target = qi.inst;
      std::vector<std::string> words;
      tokenize(target, delim, words);
      auto linkid = std::stoi(words.back());
      ERS_INFO(" -> ELINK id: " << linkid);
      m_elinks[linkid*64] = createElinkModel(qi.inst);
      m_elinks[linkid*64]->init(args);
    }
  }

  //std::vector<std::string> queue_names = args["outputs"].get<std::vector<std::string>>();
/*
  if (queue_names.size() != m_num_links_) {
    ers::error(readout::ConfigurationError(ERS_HERE, "Number of links does not match number of output queues."));
  } else {
    for (unsigned i=0; i<queue_names.size(); ++i){
     block_ptr_sinks_[i] = std::make_unique<BlockPtrSink>(queue_names[i]);
     ERS_INFO("Added BlockPtr DAQSink for link[" << i << "].");
    }
  }
*/

  m_card_id = 0; // from name?
  m_num_links = 6; // from output sinks?

  m_card_wrapper->init(args);

  // Router function of block to appropriate ElinkHandlers
  m_block_router = [&](uint64_t block_addr) {
    //block_counter++;
    const auto* block = const_cast<felix::packetformat::block*>(
      felix::packetformat::block_from_bytes(reinterpret_cast<const char*>(block_addr))
    );
    auto elink = block->elink;
    if(m_elinks.count(elink) != 0) {
      m_elinks[elink]->queue_in_block_address(block_addr);
    } else {
      // Really bad -> unexpeced ELINK ID in Block.
      // This check is needed in order to avoid dynamically add thousands
      // of ELink parser implementations on the fly, in case the data
      // corruption is extremely severe.
      //
      // Possible causes:
      //   -> enabled links that don't connect to anything
      //   -> unexpected format (fw/sw version missmatch)
      //   -> data corruption from FE
      //   -> data corruption from CR (really rare, last possible cause)
      
      // NO ERS_DEBUG, but should count and periodically report corrupted DMA blocks.
    }
  };

  // Set function for the CardWrapper's block processor.
  m_card_wrapper->set_block_addr_handler(m_block_router);
}

void
FelixCardReader::do_configure(const data_t& args)
{
  m_num_links = 5;
  m_card_wrapper->configure(args);
  for (int lid=0; lid<m_num_links; ++lid) {
    m_elinks[lid*64]->conf(args);
  } 
}

void
FelixCardReader::do_start(const data_t& args)
{
  m_card_wrapper->start(args);
  for (int lid=0; lid<m_num_links; ++lid) {
    m_elinks[lid*64]->start(args);
  } 
}

void
FelixCardReader::do_stop(const data_t& args)
{
  m_card_wrapper->stop(args);
  for (int lid=0; lid<m_num_links; ++lid) {
    m_elinks[lid*64]->stop(args);
  }
}

} // namespace flxlibs
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::flxlibs::FelixCardReader)
