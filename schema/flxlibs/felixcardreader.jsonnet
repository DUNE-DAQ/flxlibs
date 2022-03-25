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

    array : s.sequence("Array", self.count, doc="list of numbers"),

    choice : s.boolean("Choice"),

    conf: s.record("Conf", [
        s.field("card_id", self.id, 0,
                doc="Physical card identifier (in the same host)"),

        s.field("logical_unit", self.count, 0,
                doc="Superlogic region of selected card"),

        s.field("dma_id", self.id, 0,
                doc="DMA descriptor to use"),

        s.field("chunk_trailer_size", self.count, 0,
                doc="Are chunks with 32b trailer."),

        s.field("dma_block_size_kb", self.count, 4,
                doc="FELIX DMA Block size"),

        s.field("dma_memory_size_gb", self.count, 1,
                doc="CMEM_RCC memory to allocate in GBs."),

        s.field("dma_margin_blocks", self.count, 4,
                doc="DMA parser safe margin block count"),

        s.field("dma_block_threshold", self.count, 10,
                doc="DMA parser activates at number of available new blocks"),

        s.field("interrupt_mode", self.choice, false,
                doc="Use device interrupts or polling for DMA parsing"),

        s.field("poll_time", self.count, 5000,
                doc="Poll time in us. Ignored if interrupt mode is on."),

        s.field("numa_id", self.id, 0,
                doc="CMEM_RCC NUMA region selector"),

        s.field("num_sources", self.count, 1,
                doc="Read a single superlogic region, or both"),

        s.field("links_enabled", self.array, [0, 1, 2, 3, 4],
                doc="Number of elinks configured"),

    ], doc="Upstream FELIX CardReader DAQ Module Configuration"),

};

moo.oschema.sort_select(felixcardreader, ns)
