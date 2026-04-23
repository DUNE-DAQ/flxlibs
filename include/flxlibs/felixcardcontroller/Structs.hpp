/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 *
 * This contains struct and other type definitions for shema in 
 * namespace dunedaq::flxlibs::felixcardcontroller.
 */
#ifndef DUNEDAQ_FLXLIBS_FELIXCARDCONTROLLER_STRUCTS_HPP
#define DUNEDAQ_FLXLIBS_FELIXCARDCONTROLLER_STRUCTS_HPP

#include <cstdint>

#include <vector>
#include <string>

namespace dunedaq::flxlibs::felixcardcontroller {

    // @brief An unsigned of 4 bytes
    using uint4 = uint32_t; // NOLINT


    // @brief list of numbers
    using Array = std::vector<dunedaq::flxlibs::felixcardcontroller::uint4>;

    // @brief A boolean
    using boolean = bool;

    // @brief An unsigned of 8 bytes
    using uint8 = uint64_t; // NOLINT


    // @brief 
    struct Link 
    {

        // @brief Link identifier
        uint4 link_id = 0;

        // @brief Indicate whether the link is enabled
        boolean enabled = false;

        // @brief DMA channel
        uint8 dma_desc = 0;

        // @brief Superchunk factor
        uint4 superchunk_factor = 0;
    };

    // @brief A list of links
    using LinksList = std::vector<dunedaq::flxlibs::felixcardcontroller::Link>;

    // @brief 
    struct LogicalUnit 
    {

        // @brief Logical unit identifier
        uint4 log_unit_id = 0;

        // @brief Toggle emulator on/off
        boolean emu_fanout = false;

        // @brief List of links in the logical unit
        LinksList links = {};

        // @brief Elinks to ignore when checking alignment.
        Array ignore_alignment_mask = {5};
    };

    // @brief A list of logical units
    using LogicalUnitList = std::vector<dunedaq::flxlibs::felixcardcontroller::LogicalUnit>;

    // @brief Upstream FELIX CardController DAQ Module Configuration
    struct Conf 
    {

        // @brief Physical card identifier (in the same host)
        uint4 card_id = 0;

        // @brief Superlogic regions of selected card
        LogicalUnitList logical_units = {};
    };

    // @brief A string field
    using String = std::string;

    // @brief A list of registers
    using RegList = std::vector<dunedaq::flxlibs::felixcardcontroller::String>;

    // @brief Bitfield access parameters
    struct GetBFs 
    {

        // @brief Physical card identifier (in the same host)
        uint4 card_id = 0;

        // @brief Logical unit identifier
        uint4 log_unit_id = 0;

        // @brief A list of bitfields
        RegList bf_names = {};
    };

    // @brief Register access parameters
    struct GetRegisters 
    {

        // @brief Physical card identifier (in the same host)
        uint4 card_id = 0;

        // @brief Logical unit identifier
        uint4 log_unit_id = 0;

        // @brief A list of registers
        RegList reg_names = {};
    };

    // @brief 
    struct RegValPair 
    {

        // @brief The name of a register
        String reg_name = "";

        // @brief The value of a register
        uint8 reg_val = 0;
    };

    // @brief A list of registers and values
    using RegValList = std::vector<dunedaq::flxlibs::felixcardcontroller::RegValPair>;

    // @brief Bitfield access parameters
    struct SetBFs 
    {

        // @brief Physical card identifier (in the same host)
        uint4 card_id = 0;

        // @brief Logical unit identifier
        uint4 log_unit_id = 0;

        // @brief A list of bitfields and values to set
        RegValList bf_val_pairs = {};
    };

    // @brief Register access parameters
    struct SetRegisters 
    {

        // @brief Physical card identifier (in the same host)
        uint4 card_id = 0;

        // @brief Logical unit identifier
        uint4 log_unit_id = 0;

        // @brief A list of registers and values to set
        RegValList reg_val_pairs = {};
    };

} // namespace dunedaq::flxlibs::felixcardcontroller

#endif // DUNEDAQ_FLXLIBS_FELIXCARDCONTROLLER_STRUCTS_HPP