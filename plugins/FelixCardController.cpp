/**
 * @file FelixCardController.cpp FelixCardController DAQModule implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include "flxlibs/felixcardcontroller/Nljs.hpp"
#include "flxlibs/felixcardcontrollerinfo/InfoNljs.hpp"

#include "FelixCardController.hpp"
#include "FelixIssues.hpp"

#include "logging/Logging.hpp"

#include <nlohmann/json.hpp>

#include <bitset>
#include <iomanip>
#include <memory>
#include <string>
#include <vector>
#include <utility>

/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "FelixCardController" // NOLINT

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

FelixCardController::FelixCardController(const std::string& name)
  : DAQModule(name)
{
  //m_card_wrapper = std::make_unique<CardControllerWrapper>();

  register_command("conf", &FelixCardController::do_configure);
  register_command("start", &FelixCardController::gth_reset);
  register_command("getregister", &FelixCardController::get_reg);
  register_command("setregister", &FelixCardController::set_reg);
  register_command("getbitfield", &FelixCardController::get_bf);
  register_command("setbifield", &FelixCardController::set_bf);
  register_command("gthreset", &FelixCardController::gth_reset);
}

void
FelixCardController::init(const data_t& /*args*/)
{
}

void
FelixCardController::do_configure(const data_t& args)
{
  m_cfg = args.get<felixcardcontroller::Conf>();
  // get the number of misaligned links, this is needed since we only run init on device 0 but it checks alignment for both
  int misalignment_size = 0;
  for(auto lu : m_cfg.logical_units) {
    misalignment_size += lu.ignore_alignment_mask.size();
  }
  for (auto lu : m_cfg.logical_units) {
     uint32_t id = m_cfg.card_id+lu.log_unit_id;
     m_card_wrappers.emplace(std::make_pair(id,std::make_unique<CardControllerWrapper>(id)));
     if(m_card_wrappers.size() == 1) {
	 // Do the init only for the first device (whole card)
         m_card_wrappers.begin()->second->init(misalignment_size);
     }
     m_card_wrappers.at(m_cfg.card_id+lu.log_unit_id)->configure(lu);
  }
}

void
FelixCardController::get_info(opmonlib::InfoCollector& ci, int /*level*/)
{
  for (auto lu : m_cfg.logical_units) {
     uint32_t id = m_cfg.card_id+lu.log_unit_id;
     uint64_t aligned = m_card_wrappers.at(id)->get_register(REG_GBT_ALIGNMENT_DONE);
     std::vector<uint32_t> alignment_mask = lu.ignore_alignment_mask;
     for( auto li : lu.links) {
        std::stringstream info_name;
        info_name << "device_" << id << "_link_" << li.link_id; 

        // here we want to print out a log message when the links do not appear to be aligned.
        // for WIB readout link_id 5 is always reserved for tp links, so alignemnt is not expected fort these
        bool is_aligned = aligned & (1<<li.link_id);
        auto found_link = std::find(std::begin(alignment_mask), std::end(alignment_mask), li.link_id);
        if(found_link == std::end(alignment_mask)) {
          if(!lu.emu_fanout && !is_aligned) {
            ers::error(flxlibs::ChannelAlignment(ERS_HERE, li.link_id));
          }
        }

        felixcardcontrollerinfo::LinkInfo info;
        info.device_id = id;
        info.link_id = li.link_id;
        info.enabled = li.enabled;
        info.aligned = is_aligned;
        opmonlib::InfoCollector tmp_ic;
        tmp_ic.add(info);
        ci.add(info_name.str(),tmp_ic);
     }
  }
}

void
FelixCardController::get_reg(const data_t& args)
{
  auto conf = args.get<felixcardcontroller::GetRegisters>();
  auto id = conf.card_id + conf.log_unit_id;
  for (auto reg_name : conf.reg_names) {
    auto reg_val = m_card_wrappers.at(id)->get_register(reg_name);
    TLOG() << reg_name << "        0x" << std::hex << reg_val;
  }
}

void
FelixCardController::set_reg(const data_t& args)
{
  auto conf = args.get<felixcardcontroller::SetRegisters>();
  auto id = conf.card_id + conf.log_unit_id;

  for (auto p : conf.reg_val_pairs) {
    m_card_wrappers.at(id)->set_register(p.reg_name, p.reg_val);
  }
}

void
FelixCardController::get_bf(const data_t& args)
{
  auto conf = args.get<felixcardcontroller::GetBFs>();
  auto id = conf.card_id + conf.log_unit_id;

  for (auto bf_name : conf.bf_names) {
    auto bf_val = m_card_wrappers.at(id)->get_bitfield(bf_name);
    TLOG() << bf_name << "        0x" << std::hex << bf_val;
  }
}

void
FelixCardController::set_bf(const data_t& args)
{
  auto conf = args.get<felixcardcontroller::SetBFs>();
  auto id = conf.card_id + conf.log_unit_id;

  for (auto p : conf.bf_val_pairs) {
    m_card_wrappers.at(id)->set_bitfield(p.reg_name, p.reg_val);
  }
}

void
FelixCardController::gth_reset(const data_t& /*args*/)
{
  // Do the reset only for the first device (whole card)
  m_card_wrappers.begin()->second->gth_reset();
}

} // namespace flxlibs
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::flxlibs::FelixCardController)
