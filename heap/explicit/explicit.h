#ifndef EXPLICIT_H
#define EXPLICIT_H

#include <stddef.h>
#include <stdint.h>

#define ALIGN 8
#define IS_ALLOC 0x00000001
#define IS_FREE 0x00000000
#define PAYLOAD_SIZE_MASK 0xFFFFFFF8
#define TAG_SIZE 4
#define TWO_TAG_SIZE 8
#define PRED_OFFSET 4
#define SUCC_OFFSET 8
#define FREE_LIST_POINTERS_SIZE 8
#define SPECIAL_TAG_SIZES 12
#define MIN_BLOCK_SIZE 24

// Boundary tag Structure
typedef struct tag {
  uint32_t tag;
} tag;

typedef struct free_list_ptr {
    tag* pred;
    tag* succ;
} free_list_ptr;
/*
// tag Helpers
tag* exp_get_header(void* ptr);
tag* exp_get_footer(tag* curr_header);
tag* exp_get_next_header(tag* curr_header);
tag* exp_get_prev_footer(tag* curr_header);
tag* exp_get_prev_header(tag* prev_footer);
size_t exp_get_alloc(tag* t);
size_t exp_get_payload_size(tag* t);
void exp_Set_free(tag* t);
void exp_Set_alloc(tag* t);
void exp_set_payload_size(tag* t, size_t size);
// free_list_ptr helpers
free_list_ptr* exp_get_free_list_ptr(tag* curr_header);
free_list_ptr* exp_get_next_flp(free_list_ptr* curr_flp);
free_list_ptr* exp_get_prev_flp(free_list_ptr* curr_flp);
tag* exp_get_pred(free_list_ptr* flp);
tag* exp_get_succ(free_list_ptr* flp);
void exp_set_pred(free_list_ptr* flp, tag* pred);
void exp_set_succ(free_list_ptr* flp, tag* succ);
// Heap function Helpers
size_t exp_round_up(size_t size, size_t mult);
void exp_insert_free_block(tag* curr_header);
void exp_left_coalesce(tag* curr_header);
void exp_right_coalesce(tag* curr_header);
*/
// Heap Debugging
void iterate_heap();
//void print_tag(tag* t);
// Functions for Heap
int exp_heap_init(size_t size);
void exp_heap_destroy(void);
void* exp_malloc(size_t req_size);
void exp_free(void* ptr);
void* exp_realloc(void* oldPtr, size_t new_payload_size);

#endif // EXPLICIT_H
