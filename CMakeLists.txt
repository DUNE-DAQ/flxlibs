cmake_minimum_required(VERSION 3.12)
project(flxlibs VERSION 1.0.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(appfwk REQUIRED)
find_package(dataformats REQUIRED)
find_package(readout REQUIRED)
#find_package(udaq_readout_deps REQUIRED)

##############################################################################
# Dependency sets
set(FLXUTIL_DEPENDENCIES readout::readout dataformats::dataformats)
#set(FLXCARD_DEPENDENCIES udaq_readout_deps::FlxCard udaq_readout_deps::cmem_rcc udaq_readout_deps::packetformat)
message("FELIX LIB -> " $ENV{FLX_LIB})
message("FELIX INC -> " $ENV{FLX_INC})

set(FLXCARD_DEPENDENCIES 
  $ENV{FLX_LIB}/libregmap.so
  $ENV{FLX_LIB}/libpacketformat.so
  $ENV{FLX_LIB}/libdrivers_rcc.so
  $ENV{FLX_LIB}/libgetinput.so
  $ENV{FLX_LIB}/libio_rcc.so
  $ENV{FLX_LIB}/librcc_error.so
  $ENV{FLX_LIB}/libcmem_rcc.so
  $ENV{FLX_LIB}/libDFDebug.so
  $ENV{FLX_LIB}/libFlxCard.so
)

##############################################################################
# Main library
daq_add_library(DefaultParserImpl.cpp CardWrapper.cpp 
  LINK_LIBRARIES appfwk::appfwk ${FLXCARD_DEPENDENCIES} ${FLXUTIL_DEPENDENCIES}
)

# Hack targets
target_include_directories(flxlibs PUBLIC $ENV{FLX_INC})

##############################################################################
# Plugins
daq_add_plugin(FelixCardReader duneDAQModule SCHEMA LINK_LIBRARIES appfwk::appfwk flxlibs)

##############################################################################
# Integration tests
daq_add_application(flxlibs_test_cardwrapper test_cardwrapper_app.cxx TEST LINK_LIBRARIES flxlibs)
daq_add_application(flxlibs_test_elinkhandler test_elinkhandler_app.cxx TEST LINK_LIBRARIES flxlibs)

##############################################################################
# Test configurations
file(COPY test/felixreadout-commands.json DESTINATION test)

##############################################################################
# Installation
daq_install()
