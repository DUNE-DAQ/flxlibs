cmake_minimum_required(VERSION 3.12)
project(flxlibs VERSION 1.0.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(appfwk REQUIRED)
find_package(dataformats REQUIRED)
find_package(readout REQUIRED)
find_package(udaq_readout_deps REQUIRED)

##############################################################################
# Dependency sets
set(FLXCARD_DEPENDENCIES udaq_readout_deps::FlxCard udaq_readout_deps::cmem_rcc udaq_readout_deps::packetformat)
set(FLXUTIL_DEPENDENCIES readout::readout dataformats::dataformats)

##############################################################################
# Main library
daq_add_library(DefaultParserImpl.cpp          
  LINK_LIBRARIES appfwk::appfwk ${FLXCARD_DEPENDENCIES} ${FLXUTIL_DEPENDENCIES}
)

##############################################################################
# Plugins
daq_add_plugin(FelixCardReader duneDAQModule SCHEMA LINK_LIBRARIES appfwk::appfwk flxlibs)

##############################################################################
# Integration tests

##############################################################################
# Test configurations
file(COPY test/felix_readout_card0_app.json DESTINATION test)

##############################################################################
# Installation
daq_install()
