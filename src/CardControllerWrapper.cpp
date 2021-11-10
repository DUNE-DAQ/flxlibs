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
  : m_card_id(0)
  , m_logical_unit(0)
  , m_card_id_str("")
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
    m_card_id = m_cfg.card_id;
    m_logical_unit = m_cfg.logical_unit;

    std::ostringstream cardoss;
    cardoss << "[id:" << std::to_string(m_card_id) << " slr:" << std::to_string(m_logical_unit) << "]";
    m_card_id_str = cardoss.str();
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Configuring CardControllerWrapper of card " << m_card_id_str;
    // Open card
    open_card();
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Card[" << m_card_id_str << "] opened.";
    TLOG_DEBUG(TLVL_WORK_STEPS) << m_card_id_str << "] is configured for datataking.";
    m_configured = true;
  }
}

void
CardControllerWrapper::open_card()
{
  TLOG_DEBUG(TLVL_WORK_STEPS) << "Opening FELIX card " << m_card_id_str;
  try {
    m_card_mutex.lock();
    auto absolute_card_id = m_card_id + m_logical_unit;
    m_flx_card->card_open(static_cast<int>(absolute_card_id), LOCK_NONE); // FlxCard.h
    m_card_mutex.unlock();
  } catch (FlxException& ex) {
    ers::error(flxlibs::CardError(ERS_HERE, ex.what()));
    exit(EXIT_FAILURE);
  }
}

void
CardControllerWrapper::close_card()
{
  TLOG_DEBUG(TLVL_WORK_STEPS) << "Closing FELIX card " << m_card_id_str;
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
  m_flx_card->cfg_set_reg(key.c_str(), value); // handle invalid inputs
  m_card_mutex.unlock();
}

void
CardControllerWrapper::gth_reset(int quad = -1)
{
  TLOG_DEBUG(TLVL_WORK_STEPS) << "Resetting GTH quad " << quad;
  m_card_mutex.lock();
  m_flx_card->gth_rx_reset(quad);
  m_card_mutex.unlock();
}

} // namespace flxlibs
} // namespace dunedaq
