/**
 * @file CardControllerWrapper.cpp
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
// From Module
#include "CardControllerWrapper.hpp"
#include "FelixDefinitions.hpp"
#include "FelixIssues.hpp"

#include "logging/Logging.hpp"

#include "flxcard/FlxException.h"

// From STD
#include <iomanip>
#include <memory>
#include <string>

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

CardControllerWrapper::CardControllerWrapper()
{}

CardControllerWrapper::~CardControllerWrapper()
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS)
    << "CardControllerWrapper destructor called. First stop check, then closing card.";
  close_card();
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << "CardControllerWrapper destroyed.";
}

void
CardControllerWrapper::init(const data_t& /*args*/)
{
  m_flx_card = std::make_unique<FlxCard>();
  if (m_flx_card == nullptr) {
    ers::fatal(flxlibs::CardError(ERS_HERE, "Couldn't create FlxCard object."));
  }
}

void
CardControllerWrapper::configure(const data_t& args)
{
  if (m_configured) {
    TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << "Card is already configured! Won't touch it.";
  } else {
    // Load config
    m_cfg = args.get<felixcardcontroller::Conf>();
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Configuring CardControllerWrapper of card " << m_cfg.card_id;
    // Open card
    open_card();
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Card[" << m_cfg.card_id << "] opened.";
    TLOG_DEBUG(TLVL_WORK_STEPS) << m_cfg.card_id << "] is configured for datataking.";
    m_configured = true;
  }
}

void
CardControllerWrapper::open_card()
{
  TLOG_DEBUG(TLVL_WORK_STEPS) << "Opening FELIX card " << m_cfg.card_id;
  try {
    m_card_mutex.lock();
    m_flx_card->card_open(static_cast<int>(m_cfg.card_id), LOCK_NONE); // FlxCard.h
    m_card_mutex.unlock();
  } catch (FlxException& ex) {
    ers::error(flxlibs::CardError(ERS_HERE, ex.what()));
    exit(EXIT_FAILURE);
  }
}

void
CardControllerWrapper::close_card()
{
  TLOG_DEBUG(TLVL_WORK_STEPS) << "Closing FELIX card " << m_cfg.card_id;
  try {
    m_card_mutex.lock();
    m_flx_card->card_close();
    m_card_mutex.unlock();
  } catch (FlxException& ex) {
    ers::error(flxlibs::CardError(ERS_HERE, ex.what()));
    exit(EXIT_FAILURE);
  }
}

uint64_t // NOLINT(build/unsigned)
CardControllerWrapper::get_register(std::string key)
{
  TLOG_DEBUG(TLVL_WORK_STEPS) << "Reading value of register " << key;
  m_card_mutex.lock();
  auto reg_val = m_flx_card->cfg_get_reg(key.c_str());
  m_card_mutex.unlock();
  return reg_val;
}

void
CardControllerWrapper::set_register(std::string key, uint64_t value) // NOLINT(build/unsigned)
{
  TLOG_DEBUG(TLVL_WORK_STEPS) << "Setting value of register " << key;
  m_card_mutex.lock();
  m_flx_card->cfg_set_reg(key.c_str(), value);
  m_card_mutex.unlock();
}

uint64_t // NOLINT(build/unsigned)
CardControllerWrapper::get_bitfield(std::string key)
{
  TLOG_DEBUG(TLVL_WORK_STEPS) << "Reading value of bitfield " << key;
  m_card_mutex.lock();
  auto bf_val = m_flx_card->cfg_get_option(key.c_str(), false);
  m_card_mutex.unlock();
  return bf_val;
}

void
CardControllerWrapper::set_bitfield(std::string key, uint64_t value) // NOLINT(build/unsigned)
{
  TLOG_DEBUG(TLVL_WORK_STEPS) << "Setting value of bitfield " << key;
  m_card_mutex.lock();
  m_flx_card->cfg_set_option(key.c_str(), value, false);
  m_card_mutex.unlock();
}

void
CardControllerWrapper::gth_reset()
{
  TLOG_DEBUG(TLVL_WORK_STEPS) << "Resetting GTH";
  m_card_mutex.lock();
  for (auto i=0 ; i< 6; ++i) {
      m_flx_card->gth_rx_reset(i);
  }    
  m_card_mutex.unlock();
}

} // namespace flxlibs
} // namespace dunedaq
