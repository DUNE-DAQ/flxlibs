/**
 * @file CardControllerWrapper.hpp
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef FLXLIBS_SRC_CARDCONTROLLERWRAPPER_HPP_
#define FLXLIBS_SRC_CARDCONTROLLERWRAPPER_HPP_

#include "flxlibs/felixcardcontroller/Nljs.hpp"
#include "flxlibs/felixcardcontroller/Structs.hpp"

#include "flxcard/FlxCard.h"

#include <nlohmann/json.hpp>

#include <memory>
#include <mutex>
#include <string>

namespace dunedaq::flxlibs {

class CardControllerWrapper
{
public:
  /**
   * @brief CardControllerWrapper Constructor
   */
  CardControllerWrapper();
  ~CardControllerWrapper();
  CardControllerWrapper(const CardControllerWrapper&) = delete;            ///< Not copy-constructible
  CardControllerWrapper& operator=(const CardControllerWrapper&) = delete; ///< Not copy-assignable
  CardControllerWrapper(CardControllerWrapper&&) = delete;                 ///< Not move-constructible
  CardControllerWrapper& operator=(CardControllerWrapper&&) = delete;      ///< Not move-assignable

  using data_t = nlohmann::json;
  void init(const data_t& args);
  void configure(const data_t& args);

  uint64_t get_register(std::string key);             // NOLINT(build/unsigned)
  void set_register(std::string key, uint64_t value); // NOLINT(build/unsigned)
  uint64_t get_bitfield(std::string key);             // NOLINT(build/unsigned)
  void set_bitfield(std::string key, uint64_t value); // NOLINT(build/unsigned)
  void gth_reset(int quad);

private:
  // Types
  using module_conf_t = dunedaq::flxlibs::felixcardcontroller::Conf;

  // Card
  void open_card();
  void close_card();

  // Configuration and internals
  module_conf_t m_cfg;
  bool m_configured{ false };
  uint8_t m_card_id;      // NOLINT
  uint8_t m_logical_unit; // NOLINT
  std::string m_card_id_str;

  // Card object
  using UniqueFlxCard = std::unique_ptr<FlxCard>;
  UniqueFlxCard m_flx_card;
  std::mutex m_card_mutex;
};

} // namespace dunedaq::flxlibs

#endif // FLXLIBS_SRC_CARDCONTROLLERWRAPPER_HPP_
