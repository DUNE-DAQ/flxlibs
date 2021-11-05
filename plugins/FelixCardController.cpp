/**
 * @file FelixCardController.cpp FelixCardController DAQModule implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include "flxlibs/felixcardcontroller/Nljs.hpp"

#include "FelixCardController.hpp"

#include "logging/Logging.hpp"

#include "regmap/regmap.h"

#include <iomanip>
#include <memory>
#include <string>

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
  m_card_wrapper = std::make_unique<CardControllerWrapper>();

  register_command("conf", &FelixCardController::do_configure);
  register_command("getregister", &FelixCardController::get_reg);
  register_command("setregister", &FelixCardController::set_reg);
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
  m_card_wrapper->configure(args);
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

} // namespace flxlibs
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::flxlibs::FelixCardController)
