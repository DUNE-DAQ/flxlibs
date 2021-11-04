local moo = import "moo.jsonnet";

// A schema builder in the given path (namespace)
local ns = "dunedaq.flxlibs.felixcardcontroller";
local s = moo.oschema.schema(ns);

// Object structure
local felixcardcontroller = {
    count  : s.number("Count", "u4",
                      doc="Count of things"),

    id : s.number("Identifier", "i4",
                  doc="An ID of a thingy"),

    uint8 : s.number("uint8", "u8",
                      doc="An unsigned of 8 bytes"),

    string : s.string("String",
                       doc="A string field"),

    choice : s.boolean("Choice"),

    conf: s.record("Conf", [
    s.field("card_id", self.id, 0,
            doc="Physical card identifier (in the same host)"),

    s.field("logical_unit", self.count, 0,
            doc="Superlogic region of selected card"),

    s.field("dma_id", self.id, 0,
            doc="DMA descriptor to use"),

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
            doc="CMEM_RCC NUMA region selector")

    ], doc="Upstream FELIX CardController DAQ Module Configuration"),

    getregister: s.record("GetRegisterParams", [
        s.field("reg_name", self.string, "",
                doc="The name of a register")
    ], doc="Register access parameters"),

    setregister: s.record("SetRegisterParams", [
        s.field("reg_name", self.string, "",
                doc="The name of a register"),
        s.field("reg_val", self.uint8, 0,
                doc="The value of a register")
    ], doc="")
};

moo.oschema.sort_select(felixcardcontroller, ns)