#include "l1_cache.h"

l1_cache *init_l1_cache() {
    // calloc ensures initialization with 0s
    l1_cache *cache = (l1_cache *) calloc(1, sizeof(l1_cache));
    return cache;
}

/*
   Input:
       2d L1_SET_SIZE*L1_SET_SIZE size array
       Integer i, the way number whose block was recently accessed
*/
void mark_recency(bool matrix[L1_SET_SIZE][L1_SET_SIZE], int i) {
    for (int j = 0; j < L1_SET_SIZE; ++j) {
        matrix[i][j] = 1;
        matrix[j][i] = 0;
    }
}

/*
   Input:
       2d L1_SET_SIZE*L1_SET_SIZE size array
       Integer i, the set number which was recently accessed
   Returns:
        The way number (0 to L1_SET_SIZE-1) whose block will be replaced
*/
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

/*
   Called when a process is trying to read data. Read is Look Through, so read L2 cache only if this returns false. In
   that case, data is supplied directly to the processor by the higher levels, so this function is not called again.
   Input:
       Physical Address
   Returns:
       True if Read Hit
       False if Read Miss
*/
bool read_l1_cache(l1_cache *cache, uint32_t physical_address) {
    uint8_t offset = physical_address & L1_OFFSET_MASK;
    uint8_t index = (physical_address >> L1_INDEX_SHIFT) & L1_INDEX_MASK;
    uint16_t tag = (physical_address >> L1_TAG_SHIFT) & L1_TAG_MASK;

    l1_cache_set *set = &cache->sets[index];
    // 1st cycle: compare tags in parallel
    for (int i = 0; i < L1_SET_SIZE; ++i) {
        if (set->tags[i] == tag && set->valid[i]) {
            mark_recency(set->matrix, i);
            // 2nd cycle: activate 1 (out of 4) data block array and retrieve the 16B block
            // read the byte at the offset and put it in the processor-l1 bus
            return true;
        }
    }
    return false;
}

/*
   Called when a process is writing data. Write Through, so write to L2 cache immediately after this returns true. If
   returned false, read from higher levels and update the L1 cache. Call this function again and it will be write hit.
   Input:
       Physical Address
   Returns:
       True if Write Hit
       False if Write Miss
*/
bool write_l1_cache(l1_cache *l1, uint32_t physical_address) {
    uint8_t offset = physical_address & L1_OFFSET_MASK;
    uint8_t index = (physical_address >> L1_INDEX_SHIFT) & L1_INDEX_MASK;
    uint16_t tag = (physical_address >> L1_TAG_SHIFT) & L1_TAG_MASK;

    l1_cache_set *set = &l1->sets[index];

    //check if required block to write already exists or not.
    for (int i = 0; i < L1_SET_SIZE; ++i) {
        if (set->tags[i] == tag) {
            // If the tags match, read data from processor-l1 bus and write it in the block
            set->valid[i] = true;
            mark_recency(set->matrix, i);
            // write the block data to the l1-l2 bus, so that we can write to l2 cache next.
            return true;
        }
    }
    return false;
}

/*
   Called only after there was a cache read/write miss earlier. Call this to get data from higher levels to L1 cache
   Input:
       Physical Address
   Returns:
       True if update was successful
       False - Should never return. Indicates that no block could be replaced.
*/
bool update_l1_cache(l1_cache *l1, uint32_t physical_address) {
    uint8_t offset = physical_address & L1_OFFSET_MASK;
    uint8_t index = (physical_address >> L1_INDEX_SHIFT) & L1_INDEX_MASK;
    uint16_t tag = (physical_address >> L1_TAG_SHIFT) & L1_TAG_MASK;

    l1_cache_set *set = &l1->sets[index];
    // Earlier there was cache miss. Now bringing the block from l2 to l1 cache - this case replacement

    for (int i = 0; i < L1_SET_SIZE; ++i) {
        if (!set->valid[i]) { // If data is invalid in any of the blocks, it can be overwritten
            // read the data from l1-l2 bus and write it in the block.
            // update the tag no, valid bit and mark recently accessed
            set->tags[i] = tag;
            l1->sets[index].tags[i] = tag;
            set->valid[i] = true;
            mark_recency(set->matrix, i);
            return true;
        }
    }

    // Here if all the entries were valid. Get a block for replacement.
    int way_no = get_block_to_replace(set->matrix);

    // If no block was replaceable. This should never happen
    if (way_no == -1)
        return false; // failed to replace block. Write unsuccessful

    // read the data from l1-l2 bus and write it in the block.
    // update the tag no, valid bit and mark recently accessed
    set->tags[way_no] = tag;
    set->valid[way_no] = true;
    mark_recency(set->matrix, way_no);
    return true;
}
