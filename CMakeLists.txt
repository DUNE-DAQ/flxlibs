cmake_minimum_required(VERSION 3.12)
project(flxlibs VERSION 1.0.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(appfwk REQUIRED)
find_package(logging REQUIRED)
find_package(dataformats REQUIRED)
find_package(readout REQUIRED)
#find_package(felix REQUIRED)

daq_codegen(*.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2 )

##############################################################################
# Dependency sets
set(FLXUTIL_DEPENDENCIES readout::readout dataformats::dataformats)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native")
#set(FLXCARD_DEPENDENCIES felix::FlxCard felix::cmem_rcc felix::packetformat)
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
daq_add_plugin(FelixCardReader duneDAQModule LINK_LIBRARIES appfwk::appfwk flxlibs)

##############################################################################
# Integration tests
daq_add_application(flxlibs_test_cardwrapper test_cardwrapper_app.cxx TEST LINK_LIBRARIES flxlibs)
daq_add_application(flxlibs_test_elinkhandler test_elinkhandler_app.cxx TEST LINK_LIBRARIES flxlibs)
daq_add_application(flxlibs_test_elink_to_file test_elink_to_file_app.cxx TEST LINK_LIBRARIES flxlibs)

##############################################################################
# Test configurations
file(COPY test/felixreadout-commands.json DESTINATION test)

##############################################################################
# Installation
daq_install()
