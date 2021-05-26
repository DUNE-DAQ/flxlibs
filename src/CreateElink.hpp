/**
 * @file CreateElink.hpp Specific ElinkConcept creator.
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef FLXLIBS_SRC_CREATEELINK_HPP_
#define FLXLIBS_SRC_CREATEELINK_HPP_

#include "ElinkConcept.hpp"
#include "ElinkModel.hpp"
#include "flxlibs/AvailableParserOperations.hpp"
#include "readout/ReadoutTypes.hpp"

#include <memory>
#include <string>

namespace dunedaq {
namespace flxlibs {

std::unique_ptr<ElinkConcept>
createElinkModel(const std::string& target)
{
  if (target.find("wib") != std::string::npos && target.find("wib2") == std::string::npos) {
    // WIB1 specific char arrays
    // Create Model
    auto elink_model = std::make_unique<ElinkModel<readout::types::WIB_SUPERCHUNK_STRUCT>>();

    // Setup sink (acquire pointer from QueueRegistry)
    elink_model->set_sink(target);

    // Get parser and sink
    auto& parser = elink_model->get_parser();
    auto& sink = elink_model->get_sink();

    // Modify parser as needed...
    parser.process_chunk_func = parsers::fixsizedChunkInto<readout::types::WIB_SUPERCHUNK_STRUCT>(sink);
    // parser.process_block_func = ...

    // Return with setup model
    return elink_model;

  } else if (target.find("wib2") != std::string::npos) {
    // WIB2 specific char arrays
    auto elink_model = std::make_unique<ElinkModel<readout::types::WIB2_SUPERCHUNK_STRUCT>>();
    elink_model->set_sink(target);
    auto& parser = elink_model->get_parser();
    auto& sink = elink_model->get_sink();
    parser.process_chunk_func = parsers::fixsizedChunkInto<readout::types::WIB2_SUPERCHUNK_STRUCT>(sink);
    return elink_model;

  } else if (target.find("pds") != std::string::npos) {
    // PDS specific char arrays
    auto elink_model = std::make_unique<ElinkModel<readout::types::PDS_SUPERCHUNK_STRUCT>>();
    elink_model->set_sink(target);
    auto& parser = elink_model->get_parser();
    auto& sink = elink_model->get_sink();
    parser.process_chunk_func = parsers::fixsizedChunkInto<readout::types::PDS_SUPERCHUNK_STRUCT>(sink);
    return elink_model;

  } else if (target.find("varsize") != std::string::npos) {
    // Variable sized user payloads
    auto elink_model = std::make_unique<ElinkModel<readout::types::VariableSizePayloadWrapper>>();
    elink_model->set_sink(target);
    auto& parser = elink_model->get_parser();
    auto& sink = elink_model->get_sink();
    parser.process_chunk_func = parsers::varsizedChunkIntoWrapper(sink);
    parser.process_shortchunk_func = parsers::varsizedShortchunkIntoWrapper(sink);
    return elink_model;
  }

  return nullptr;
}

} // namespace flxlibs
} // namespace dunedaq

#endif // FLXLIBS_SRC_CREATEELINK_HPP_
