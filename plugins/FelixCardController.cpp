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
  , m_card_id(0)
  , m_logical_unit(0)
  , m_is_aligned(false)
{
  m_card_wrapper = std::make_unique<CardControllerWrapper>();

  register_command("conf", &FelixCardController::do_configure);
  register_command("getregister", &FelixCardController::get_reg);
  register_command("setregister", &FelixCardController::set_reg);
  register_command("getbitfield", &FelixCardController::get_bf);
  register_command("setbifield", &FelixCardController::set_bf);
  register_command("gthreset", &FelixCardController::gth_reset);
}

void
FelixCardController::init(const data_t& args)
{
  m_card_wrapper->init(args);
}

void
FelixCardController::do_configure(const data_t& args)
{
  m_cfg = args.get<felixcardcontroller::Conf>();
  m_card_id = m_cfg.card_id;
  m_logical_unit = m_cfg.logical_unit;
  m_card_wrapper->configure(args);
}

void
FelixCardController::get_info(opmonlib::InfoCollector& ci, int /*level*/)
{
  felixcardcontrollerinfo::ChannelInfo info;

  uint64_t aligned = m_card_wrapper->get_register(REG_GBT_ALIGNMENT_DONE);     // NOLINT(build/unsigned)
  uint64_t number_channels = m_card_wrapper->get_register(BF_NUM_OF_CHANNELS); // NOLINT(build/unsigned)

  std::vector<int> stats(number_channels, 0);
  size_t num_aligned = 0;
  auto index = m_logical_unit * number_channels;
  for (size_t i = 0; i < number_channels; ++i) {
    if (aligned & (1 << index)) {
      stats[i] = 1;
      num_aligned++;
    } else if (m_is_aligned) {
      m_is_aligned = false;
      ers::warning(ChannelAlignment(ERS_HERE, index));
    }
    index++;
  }

  m_is_aligned = (num_aligned < number_channels ? false : true);

  info.channel00_alignment_status = stats[0];
  info.channel01_alignment_status = stats[1];
  info.channel02_alignment_status = stats[2];
  info.channel03_alignment_status = stats[3];
  info.channel04_alignment_status = stats[4];
  info.channel05_alignment_status = stats[5];
  ci.add(info);
}

void
FelixCardController::get_reg(const data_t& args)
{
  auto conf = args.get<felixcardcontroller::GetRegisterParams>();
  for (auto reg_name : conf.reg_names) {
    auto reg_val = m_card_wrapper->get_register(reg_name);
    TLOG() << reg_name << "        0x" << std::hex << reg_val;
  }
}

void
FelixCardController::set_reg(const data_t& args)
{
  auto conf = args.get<felixcardcontroller::SetRegisterParams>();
  for (auto p : conf.reg_val_pairs) {
    m_card_wrapper->set_register(p.reg_name, p.reg_val);
  }
}

void
FelixCardController::get_bf(const data_t& args)
{
  auto conf = args.get<felixcardcontroller::GetBFParams>();
  for (auto bf_name : conf.bf_names) {
    auto bf_val = m_card_wrapper->get_bitfield(bf_name);
    TLOG() << bf_name << "        0x" << std::hex << bf_val;
  }
}

void
FelixCardController::set_bf(const data_t& args)
{
  auto conf = args.get<felixcardcontroller::SetBFParams>();
  for (auto p : conf.bf_val_pairs) {
    m_card_wrapper->set_bitfield(p.reg_name, p.reg_val);
  }
}

void
FelixCardController::gth_reset(const data_t& args)
{
  if (m_logical_unit != 0)
    return;

  auto conf = args.get<felixcardcontroller::GTHResetParams>();
  for (int i = 0; i < 6; ++i) {
    if (conf.quads & 1 << i)
      m_card_wrapper->gth_reset(i);
  }
}

} // namespace flxlibs
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::flxlibs::FelixCardController)
