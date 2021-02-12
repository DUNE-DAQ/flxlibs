/**
* @file ElinkConcept.hpp ElinkConcept for constructors and
* forwarding command args.
*
* This is part of the DUNE DAQ , copyright 2020.
* Licensing/copyright details are in the COPYING file that you should have
* received with this code.
*/
#ifndef FLXLIBS_SRC_ELINKCONCEPT_HPP_
#define FLXLIBS_SRC_ELINKCONCEPT_HPP_

#include "DefaultParserImpl.hpp"

#include "packetformat/detail/block_parser.hpp"

#include <nlohmann/json.hpp>

namespace dunedaq {
namespace flxlibs {

class ElinkConcept {
public:
  explicit ElinkConcept()
    : m_parser_impl()
    , m_link_id(0)
    , m_link_tag(0)
  { 
    m_parser = std::make_unique<felix::packetformat::BlockParser<DefaultParserImpl>>( m_parser_impl );
  }
  ~ElinkConcept() {}

  ElinkConcept(const ElinkConcept&)
    = delete; ///< ElinkConcept is not copy-constructible
  ElinkConcept& operator=(const ElinkConcept&)
    = delete; ///< ElinkConcept is not copy-assginable
  ElinkConcept(ElinkConcept&&)
    = delete; ///< ElinkConcept is not move-constructible
  ElinkConcept& operator=(ElinkConcept&&)
    = delete; ///< ElinkConcept is not move-assignable

  virtual void init(const nlohmann::json& args) = 0;
  virtual void conf(const nlohmann::json& args) = 0;
  virtual void start(const nlohmann::json& args) = 0;
  virtual void stop(const nlohmann::json& args) = 0;

  virtual bool queue_in_block(uint64_t block_addr) = 0;

  DefaultParserImpl& get_parser() { 
    return std::ref(m_parser_impl); 
  }

  void set_ids(unsigned id, unsigned tag) {
    m_link_id = id;
    m_link_tag = tag;
  }


protected:
  // Block Parser
  DefaultParserImpl m_parser_impl;
  std::unique_ptr<felix::packetformat::BlockParser<DefaultParserImpl>> m_parser;

  unsigned m_link_id;
  unsigned m_link_tag;

private:

};

} // namespace readout
} // namespace dunedaq

#endif // FLXLIBS_SRC_ELINKCONCEPT_HPP_
