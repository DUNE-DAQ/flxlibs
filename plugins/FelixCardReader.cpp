/**
 * @file FelixCardReader.cc FelixCardReader DAQModule implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include "flxlibs/felixcardreader/Nljs.hpp"

#include "CreateElink.hpp"
#include "FelixCardReader.hpp"
#include "FelixIssues.hpp"

#include "logging/Logging.hpp"

#include "flxcard/FlxException.h"

#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "FelixCardReader" // NOLINT

/**
 * @brief TRACE debug levels used in this source file
 */
enum
{
  TLVL_ENTER_EXIT_METHODS = 5,
  TLVL_WORK_STEPS = 10,
  TLVL_BOOKKEEPING = 15
};

namespace dunedaq {
namespace flxlibs {

FelixCardReader::FelixCardReader(const std::string& name)
  : DAQModule(name)
  , m_configured(false)
  , m_card_id(0)
  , m_logical_unit(0)
  , m_links_enabled({ 0 })
  , m_num_links(0)
  , m_block_size(0)
  , m_block_router(nullptr)
//, block_ptr_sinks_{ }

{
  m_card_wrapper = std::make_unique<CardWrapper>();

  register_command("conf", &FelixCardReader::do_configure);
  register_command("start", &FelixCardReader::do_start);
  register_command("stop_trigger_sources", &FelixCardReader::do_stop);
}

inline void
tokenize(std::string const& str, const char delim, std::vector<std::string>& out)
{
  std::size_t start;
  std::size_t end = 0;
  while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
    end = str.find(delim, start);
    out.push_back(str.substr(start, end - start));
  }
}

void
FelixCardReader::init(const data_t& args)
{
  auto ini = args.get<appfwk::app::ModInit>();
  m_card_wrapper->init(args);
  for (const auto& qi : ini.conn_refs) {
    if (qi.uid == "errored_chunks_q") {
      continue;
    } else {
      TLOG_DEBUG(TLVL_WORK_STEPS) << ": CardReader output queue is " << qi.uid;
      const char delim = '_';
      std::string target = qi.uid;
      std::vector<std::string> words;
      tokenize(target, delim, words);
      int linkid = -1;
      try {
        linkid = std::stoi(words.back());
      } catch (const std::exception& ex) {
        ers::fatal(InitializationError(ERS_HERE, "Link ID could not be parsed on queue instance name! "));
      }
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating ElinkModel for target queue: " << target << " DLH number: " << linkid;
      m_elinks[linkid] = createElinkModel(qi.uid);
      if (m_elinks[linkid] == nullptr) {
        ers::fatal(InitializationError(ERS_HERE, "CreateElink failed to provide an appropriate model for queue!"));
      }
      m_elinks[linkid]->init(args, m_block_queue_capacity);
    }
  }

  // Router function of block to appropriate ElinkHandlers
  m_block_router = [&](uint64_t block_addr) { // NOLINT
    // block_counter++;
    const auto* block = const_cast<felix::packetformat::block*>(
      felix::packetformat::block_from_bytes(reinterpret_cast<const char*>(block_addr)) // NOLINT
    );
    auto elink = block->elink;
    if (m_elinks.count(elink) != 0) {
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

      // NO TLOG_DEBUG, but should count and periodically report corrupted DMA blocks.
    }
  };

  // Set function for the CardWrapper's block processor.
  m_card_wrapper->set_block_addr_handler(m_block_router);
}

void
FelixCardReader::do_configure(const data_t& args)
{
  m_cfg = args.get<felixcardreader::Conf>();
  m_card_id = m_cfg.card_id;
  m_logical_unit = m_cfg.logical_unit;
  m_links_enabled = m_cfg.links_enabled;
  m_num_links = m_links_enabled.size();
  m_block_size = m_cfg.dma_block_size_kb * m_1kb_block_size;
  m_chunk_trailer_size = m_cfg.chunk_trailer_size;
  bool is_32b_trailer = false;

  TLOG(TLVL_BOOKKEEPING) << "Number of elinks specified in configuration: " << m_links_enabled.size();
  TLOG(TLVL_BOOKKEEPING) << "Number of data link handlers: " << m_elinks.size();

  // Config checks
  if (m_num_links != m_elinks.size()) {
    ers::fatal(ElinkConfigurationInconsistency(ERS_HERE, m_links_enabled.size()));
  }
  if (m_block_size % m_1kb_block_size != 0) {
    ers::fatal(BlockSizeConfigurationInconsistency(ERS_HERE, m_block_size));
  } else if (m_block_size != m_1kb_block_size && m_chunk_trailer_size != m_32b_trailer_size) {
    ers::fatal(BlockSizeConfigurationInconsistency(ERS_HERE, m_block_size));
  } else if (m_chunk_trailer_size == m_32b_trailer_size) {
    is_32b_trailer = true;
  }

  // Configure components
  TLOG(TLVL_WORK_STEPS) << "Card ID: " << m_card_id;
  TLOG(TLVL_WORK_STEPS) << "Configuring components with Block size:" << m_block_size
                        << " & trailer size: " << m_chunk_trailer_size;
  m_card_wrapper->configure(args);
  // get linkids defined by queues
  std::vector<int> linkids;
  for (auto& [id, elink] : m_elinks) {
    linkids.push_back(id);
  }
  // loop through all elinkmodels, change the linkids to link tags and configure
  for (unsigned i = 0; i < m_num_links; ++i) {
    auto elink = m_elinks.extract(linkids[i]);
    auto tag = m_links_enabled[i] * m_elink_multiplier;
    elink.key() = tag;
    m_elinks.insert(std::move(elink));
    m_elinks[tag]->set_ids(m_card_id, m_logical_unit, m_links_enabled[i], tag);
    m_elinks[tag]->conf(args, m_block_size, is_32b_trailer);
  }
}

void
FelixCardReader::do_start(const data_t& args)
{
  m_card_wrapper->start(args);
  for (auto& [tag, elink] : m_elinks) {
    elink->start(args);
  }
}

void
FelixCardReader::do_stop(const data_t& args)
{
  m_card_wrapper->stop(args);
  for (auto& [tag, elink] : m_elinks) {
    elink->stop(args);
  }
}

void
FelixCardReader::get_info(opmonlib::InfoCollector& ci, int level)
{
  for (unsigned lid = 0; lid < m_num_links; ++lid) {
    auto tag = m_links_enabled[lid] * m_elink_multiplier;
    m_elinks[tag]->get_info(ci, level);
  }
}

} // namespace flxlibs
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::flxlibs::FelixCardReader)
