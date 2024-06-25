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
#include "datahandlinglibs/DataHandlingIssues.hpp"
//#include "fdreadoutlibs/ProtoWIBSuperChunkTypeAdapter.hpp"
//#include "fdreadoutlibs/DUNEWIBSuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/DAPHNESuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/DAPHNEStreamSuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/VariableSizePayloadTypeAdapter.hpp"

#include <memory>
#include <string>

namespace dunedaq {

//DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::ProtoWIBSuperChunkTypeAdapter, "WIBFrame")
//DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DUNEWIBSuperChunkTypeAdapter, "WIB2Frame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DAPHNESuperChunkTypeAdapter, "PDSFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DAPHNEStreamSuperChunkTypeAdapter, "PDSStreamFrame")

namespace flxlibs {

std::unique_ptr<ElinkConcept>
createElinkModel(const std::string& conn_uid)
{
  auto datatypes = dunedaq::iomanager::IOManager::get()->get_datatypes(conn_uid);
  if (datatypes.size() != 1) {
    ers::error(dunedaq::datahandlinglibs::GenericConfigurationError(ERS_HERE,
      "Multiple output data types specified! Expected only a single type!"));
  }
  std::string raw_dt{ *datatypes.begin() };
  TLOG() << "Choosing specializations for ElinkModel for output connection "
         << " [uid:" << conn_uid << " , data_type:" << raw_dt << ']';
/*

  if (raw_dt.find("WIBFrame") != std::string::npos) {
    // WIB1 specific char arrays
    // Create Model
    auto elink_model = std::make_unique<ElinkModel<fdreadoutlibs::types::ProtoWIBSuperChunkTypeAdapter>>();

    // Setup sink (acquire pointer from QueueRegistry)
    elink_model->set_sink(conn_uid);

    // Get parser and sink
    auto& parser = elink_model->get_parser();
    auto& sink = elink_model->get_sink();
    auto& error_sink = elink_model->get_error_sink();

    // Modify parser as needed...
    parser.process_chunk_func = parsers::fixsizedChunkInto<fdreadoutlibs::types::ProtoWIBSuperChunkTypeAdapter>(sink);
    if (error_sink != nullptr) {
      parser.process_chunk_with_error_func = parsers::errorChunkIntoSink(error_sink);
    }
    // parser.process_block_func = ...

    // Return with setup model
    return elink_model;

  } else if (raw_dt.find("WIB2Frame") != std::string::npos) {
    // WIB2 specific char arrays
    auto elink_model = std::make_unique<ElinkModel<fdreadoutlibs::types::DUNEWIBSuperChunkTypeAdapter>>();
    elink_model->set_sink(conn_uid);
    auto& parser = elink_model->get_parser();
    auto& sink = elink_model->get_sink();
    parser.process_chunk_func = parsers::fixsizedChunkInto<fdreadoutlibs::types::DUNEWIBSuperChunkTypeAdapter>(sink);
    return elink_model;

  } else*/ 
  if (raw_dt.find("PDSStreamFrame") != std::string::npos) {
    // PDS specific char arrays
    auto elink_model = std::make_unique<ElinkModel<fdreadoutlibs::types::DAPHNEStreamSuperChunkTypeAdapter>>();
    elink_model->set_sink(conn_uid);
    auto& parser = elink_model->get_parser();
    auto& sink = elink_model->get_sink();
    parser.process_chunk_func = parsers::fixsizedChunkInto<fdreadoutlibs::types::DAPHNEStreamSuperChunkTypeAdapter>(sink);
    return elink_model;

  } else if (raw_dt.find("PDSFrame") != std::string::npos) {
    // PDS specific char arrays
    auto elink_model = std::make_unique<ElinkModel<fdreadoutlibs::types::DAPHNESuperChunkTypeAdapter>>();
    elink_model->set_sink(conn_uid);
    auto& parser = elink_model->get_parser();
    auto& sink = elink_model->get_sink();
    parser.process_chunk_func = parsers::fixsizedChunkInto<fdreadoutlibs::types::DAPHNESuperChunkTypeAdapter>(sink);
    return elink_model;


  } else if (raw_dt.find("varsize") != std::string::npos) {
    // Variable sized user payloads
    auto elink_model = std::make_unique<ElinkModel<fdreadoutlibs::types::VariableSizePayloadTypeAdapter>>();
    elink_model->set_sink(conn_uid);
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
