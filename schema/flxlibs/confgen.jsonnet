// This is the configuration schema for timinglibs

local moo = import "moo.jsonnet";
local sdc = import "daqconf/confgen.jsonnet";
local daqconf = moo.oschema.hier(sdc).dunedaq.daqconf.confgen;

local ns = "dunedaq.flxlibs.confgen";
local s = moo.oschema.schema(ns);

// A temporary schema construction context.
local cs = {

  flxcardcontroller: s.record('flxcardcontroller', [
    s.field("hardware_map_file", daqconf.Path, default='./HardwareMap.txt', doc="File containing detector hardware map for configuration to run"),
    s.field("emulator_mode",     daqconf.Flag, default=false, doc="If active, timestamps of data frames are overwritten when processed by the readout. This is necessary if the felix card does not set correct timestamps. Former -e"),
  ]),

  flxcardcontrollerconf: s.record('flxcardcontrollerconf', [
    s.field('boot',              daqconf.boot,           default=daqconf.boot,           doc='Boot parameters'),
    s.field('flxcardcontroller', self.flxcardcontroller, default=self.flxcardcontroller, doc='FELIX conf parameters'),
  ]),

};

// Output a topologically sorted array.
sdc + moo.oschema.sort_select(cs, ns)
