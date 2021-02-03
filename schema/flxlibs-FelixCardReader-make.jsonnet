// hand written helpers to make object compliant with flxlibs-FelixCardReader
{
    // The internally known name of the only queue used
    queue: "blocks-0", 
           "blocks-64", 
           "blocks-128", 
           "blocks-192", 
           "blocks-256", 
           "blocks-320"

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

