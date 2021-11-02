local moo = import "moo.jsonnet";

// A schema builder in the given path (namespace)
local ns = "dunedaq.flxlibs.felixcardcontroller";
local s = moo.oschema.schema(ns);

// Object structure
local felixcardcontroller = {
    conf: s.record("Conf", [
    ], doc="Upstream FELIX CardController DAQ Module Configuration"),
};

moo.oschema.sort_select(felixcardcontroller, ns)