#include "l2_cache.h"
#include "../global_variables.h"

l2_cache *init_l2_cache() {
    l2_cache *cache = (l2_cache *) malloc(sizeof(l2_cache));
    for (int i = 0; i < L2_NUM_SETS; ++i) {
        cache->sets[i].tags = createQueue(L2_SET_SIZE);
    }
    return cache;
}

// Look aside. Call read_memory() just after calling this, even if it returns true
// If this returned false, then after read_memory() call update_l2_cache(). No need to call read_l2_cache() again as
// data was supplied directly to the processor by the memory
bool read_l2_cache(l2_cache *cache, uint32_t physical_address) {
    uint8_t offset = physical_address & L2_OFFSET_MASK;
    uint8_t index = (physical_address >> L2_INDEX_SHIFT) & L2_INDEX_MASK;
    uint16_t tag = (physical_address >> L2_TAG_SHIFT) & L2_TAG_MASK;

    l2_cache_set set = cache->sets[index];
    q_node *node = set.tags->front;
    l2_cache_block *current_block;

    for (int i = 0; i < set.tags->node_count; ++i) {
        current_block = node->data_ptr;
        if (current_block->tag == tag && current_block->valid) {
            // use the offset to read the required byte
            return true; // cache hit
        }
        node = node->next;
    }
    return false; // cache miss
}

// Write back. So write to main memory takes place during replacement
// this function is called only after write_l1_cache() returns true.
// Return true if the block to which we want to write is present in the cache, else false
// If returned false, means write miss. In this case, call read_memory(). Update l2 cache using update_l2_cache() and then
// call write_l2_cache() again. This time, it should be write hit and will return true.
bool write_l2_cache(l2_cache *cache, uint32_t physical_address) {

    uint8_t offset = physical_address & L2_OFFSET_MASK;
    uint8_t index = (physical_address >> L2_INDEX_SHIFT) & L2_INDEX_MASK;
    uint16_t tag = (physical_address >> L2_TAG_SHIFT) & L2_TAG_MASK;

    l2_cache_set set = cache->sets[index];
    q_node *node = set.tags->front;
    l2_cache_block *current_block;

    for (int i = 0; i < set.tags->node_count; ++i) {
        current_block = node->data_ptr;
        if (current_block->tag == tag) {
            // If the tags match, read data from l1-l2 bus and write it in the block
            // Mark the block valid and dirty
            current_block->valid = 1;
            current_block->dirty = 1; // Data modified, so need to write to main memory during replacement
            return true;
        }
        node = node->next;
    }

    return false;
}

// This function is called only after there was a cache read/write miss earlier. Call this to get data from memory to l2 cache
void update_l2_cache(l2_cache *cache, main_memory *mm, uint32_t physical_address) {
    uint8_t offset = physical_address & L2_OFFSET_MASK;
    uint8_t index = (physical_address >> L2_INDEX_SHIFT) & L2_INDEX_MASK;
    uint16_t tag = (physical_address >> L2_TAG_SHIFT) & L2_TAG_MASK;

    l2_cache_set set = cache->sets[index];
    q_node *node = set.tags->front;
    l2_cache_block *current_block;

    for (int i = 0; i < set.tags->node_count; ++i) {
        current_block = node->data_ptr;
        if (!current_block->valid) { // If data is invalid in any of the blocks, it can be overwritten
            // read the data from l2-memory bus and write it in the block.
            // set the tag no and mark the block valid and dirty
            current_block->tag = tag;
            current_block->valid = 1;
            current_block->dirty = 0; // As it has the same data as in memory after the update
            return;
        }
        node = node->next;
    }

    // Here if all the entries were valid. Get a block for replacement.
    l2_cache_block *new_block = (l2_cache_block *) malloc(sizeof(l2_cache_block));
    // read the data from l2-memory bus and write it in the block.
    // set the tag no and valid bit
    new_block->tag = tag;
    new_block->index = index;
    new_block->offset = offset;
    new_block->valid = 1;
    new_block->dirty = 0; // As it has the same data as in memory after the update
    custom_push(set.tags, new_block, mm);
    return;
}

void custom_push(queue *q, void *data, main_memory *mm) {
    // If maximum limit on number of nodes is reached
    if (isFull(q))
        custom_pop(q, mm);

    push(q, data);
}

void custom_pop(queue *q, main_memory *mm) {

    q_node *node = pop(q);
    l2_cache_block *block = node->data_ptr;
    if (block->dirty) {
        uint32_t physical_address = (((block->tag << L2_INDEX_BITS) + block->index) << L2_OFFSET_BITS) + block->offset;
        // write the block data to the l2-memory bus
       write_main_memory(mm, gm_subsys->tlb, physical_address);
    }
}