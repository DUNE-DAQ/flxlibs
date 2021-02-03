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
    conf(cid=0, coff=0, did=0, nid=0, nums=1, numl=6) :: {
        card_id: cid, 
        card_offset: coff, 
        dma_id: did, 
        numa_id: nid,
        num_sources: nums, 
        num_links: numl
    },
}

