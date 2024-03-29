// This is the configuration schema for timinglibs

local moo = import "moo.jsonnet";

local stypes = import "daqconf/types.jsonnet";
local types = moo.oschema.hier(stypes).dunedaq.daqconf.types;

local sboot = import "daqconf/bootgen.jsonnet";
local bootgen = moo.oschema.hier(sboot).dunedaq.daqconf.bootgen;


local ns = "dunedaq.flxlibs.confgen";
local s = moo.oschema.schema(ns);

// A temporary schema construction context.
local cs = {

  array: s.sequence("array", types.count, "A list of counts."),
  alignment_mask: s.sequence("alignment_mask", self.array, "A list of arrays, should be 2 (1 for each SLR)"),

  flxcardcontroller: s.record('flxcardcontroller', [
    s.field("detector_readout_map_file", types.path, default='./detro_map.json', doc="File containing detector readout map for configuration to run"),
    s.field("emulator_mode",     types.flag, default=false, doc="If active, timestamps of data frames are overwritten when processed by the readout. This is necessary if the felix card does not set correct timestamps. Former -e"),
    s.field("ignore_alignment_mask", self.alignment_mask, default=[[],[]], doc="elink numbers to ignore when checking link alignment."),
  ]),

  flxlibs_gen: s.record('flxlibs_gen', [
    s.field('boot',              bootgen.boot,           default=bootgen.boot,           doc='Boot parameters'),
    s.field('flxcardcontroller', self.flxcardcontroller, default=self.flxcardcontroller, doc='FELIX conf parameters'),
  ]),

};

// Output a topologically sorted array.
stypes + sboot + moo.oschema.sort_select(cs, ns)
