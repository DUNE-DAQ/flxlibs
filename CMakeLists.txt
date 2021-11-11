cmake_minimum_required(VERSION 3.12)
project(flxlibs VERSION 1.2.3)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(ers REQUIRED)
find_package(appfwk REQUIRED)
find_package(logging REQUIRED)
find_package(readout REQUIRED)
find_package(felix REQUIRED)

daq_codegen(felixcardreader.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2 )
daq_codegen(felixcardcontroller.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2 )
daq_codegen( *info.jsonnet DEP_PKGS opmonlib TEMPLATES opmonlib/InfoStructs.hpp.j2 opmonlib/InfoNljs.hpp.j2 )

##############################################################################
# Dependency sets
set(DUNEDAQ_DEPENDENCIES appfwk::appfwk readout::readout)

# Provide override functionality for FELIX dependencies
option(WITH_FELIX_AS_PACKAGE "FELIX externals as a dunedaq package" ON)

if(WITH_FELIX_AS_PACKAGE)
  set(FELIX_DEPENDENCIES 
    felix::regmap
    felix::packetformat
    felix::drivers_rcc
    felix::getinput
    felix::rcc_error
    felix::cmem_rcc
    felix::DFDebug
    felix::FlxCard
  )
else()
  # From env
  set(FELIX_DEPENDENCIES 
    $ENV{FLX_LIB}/libregmap.so
    $ENV{FLX_LIB}/libpacketformat.so
    $ENV{FLX_LIB}/libdrivers_rcc.so
    $ENV{FLX_LIB}/libgetinput.so
    $ENV{FLX_LIB}/librcc_error.so
    $ENV{FLX_LIB}/libcmem_rcc.so
    $ENV{FLX_LIB}/libDFDebug.so
    $ENV{FLX_LIB}/libFlxCard.so
  )
endif()

##############################################################################
# Main library
daq_add_library(DefaultParserImpl.cpp CardWrapper.cpp CardControllerWrapper.cpp LINK_LIBRARIES ${FELIX_DEPENDENCIES} ${DUNEDAQ_DEPENDENCIES})


if(WITH_FELIX_AS_PACKAGE)
  # All good
else()
  # Hack target
  target_include_directories(flxlibs PUBLIC $ENV{FLX_INC})
endif()

##############################################################################
# Plugins
daq_add_plugin(FelixCardReader duneDAQModule LINK_LIBRARIES flxlibs)
daq_add_plugin(FelixCardController duneDAQModule LINK_LIBRARIES flxlibs)

##############################################################################
# Integration tests
daq_add_application(flxlibs_test_cardwrapper test_cardwrapper_app.cxx TEST LINK_LIBRARIES flxlibs)
daq_add_application(flxlibs_test_elinkhandler test_elinkhandler_app.cxx TEST LINK_LIBRARIES flxlibs)
daq_add_application(flxlibs_test_elink_to_file test_elink_to_file_app.cxx TEST LINK_LIBRARIES flxlibs)
daq_add_application(flxlibs_test_elink_to_heap test_elink_to_heap_app.cxx TEST LINK_LIBRARIES flxlibs)

##############################################################################
# Applications
daq_add_application(flxlibs_emu_confgen emu_confgen.cxx LINK_LIBRARIES flxlibs)

##############################################################################
# Installation
daq_install()
