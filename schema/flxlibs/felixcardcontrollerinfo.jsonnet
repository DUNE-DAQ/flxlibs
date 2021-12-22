// This is the application info schema used by the card controller module.
// It describes the information object structure passed by the application
// for operational monitoring

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.flxlibs.felixcardcontrollerinfo");

local info = {
    uint8 : s.number("uint8", "u8",
        doc="An unsigned of 8 bytes"),

info: s.record("ChannelInfo", [
    s.field("channel00_alignment_status", self.uint8, 0, doc="Alignment status for channel 0"),
    s.field("channel01_alignment_status", self.uint8, 0, doc="Alignment status for channel 1"),
    s.field("channel02_alignment_status", self.uint8, 0, doc="Alignment status for channel 2"),
    s.field("channel03_alignment_status", self.uint8, 0, doc="Alignment status for channel 3"),
    s.field("channel04_alignment_status", self.uint8, 0, doc="Alignment status for channel 4"),
    s.field("channel05_alignment_status", self.uint8, 0, doc="Alignment status for channel 5")
], doc="Channel information")
};

moo.oschema.sort_select(info)