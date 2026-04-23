/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 *
 * This contains functions struct and other type definitions for shema in 
 * namespace dunedaq::flxlibs::felixcardcontroller to be serialized via nlohmann::json.
 */
#ifndef DUNEDAQ_FLXLIBS_FELIXCARDCONTROLLER_NLJS_HPP
#define DUNEDAQ_FLXLIBS_FELIXCARDCONTROLLER_NLJS_HPP

// My structs
#include "flxlibs/felixcardcontroller/Structs.hpp"


#include <nlohmann/json.hpp>

namespace dunedaq::flxlibs::felixcardcontroller {

    using data_t = nlohmann::json;
    
    inline void to_json(data_t& j, const Link& obj) {
        j["link_id"] = obj.link_id;
        j["enabled"] = obj.enabled;
        j["dma_desc"] = obj.dma_desc;
        j["superchunk_factor"] = obj.superchunk_factor;
    }
    
    inline void from_json(const data_t& j, Link& obj) {
        if (j.contains("link_id"))
            j.at("link_id").get_to(obj.link_id);    
        if (j.contains("enabled"))
            j.at("enabled").get_to(obj.enabled);    
        if (j.contains("dma_desc"))
            j.at("dma_desc").get_to(obj.dma_desc);    
        if (j.contains("superchunk_factor"))
            j.at("superchunk_factor").get_to(obj.superchunk_factor);    
    }
    
    inline void to_json(data_t& j, const LogicalUnit& obj) {
        j["log_unit_id"] = obj.log_unit_id;
        j["emu_fanout"] = obj.emu_fanout;
        j["links"] = obj.links;
        j["ignore_alignment_mask"] = obj.ignore_alignment_mask;
    }
    
    inline void from_json(const data_t& j, LogicalUnit& obj) {
        if (j.contains("log_unit_id"))
            j.at("log_unit_id").get_to(obj.log_unit_id);    
        if (j.contains("emu_fanout"))
            j.at("emu_fanout").get_to(obj.emu_fanout);    
        if (j.contains("links"))
            j.at("links").get_to(obj.links);    
        if (j.contains("ignore_alignment_mask"))
            j.at("ignore_alignment_mask").get_to(obj.ignore_alignment_mask);    
    }
    
    inline void to_json(data_t& j, const Conf& obj) {
        j["card_id"] = obj.card_id;
        j["logical_units"] = obj.logical_units;
    }
    
    inline void from_json(const data_t& j, Conf& obj) {
        if (j.contains("card_id"))
            j.at("card_id").get_to(obj.card_id);    
        if (j.contains("logical_units"))
            j.at("logical_units").get_to(obj.logical_units);    
    }
    
    inline void to_json(data_t& j, const GetBFs& obj) {
        j["card_id"] = obj.card_id;
        j["log_unit_id"] = obj.log_unit_id;
        j["bf_names"] = obj.bf_names;
    }
    
    inline void from_json(const data_t& j, GetBFs& obj) {
        if (j.contains("card_id"))
            j.at("card_id").get_to(obj.card_id);    
        if (j.contains("log_unit_id"))
            j.at("log_unit_id").get_to(obj.log_unit_id);    
        if (j.contains("bf_names"))
            j.at("bf_names").get_to(obj.bf_names);    
    }
    
    inline void to_json(data_t& j, const GetRegisters& obj) {
        j["card_id"] = obj.card_id;
        j["log_unit_id"] = obj.log_unit_id;
        j["reg_names"] = obj.reg_names;
    }
    
    inline void from_json(const data_t& j, GetRegisters& obj) {
        if (j.contains("card_id"))
            j.at("card_id").get_to(obj.card_id);    
        if (j.contains("log_unit_id"))
            j.at("log_unit_id").get_to(obj.log_unit_id);    
        if (j.contains("reg_names"))
            j.at("reg_names").get_to(obj.reg_names);    
    }
    
    inline void to_json(data_t& j, const RegValPair& obj) {
        j["reg_name"] = obj.reg_name;
        j["reg_val"] = obj.reg_val;
    }
    
    inline void from_json(const data_t& j, RegValPair& obj) {
        if (j.contains("reg_name"))
            j.at("reg_name").get_to(obj.reg_name);    
        if (j.contains("reg_val"))
            j.at("reg_val").get_to(obj.reg_val);    
    }
    
    inline void to_json(data_t& j, const SetBFs& obj) {
        j["card_id"] = obj.card_id;
        j["log_unit_id"] = obj.log_unit_id;
        j["bf_val_pairs"] = obj.bf_val_pairs;
    }
    
    inline void from_json(const data_t& j, SetBFs& obj) {
        if (j.contains("card_id"))
            j.at("card_id").get_to(obj.card_id);    
        if (j.contains("log_unit_id"))
            j.at("log_unit_id").get_to(obj.log_unit_id);    
        if (j.contains("bf_val_pairs"))
            j.at("bf_val_pairs").get_to(obj.bf_val_pairs);    
    }
    
    inline void to_json(data_t& j, const SetRegisters& obj) {
        j["card_id"] = obj.card_id;
        j["log_unit_id"] = obj.log_unit_id;
        j["reg_val_pairs"] = obj.reg_val_pairs;
    }
    
    inline void from_json(const data_t& j, SetRegisters& obj) {
        if (j.contains("card_id"))
            j.at("card_id").get_to(obj.card_id);    
        if (j.contains("log_unit_id"))
            j.at("log_unit_id").get_to(obj.log_unit_id);    
        if (j.contains("reg_val_pairs"))
            j.at("reg_val_pairs").get_to(obj.reg_val_pairs);    
    }
    
} // namespace dunedaq::flxlibs::felixcardcontroller

#endif // DUNEDAQ_FLXLIBS_FELIXCARDCONTROLLER_NLJS_HPP