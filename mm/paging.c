#include "paging.h"

frame_table* init_frame_table(){
    frame_table* frm_tbl = malloc(sizeof(frame_table));
    for(int i=0;i<NUM_FRAMES;i++){
        frm_tbl->table[i].valid = 0;
    }
    frm_tbl->lru = createQueue(NUM_FRAMES);
    return frm_tbl;
}

/*
Make the frame pointed to by frtbl_ent MRU, i.e. push it to back of LRU Queue
*/
void lru_move_to_back(frame_table* frame_tbl, frame_table_entry* frtbl_ent){
    frame_table_entry* mru_entry = lru_remove_by_frame_tbl_entry(frame_tbl,frtbl_ent);
    push(frame_tbl->lru,mru_entry);
}

/*
Find and remove a node from LRU Queue by matching frame table entry
*/
frame_table_entry* lru_remove_by_frame_tbl_entry(frame_table* frame_tbl, frame_table_entry* frtbl_ent){
    if (isEmpty(frame_tbl->lru))
        return NULL;
    q_node* curr = frame_tbl->lru->front;
    q_node* prev = NULL;
    /* Iterate through LRU queue */
    while(((frame_table_entry*)(curr->data_ptr)) != frtbl_ent) {
        if(curr->next == NULL)
            return NULL;
        prev = curr;
        curr = curr->next;
    }
    /* Remove entry from LRU queue */
    if(curr == frame_tbl->lru->front){
        frame_tbl->lru->front = frame_tbl->lru->front->next;
    }
    else{
        prev->next = curr->next;
    }
    if(curr==frame_tbl->lru->rear){
        frame_tbl->lru->rear = prev;
    }
    frame_tbl->lru->node_count--;
    return (frame_table_entry*)(curr->data_ptr);
}

/*
Find and remove a node from LRU Queue by matching it's pid
In essence get's the LRU frame for a given pid.
Required for local replacement in case a process reaches maximum number of frames.
*/
frame_table_entry* lru_remove_by_pid(frame_table* frame_tbl, int pid){
    if (isEmpty(frame_tbl->lru))
        return NULL;
    q_node* curr = frame_tbl->lru->front;
    q_node* prev = NULL;
    /* Iterate through LRU queue */
    while(((frame_table_entry*)(curr->data_ptr))->pid != pid) {
        if(curr->next == NULL)
            return NULL;
        prev = curr;
        curr = curr->next;
    }
    /* Remove entry from LRU queue */
    if(curr == frame_tbl->lru->front){
        frame_tbl->lru->front = frame_tbl->lru->front->next;
    }
    else{
        prev->next = curr->next;
    }
    if(curr==frame_tbl->lru->rear){
        frame_tbl->lru->rear = prev;
    }
    frame_tbl->lru->node_count--;
    return (frame_table_entry*)(curr->data_ptr);
}