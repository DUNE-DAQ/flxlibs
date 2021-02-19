// hand written helpers to make object compliant with flxlibs-FelixCardReader
{
    // The internally known name of the only queue used
    queue: "out-0", 
           "out-1", 
           "out-2", 
           "out-3", 
           "out-4", 
           "out-5"

    // Make a conf object for cardreader
    conf(cid=0, logu=0, did=0, dbs=4, dms=4, cts=32, nid=0, nums=1, numl=6) :: {
        card_id: cid,
        logical_unit: logu,
        dma_id: did,
        dma_block_size: dbs,
        dma_memory_size: dms,
        chunk_trailer_size: cts,
        numa_id: nid,
        num_sources: nums, 
        num_links: numl
    },
}

