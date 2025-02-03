#ifndef IMPLICIT_H
#define IMPLICIT_H

#include <stddef.h>
#include <stdint.h>

#define ALIGN 8
#define IS_ALLOC 0x00000001
#define IS_FREE 0x00000000
#define PAYLOAD_SIZE_MASK 0xFFFFFFF8
#define TAG_SIZE 4
#define TWO_TAG_SIZE 8
#define SPECIAL_TAG_SIZES 12
#define MIN_BLOCK_SIZE 16

// Boundary tag Structure
typedef struct tag {
  uint32_t tag;
} tag;

// Header and Footer helper functions
tag* imp_get_header(void* ptr);
tag* imp_get_footer(tag* currHeader);
tag* imp_get_next_header(tag* currHeader);
tag* imp_get_prev_footer(tag *currHeader);
tag* imp_get_prev_header(tag* prevFooter);
size_t imp_get_alloc(tag* t);
size_t imp_get_payload_size(tag* t);
void imp_set_free(tag* t);
void imp_set_alloc(tag* t);
void imp_set_payload_size(tag* t, size_t size);
// Heap helper functions
size_t imp_round_up(size_t size, size_t mult);
void imp_left_coalesce(tag* currHeader);
void imp_right_coalesce(tag* currHeader);
// Heap debugging functions
void print_tag(tag* t);
void iterate_heap();
// Heap functions
int  imp_heap_init(size_t size);
void imp_heap_destroy(void);
void* imp_malloc(size_t reqSize);
void imp_free(void* ptr);
void* imp_realloc(void* oldPtr, size_t newPayloadSize);

#endif // IMPLICIT_H
