// This is the application info schema used by the card controller module.
// It describes the information object structure passed by the application
// for operational monitoring

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.flxlibs.felixcardcontrollerinfo");

local info = {};

moo.oschema.sort_select(info)