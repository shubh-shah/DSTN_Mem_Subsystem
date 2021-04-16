#include "l2_cache.h"


l2_cache *init_l2_cache() {
    l2_cache *cache = (l2_cache *) malloc(sizeof(l2_cache));
    for (int i = 0; i < L2_NUM_SETS; ++i) {
        cache->sets[i].tags = createQueue(L2_SET_SIZE);
    }
    return cache;
}

// Call read_memory() just after calling this, even if it returns true
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

// write to main memory takes place during replacement
// this function is called to write to l2 from l1
// call this function when l2 cache miss occurs and bringing in data from memory
void write_l2_cache(l2_cache *cache, main_memory *mm, uint32_t physical_address) {

    uint8_t offset = physical_address & L2_OFFSET_MASK;
    uint8_t index = (physical_address >> L2_INDEX_SHIFT) & L2_INDEX_MASK;
    uint16_t tag = (physical_address >> L2_TAG_SHIFT) & L2_TAG_MASK;

    l2_cache_set set = cache->sets[index];
    q_node *node = set.tags->front;
    l2_cache_block *current_block;

    for (int i = 0; i < set.tags->node_count; ++i) {
        current_block = node->data_ptr;
        if (current_block->tag == tag || !current_block->valid) { // If either the tag matches or the data is invalid
            // no need to update the tag.
            // update at the desired offset in the block

            // Mark the block dirty and valid
            current_block->dirty = 1;
            current_block->valid = 1;
            // It will be written to main memory during replacement
            return;
        }
        node = node->next;
    }

    // Here, if the tag was not found in the cache
    l2_cache_block new_block = (l2_cache_block *) malloc(sizeof(l2_cache_block));
    new_block.tag = tag;
    new_block.index = index;
    new_block.offset = offset;
    new_block.valid = 1;
    new_block.dirty = 0;
    // while popping out an entry ensure it gets written to main memory
    custom_push(set.tags, new_block, mm);

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
//        write_main_memory(mm, physical_address);
    }
}