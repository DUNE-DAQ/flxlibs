// The schema used by classes in the appfwk code tests.
//
// It is an example of the lowest layer schema below that of the "cmd"
// and "app" and which defines the final command object structure as
// consumed by instances of specific DAQModule implementations (ie,
// the test/Felix* modules).

local moo = import "moo.jsonnet";

// A schema builder in the given path (namespace)
local ns = "dunedaq.flxlibs.felixcardreader";
local s = moo.oschema.schema(ns);

// Object structure used by the test/fake producer module
local felixcardreader = {
    count  : s.number("Count", "u4",
                      doc="Count of things"),

    id : s.number("Identifier", "i4",
                  doc="An ID of a thingy"),

    conf: s.record("Conf", [
        s.field("card_id", self.id, 0,
                doc="Physical card identifier (in the same host)"),

        s.field("card_offset", self.count, 0,
                doc="Superlogic region of selected card"),

        s.field("dma_id", self.id, 0,
                doc="DMA descriptor to use"),

        s.field("numa_id", self.id, 0,
                doc="CMEM_RCC NUMA region selector"),

        s.field("num_sources", self.count, 1,
                doc="Read a single superlogic region, or both"),

        s.field("num_links", self.count, 5,
                doc="Number of elinks configured"),       

    ], doc="Upstream FELIX CardReader DAQ Module Configuration"),

};

moo.oschema.sort_select(felixcardreader, ns)
