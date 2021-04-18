#include "l1_cache.h"

l1_cache *init_l1_cache() {
    // calloc ensures initialization with 0s
    // verify if bool array initialization will also be with all 0 aka false
    l1_cache *cache = (l1_cache *) calloc(1, sizeof(l1_cache));
    return cache;
}

// Look through.
// If this function returns false then call read_l2_cache(). Irrespective if it returns true/ false, call read_memory()
// also as l2 is read aside. Update l2_cache() if read_l2_cache() returned false.
// Next update l1 cache using update_l1_cache(). No need to call read_l1_cache() again as data was supplied directly to the processor
// by l2 cache or memory.
bool read_l1_cache(l1_cache *cache, uint32_t physical_address) {
    uint8_t offset = physical_address & L1_OFFSET_MASK;
    uint8_t index = (physical_address >> L1_INDEX_SHIFT) & L1_INDEX_MASK;
    uint16_t tag = (physical_address >> L1_TAG_SHIFT) & L1_TAG_MASK;

    l1_cache_set set = cache->sets[index];
    // 1st cycle: compare tags in parallel
    for (int i = 0; i < L1_SET_SIZE; ++i) {
        if (set.tags[i] == tag && set.valid[i]) {
            mark_recency(set.matrix, i);
            // 2nd cycle: activate 1 (out of 4) data block array and retrieve the 16B block
            // read the byte at the offset and put it in the processor-l1 bus
            return true; // cache hit
        }
    }
    return false; // cache miss
}

void mark_recency(bool matrix[L1_SET_SIZE][L1_SET_SIZE], int i) {
    for (int j = 0; j < L1_SET_SIZE; ++j) {
        matrix[i][j] = 1;
        matrix[j][i] = 0;
    }
}


// Write through. Every write to l1 results in write to l2. call write_l2_cache() immediately after this returns true.
// Return true if the block to which we want to write is present in the cache, else false
// If returned false, means write miss. In this case, call read_l2_cache(). Irrespective if it returns true/ false,
// call read_memory() also as l2 is read aside. Update l2_cache() if read_l2_cache() returned false.
// Next update l1 cache using update_l1_cache(). Call write_l1_cache() again. This time, it should be write hit and will return true.
bool write_l1_cache(l1_cache *l1, l2_cache *l2, uint32_t physical_address) {

    uint8_t offset = physical_address & L1_OFFSET_MASK;
    uint8_t index = (physical_address >> L1_INDEX_SHIFT) & L1_INDEX_MASK;
    uint16_t tag = (physical_address >> L1_TAG_SHIFT) & L1_TAG_MASK;

    l1_cache_set set = l1->sets[index];

    //check if required block to write already exists or not.
    for (int i = 0; i < L1_SET_SIZE; ++i) {
        if (set.tags[i] == tag) {
            // If the tags match, read data from processor-l1 bus and write it in the block
            set.valid[i] = true;
            mark_recency(set.matrix, i);
            // write the block data to the l1-l2 bus, so that we can write to l2 cache next.
            return true;
        }
    }
    return false;
}

// This function is called only after there was a cache read/write miss earlier. Call this to get data from l2 to l1 cache
bool update_l1_cache(l1_cache *l1, uint32_t physical_address) {
    uint8_t offset = physical_address & L1_OFFSET_MASK;
    uint8_t index = (physical_address >> L1_INDEX_SHIFT) & L1_INDEX_MASK;
    uint16_t tag = (physical_address >> L1_TAG_SHIFT) & L1_TAG_MASK;

    l1_cache_set set = l1->sets[index];
    // Earlier there was cache miss. Now bringing the block from l2 to l1 cache - this case replacement

    for (int i = 0; i < L1_SET_SIZE; ++i) {
        if (!set.valid[i]) { // If data is invalid in any of the blocks, it can be overwritten
            // read the data from l1-l2 bus and write it in the block.
            // update the tag no, valid bit and mark recently accessed
            set.tags[i] = tag;
            set.valid[i] = true;
            mark_recency(set.matrix, i);
            return true;
        }
    }

    // Here if all the entries were valid. Get a block for replacement.
    int way_no = get_block_to_replace(set.matrix);

    // If no block was replaceable. This should never happen
    if (way_no == -1)
        return false; // failed to replace block. Write unsuccessful

    // read the data from l1-l2 bus and write it in the block.
    // update the tag no, valid bit and mark recently accessed
    set.tags[way_no] = tag;
    set.valid[way_no] = true;
    mark_recency(set.matrix, way_no);
    return true;
}

// returns the way number {0,1,2,3} of the block to be replaced
int get_block_to_replace(bool matrix[L1_SET_SIZE][L1_SET_SIZE]) {
    bool replaceable;
    for (int i = 0; i < L1_SET_SIZE; ++i) {
        replaceable = true;
        for (int j = 0; j < L1_SET_SIZE; ++j) {
            if (matrix[i][j] == 1) {
                replaceable = false;
                break;
            }
        }
        if (replaceable)
            return i;
    }
    return -1;
}
