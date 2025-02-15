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

void iterate_heap();
int  imp_heap_init(size_t size);
void imp_heap_destroy(void);
void* imp_malloc(size_t reqSize);
void imp_free(void* ptr);
void* imp_realloc(void* oldPtr, size_t newPayloadSize);

#endif // IMPLICIT_H
