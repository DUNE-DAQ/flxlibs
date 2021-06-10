cmake_minimum_required(VERSION 3.12)
project(flxlibs VERSION 1.1.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(appfwk REQUIRED)
find_package(logging REQUIRED)
find_package(dataformats REQUIRED)
find_package(readout REQUIRED)
find_package(felix REQUIRED)

daq_codegen(felixcardreader.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2 )
daq_codegen( *info.jsonnet DEP_PKGS opmonlib TEMPLATES opmonlib/InfoStructs.hpp.j2 opmonlib/InfoNljs.hpp.j2 )

##############################################################################
# Dependency sets
set(DUNEDAQ_DEPENDENCIES readout::readout dataformats::dataformats)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native")

set(FELIX_DEPENDENCIES 
  felix::regmap
  felix::packetformat
  felix::drivers_rcc
  felix::getinput
  #felix::io_rcc
  felix::rcc_error
  felix::cmem_rcc
  felix::DFDebug
  felix::FlxCard
)

##############################################################################
# Main library
daq_add_library(DefaultParserImpl.cpp CardWrapper.cpp LINK_LIBRARIES appfwk::appfwk ${FELIX_DEPENDENCIES} ${DUNEDAQ_DEPENDENCIES})

##############################################################################
# Plugins
daq_add_plugin(FelixCardReader duneDAQModule LINK_LIBRARIES appfwk::appfwk flxlibs)

##############################################################################
# Integration tests
daq_add_application(flxlibs_test_cardwrapper test_cardwrapper_app.cxx TEST LINK_LIBRARIES flxlibs)
daq_add_application(flxlibs_test_elinkhandler test_elinkhandler_app.cxx TEST LINK_LIBRARIES flxlibs)
daq_add_application(flxlibs_test_elink_to_file test_elink_to_file_app.cxx TEST LINK_LIBRARIES flxlibs)
daq_add_application(flxlibs_test_elink_to_heap test_elink_to_heap_app.cxx TEST LINK_LIBRARIES flxlibs)

##############################################################################
# Test configurations
file(COPY test/felix_wib_readout.json DESTINATION test)
file(COPY test/felix_wib2_readout.json DESTINATION test)

##############################################################################
# Installation
daq_install()
