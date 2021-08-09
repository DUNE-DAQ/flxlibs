// This is the application info schema used by the card reader module.
// It describes the information object structure passed by the application
// for operational monitoring

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.flxlibs.felixcardreaderinfo");

local info = {
    uint8  : s.number("uint8", "u8",
        doc="An unsigned of 8 bytes"),
    float8 : s.number("float8", "f8",
        doc="A float of 8 bytes"),

info: s.record("ELinkInfo", [
    s.field("card_id", self.uint8, 0, doc="Card ID"),
    s.field("logical_unit", self.uint8, 0, doc="Logical unit number"),
    s.field("link_id", self.uint8, 0, doc="Link ID"),
    s.field("link_tag", self.uint8, 0, doc="Link tag"),
    s.field("num_short_chunks_processed", self.uint8, 0, doc="Short chunks processed"),
    s.field("num_chunks_processed", self.uint8, 0, doc="Chunks processed"),
    s.field("num_subchunks_processed", self.uint8, 0, doc="Subchunks processed"),
    s.field("num_blocks_processed", self.uint8, 0, doc="Blocks processed"),
    s.field("num_short_chunks_processed_with_error", self.uint8, 0, doc="Short chunks processed with error"),
    s.field("num_chunks_processed_with_error", self.uint8, 0, doc="Chunks processed with error"),
    s.field("num_subchunks_processed_with_error", self.uint8, 0, doc="Subchunks processed with error"),
    s.field("num_blocks_processed_with_error", self.uint8, 0, doc="Blocks processed with error"),
    s.field("rate_blocks_processed", self.float8, 0.0, doc="Rate of processed blocks in KHz"),
    s.field("rate_chunks_processed", self.float8, 0.0, doc="Rate of processed chunks in KHz")
  ], doc="ELink information")
};

moo.oschema.sort_select(info)
