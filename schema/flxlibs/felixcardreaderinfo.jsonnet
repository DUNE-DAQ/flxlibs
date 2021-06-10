// This is the application info schema used by the card reader module.
// It describes the information object structure passed by the application
// for operational monitoring

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.flxlibs.felixcardreaderinfo");

local info = {
uint8  : s.number("uint8", "u8",
    doc="An unsigned of 8 bytes"),

info: s.record("Info", [
    s.field("card_id", self.uint8, 0, doc="Card ID"),
    s.field("logical_unit", self.uint8, 0, doc="Logical unit number"),
    s.field("link_id", self.uint8, 0, doc="Link ID"),
    s.field("link_tag", self.uint8, 0, doc="Link tag"),
    s.field("short_chunks_processed", self.uint8, 0, doc="Short chunks processed"),
    s.field("chunks_processed", self.uint8, 0, doc="Cunks processed"),
    s.field("subchunks_processed", self.uint8, 0, doc="Subchunks processed"),
    s.field("blocks_processed", self.uint8, 0, doc="Blocks processed"),
    s.field("short_chunks_processed_with_error", self.uint8, 0, doc="Short chunks processed with error"),
    s.field("chunks_processed_with_error", self.uint8, 0, doc="Chunks processed with error"),
    s.field("subchunks_processed_with_error", self.uint8, 0, doc="Subchunks processed with error"),
    s.field("blocks_processed_with_error", self.uint8, 0, doc="Blocks processed with error"),
  ], doc="Felix card reader information")
};

moo.oschema.sort_select(info)