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
  m_card_wrapper = std::make_unique<CardWrapper>();

  register_command("conf", &FelixCardController::do_configure);
  register_command("start", &FelixCardController::do_start);
  register_command("stop", &FelixCardController::do_stop);
}

void FelixCardController::init(const data_t& args)
{
  m_card_wrapper->init(args);
}

void FelixCardController::do_configure(const data_t& args)
{
  m_cfg = args.get<felixcardcontroller::Conf>();
//  m_card_wrapper->configure(args);
  TLOG() << "Hello from FelixCardController::do_configure()";
}

void FelixCardController::do_start(const data_t& args)
{
  m_card_wrapper->start(args);
}

void FelixCardController::do_stop(const data_t& args)
{
  m_card_wrapper->stop(args);
}

void FelixCardController::read_register()
{
  TLOG() << "Hello from FelixCardController::read_register()";
}

void FelixCardController::load_register()
{}

} // namespace flxlibs
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::flxlibs::FelixCardController)
