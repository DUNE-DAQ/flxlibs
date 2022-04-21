// This is the application info schema used by the card controller module.
// It describes the information object structure passed by the application
// for operational monitoring

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.flxlibs.felixcardcontrollerinfo");

local info = {
    boolean : s.boolean("boolean",
                      doc="A boolean"),

    uint4  : s.number("uint4", "u4",
                      doc="An unsigned of 4 bytes"),

    link : s.record("LinkInfo", [
    s.field("device_id", self.uint4, doc="Device identifier"),
    s.field("link_id", self.uint4, doc="Link identifier"),
    s.field("enabled", self.boolean, doc="Indicate whether the link is enabled"),
    s.field("aligned", self.boolean, doc="Indicate whether the link is aligned")
    ], doc="Upstream FELIX Link information")

};

moo.oschema.sort_select(info)
