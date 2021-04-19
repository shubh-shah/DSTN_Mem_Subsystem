#include "l2_cache.h"

l2_cache *init_l2_cache() {
    l2_cache *cache = (l2_cache *) malloc(sizeof(l2_cache));
    for (int i = 0; i < L2_NUM_SETS; ++i) {
        cache->sets[i].tags = createQueue(L2_SET_SIZE);
    }
    return cache;
}

/*
   Pushes the New Block at the rear of the Block Queue
   Input:
       Pointer to a Queue of Blocks in a Set
       Pointer to an initialized l2_cache_block
       Pointer to the main_memory of the memory_subsystem
*/
void custom_push(queue *q, void *data, main_memory *mm) {
    // If maximum limit on number of nodes is reached
    if (isFull(q))
        custom_pop(q, mm);

    push(q, data);
}

/*
   Pops a block from the front of the Block Queue and writes it to the Main Memory if dirty(modified)
   Input:
       Pointer to a Queue of Blocks in a Set
       Pointer to the main_memory of the memory_subsystem
*/
void custom_pop(queue *q, main_memory *mm) {

    q_node *node = pop(q);
    l2_cache_block *block = node->data_ptr;
    if (block->dirty) {
        uint32_t physical_address = (((block->tag << L2_INDEX_BITS) + block->index) << L2_OFFSET_BITS) + block->offset;
        // write the block data to the l2-memory bus
        write_main_memory(mm, gm_subsys->tlb, physical_address);
    }
}

/*
   Called when L1 read is a miss. Read is Look Aside, so read from Main Memory immediately after this regardless it returns
   true/false. In case of miss, this function is not called again as data is supplied directly to the processor by the Memory
   Input:
       Physical Address
   Returns:
       True if Read Hit
       False if Read Miss
*/
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

/*
   Called when write to L1 cache occurs. Write Back, so write to main memory takes place during replacement. If returned
   false, read from Main Memory and update the L2 cache. Call this function again and it will be write hit.
   Input:
       Physical Address
   Returns:
       True if Write Hit
       False if Write Miss
*/
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

/*
   Called only after there was a cache read/write miss earlier. Call this to get data from Main Memory to L2 cache
   Input:
       Physical Address
*/
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