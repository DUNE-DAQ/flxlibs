// This is the configuration schema for timinglibs

local moo = import "moo.jsonnet";
local sdc = import "daqconf/confgen.jsonnet";
local daqconf = moo.oschema.hier(sdc).dunedaq.daqconf.confgen;

local ns = "dunedaq.flxlibs.confgen";
local s = moo.oschema.schema(ns);

// A temporary schema construction context.
local cs = {

  array: s.sequence("array", daqconf.count, "A list of counts."),
  alignment_mask: s.sequence("alignment_mask", self.array, "A list of arrays, should be 2 (1 for each SLR)"),

  flxcardcontroller: s.record('flxcardcontroller', [
    s.field("hardware_map_file", daqconf.Path, default='./HardwareMap.txt', doc="File containing detector hardware map for configuration to run"),
    s.field("emulator_mode",     daqconf.Flag, default=false, doc="If active, timestamps of data frames are overwritten when processed by the readout. This is necessary if the felix card does not set correct timestamps. Former -e"),
    s.field("enable_firmware_tpg", daqconf.Flag, default=false, doc="If active, will add the tp links to the FELIX card controller when configuring."),
    s.field("ignore_alignment_mask", self.alignment_mask, default=[[],[]], doc="elink numbers to ignore when checking link alignment."),
  ]),

  flxlibs_gen: s.record('flxlibs_gen', [
    s.field('boot',              daqconf.boot,           default=daqconf.boot,           doc='Boot parameters'),
    s.field('flxcardcontroller', self.flxcardcontroller, default=self.flxcardcontroller, doc='FELIX conf parameters'),
  ]),

};

// Output a topologically sorted array.
sdc + moo.oschema.sort_select(cs, ns)
