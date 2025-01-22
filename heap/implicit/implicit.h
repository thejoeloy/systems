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

// Boundary Tag Structure
typedef struct Tag {
  uint32_t tag;
} Tag;

// Header and Footer helper functions
Tag* ImpGetHeader(void* ptr);
Tag* ImpGetFooter(Tag* currHeader);
Tag* ImpGetNextHeader(Tag* currHeader);
Tag* ImpGetPrevFooter(Tag* currHeader);
Tag* ImpGetPrevHeader(Tag* prevFooter);
size_t ImpGetAlloc(Tag* t);
size_t ImpGetPayloadSize(Tag* t);
void ImpSetFree(Tag* t);
void ImpSetAlloc(Tag* t);
void ImpSetPayloadSize(Tag* t, size_t size);
// Heap helper functions
size_t ImpRoundUp(size_t size, size_t mult);
void ImpLeftCoalesce(Tag* currHeader);
void ImpRightCoalesce(Tag* currHeader);
// Heap debugging functions
void PrintTag(Tag* t);
void IterateHeap();
// Heap functions
int  ImpHeapInit(size_t size);
void ImpHeapDestroy(void);
void* ImpMalloc(size_t reqSize);
void ImpFree(void* ptr);
void* ImpRealloc(void* oldPtr, size_t newPayloadSize);

#endif // IMPLICIT_H
