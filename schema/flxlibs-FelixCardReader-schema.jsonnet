// The schema used by classes in the appfwk code tests.
//
// It is an example of the lowest layer schema below that of the "cmd"
// and "app" and which defines the final command object structure as
// consumed by instances of specific DAQModule implementations (ie,
// the test/Fake* modules).

local moo = import "moo.jsonnet";

// A schema builder in the given path (namespace)
local ns = "dunedaq.flxlibs.felixcardreader";
local s = moo.oschema.schema(ns);

// Object structure used by the test/fake producer module
local felixcardreader = {
    size  : s.number("Size", "u8",
                     doc="A count of very many things"),

    count : s.number("Count", "i4",
                     doc="A count of not too many things"),

    conf: s.record("Conf", [
        s.field("card_id", self.count, 0,
                doc="Physical card identifier (in the same host)"),

        s.field("card_offset", self.count, 0,
                doc="Superlogic region of selected card"),

        s.field("dma_id", self.count, 0,
                doc="DMA descriptor to use"),

        s.field("numa_id", self.count, 0,
                doc="CMEM_RCC NUMA region selector"),

        s.field("num_sources", self.count, 1,
                doc="Read a single superlogic region, or both"),

        s.field("num_links", self.count, 5,
                doc="Number of elinks configured"),       

    ], doc="Upstream FELIX CardReader DAQ Module Configuration"),

};

moo.oschema.sort_select(felixcardreader, ns)
