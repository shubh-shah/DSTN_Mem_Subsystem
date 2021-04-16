#include "l1_cache.h"

l1_cache *init_l1_cache() {
    // calloc ensures initialization with 0s
    // verify if bool array initialization will also be with all 0 aka false
    l1_cache *cache = (l1_cache *) calloc(1, sizeof(l1_cache));
    return cache;
}

// Look through. call read_l2_cache only if this function returns false
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
            // use the offset to read the required byte
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


// Write through. Every write to l1 results in write to l2.
// returns true if write successful
// bool updating tells whether cache updation(from l2) is happening or cache write by the process
// If updation, then do not write to l2 as there is no concept of dirty bit here. Data in l1 is always in sync with l2
bool write_l1_cache(l1_cache *l1, l2_cache *l2, uint32_t physical_address, bool updating) {

    uint8_t offset = physical_address & L1_OFFSET_MASK;
    uint8_t index = (physical_address >> L1_INDEX_SHIFT) & L1_INDEX_MASK;
    uint16_t tag = (physical_address >> L1_TAG_SHIFT) & L1_TAG_MASK;

    l1_cache_set set = l1->sets[index];
    // 3 cases
    // 1. Earlier there was cache miss. Now bringing the block from l2 to l1 cache - this case replacement
    // 2. The block to which we want to write is present in the cache.
    // 3. The block to which we want to write is absent from cache. - this case replacement if all entries valid

    //checking if replacement is required or not.
    for (int i = 0; i < L1_SET_SIZE; ++i) {
        if (set.tags[i] == tag || !set.valid[i]) { // If either the tag matches or the data is invalid
            // no need to update the tag.
            // update at the desired offset in the block and mark recency
            mark_recency(set.matrix, i);
            set.valid[i] = true;
            if (updating)
                return true;
            return write_l2_cache(l2, physical_address); // write to l2 simultaneously
        }
    }

    // Here if the tag was not found in the cache and all the entries were valid
    int way_no = get_block_to_replace(set.matrix);

    // If no block was replaceable. This should never happen
    if (way_no == -1)
        return false; // failed to replace block. Write unsuccessful

    //update the tag no
    set.tags[way_no] = tag;
    set.valid[way_no] = true;
    // replace the block and mark recency
    mark_recency(set.matrix, way_no);

    if (updating)
        return true;
    return write_l2_cache(l2, physical_address);
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
