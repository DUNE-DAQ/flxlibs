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

    regvalpair : s.record("RegValPair", [
    s.field("reg_name", self.string, "",
                doc="The name of a register"),
    s.field("reg_val", self.uint8, 0,
                doc="The value of a register")
    ]),

    reglist : s.sequence("RegList", self.string,
                doc="A list of registers"),

    regvallist : s.sequence("RegValList", self.regvalpair,
                doc="A list of registers and values"),

    conf: s.record("Conf", [
    s.field("card_id", self.id, 0,
            doc="Physical card identifier (in the same host)"),

    s.field("logical_unit", self.count, 0,
            doc="Superlogic region of selected card"),

    ], doc="Upstream FELIX CardController DAQ Module Configuration"),

    getregister: s.record("GetRegisterParams", [
    s.field("reg_names", self.reglist,
                doc="A list of registers")
    ], doc="Register access parameters"),

    setregister: s.record("SetRegisterParams", [
    s.field("reg_val_pairs", self.regvallist,
                doc="A list of registers and values to set")
    ], doc="Register access parameters"),

    gthreset: s.record("GTHResetParams", [
    s.field("quads", self.uint8, 0,
                doc="Binary representation of which quads (0-5) to reset. Eg. 010000 to reset quad 4")
    ], doc="GTH reciever reset parameters")
};

moo.oschema.sort_select(felixcardcontroller, ns)