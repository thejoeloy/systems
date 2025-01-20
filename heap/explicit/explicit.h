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

// Boundary Tag Structure
typedef struct Tag {
  uint32_t tag;
} Tag;

typedef struct FreeListPtr {
    Tag* pred;
    Tag* succ;
} FreeListPtr;

// Tag Helpers
Tag* ExpGetHeader(void* ptr);
Tag* ExpGetFooter(Tag* currHeader);
Tag* ExpGetNextHeader(Tag* currHeader);
Tag* ExpGetPrevFooter(Tag* currHeader);
Tag* ExpGetPrevHeader(Tag* prevFooter);
size_t ExpGetAlloc(Tag* t);
size_t ExpGetPayloadSize(Tag* t);
void ExpSetFree(Tag* t);
void ExpSetAlloc(Tag* t);
void ExpSetPayloadSize(Tag* t, size_t size);
// FreeListPtr helpers
FreeListPtr* ExpGetFreeListPtr(Tag* currHeader);
FreeListPtr* ExpGetNextFreeListPtr(FreeListPtr* currFLP);
FreeListPtr* ExpGetPrevFreeListPtr(FreeListPtr* currFLP);
Tag* ExpGetPred(FreeListPtr* flp);
Tag* ExpGetSucc(FreeListPtr* flp);
void ExpSetPred(FreeListPtr* flp, Tag* pred);
void ExpSetSucc(FreeListPtr* flp, Tag* succ);
// Heap function Helpers
size_t ExpRoundUp(size_t size, size_t mult);
void ExpInsertFreeBlock(Tag* currHeader);
void ExpLeftCoalesce(Tag* currHeader);
void ExpRightCoalesce(Tag* currHeader);
// Heap Debugging
void IterateHeap();
void PrintTag(Tag* t);
// Functions for Heap
int ExpHeapInit(size_t size);
void ExpHeapDestroy(void);
void* ExpMalloc(size_t reqSize);
void ExpFree(void* ptr);
void* ExpRealloc(void* oldPtr, size_t newPayloadSize);

#endif // EXPLICIT_H
