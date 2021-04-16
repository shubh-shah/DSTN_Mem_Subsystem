#include "paging.h"

frame_table* init_frame_table(){
    frame_table* frm_tbl = malloc(sizeof(frame_table));
    for(int i=0;i<NUM_FRAMES;i++){
        frm_tbl->table[i].valid = 0;
    }
    frm_tbl->lru = createQueue(NUM_FRAMES);
    return frm_tbl;
}

extern void lru_move_to_back(frame_table* frame_tbl, uint32_t* pt_ent){
    frame_table_entry* mru_entry = lru_remove_by_pgtbl_entry(frame_tbl,pt_ent);
    push(frame_tbl->lru,mru_entry);
}

frame_table_entry* lru_remove_by_pgtbl_entry(frame_table* frame_tbl, uint32_t* pt_ent){
    if (isEmpty(frame_tbl->lru))
        return NULL;        /* Won't Happen */
    q_node* curr = frame_tbl->lru->front;
    q_node* prev = NULL;
    while(((frame_table_entry*)(curr->data_ptr))->page_table_entry != pt_ent) {
        if(curr->next == NULL)
            return NULL;    /* Won't Happen */
        prev = curr;
        curr = curr->next;
    }
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

frame_table_entry* lru_remove_by_pid(frame_table* frame_tbl, int pid){
    if (isEmpty(frame_tbl->lru))
        return NULL;        /* Won't Happen */
    q_node* curr = frame_tbl->lru->front;
    q_node* prev = NULL;
    while(((frame_table_entry*)(curr->data_ptr))->pid != pid) {
        if(curr->next == NULL)
            return NULL;    /* Won't Happen */
        prev = curr;
        curr = curr->next;
    }
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