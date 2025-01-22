#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "explicit.h"

void* sbrk(intptr_t increment);

static void* base;
static void* heapListPtr;
static void* maxHeapAddr;
static size_t heapSize;
static Tag* freeListStart;

// Tag helpers
Tag* ExpGetHeader(void* ptr) {
	return (Tag*)((char*)ptr - TAG_SIZE);
}

Tag* ExpGetFooter(Tag* currHeader) {
	return (Tag*)((char*)currHeader + ExpGetPayloadSize(currHeader) + TAG_SIZE);
}

Tag* ExpGetNextHeader(Tag* currHeader) {
	size_t payloadSize = ExpGetPayloadSize(currHeader);
	Tag* nextHeader = (Tag*)((char*)currHeader + (payloadSize + TWO_TAG_SIZE));
	return nextHeader;	
}

Tag* ExpGetPrevFooter(Tag* currHeader) {
	return (Tag*)((char*)currHeader - TAG_SIZE);	
}

Tag* ExpGetPrevHeader(Tag* prevFooter) {
	return (Tag*)((char*)prevFooter - ExpGetPayloadSize(prevFooter) - TAG_SIZE);
}

size_t ExpGetPayloadSize(Tag* t) {
	return (size_t)((t->tag & PAYLOAD_SIZE_MASK) >> 3);
}

size_t ExpGetAlloc(Tag* t) {
	return t->tag & IS_ALLOC;
}

void ExpSetFree(Tag* t) {
    t->tag &= PAYLOAD_SIZE_MASK;
}

void ExpSetAlloc(Tag* t) {
    t->tag |= IS_ALLOC;
}

void ExpSetPayloadSize(Tag* t, size_t size) {
	size <<= 3;
	t->tag = (t->tag & IS_ALLOC) | size;
}

// FreeListPtr helpers
FreeListPtr* ExpGetFreeListPtr(Tag* currHeader) {
    return (FreeListPtr*)((char*)currHeader + TAG_SIZE);
}

FreeListPtr* ExpGetNextFreeListPtr(FreeListPtr* currFLP) {
    return ExpGetFreeListPtr(currFLP->succ);
}

FreeListPtr* ExpGetPrevFreeListPtr(FreeListPtr* currFLP) {
    return ExpGetFreeListPtr(currFLP->pred);
}

Tag* ExpGetPred(FreeListPtr* flp) {
    return flp->pred;
}

Tag* ExpGetSucc(FreeListPtr* flp) {
    return flp->succ;
}

void ExpSetPred(FreeListPtr* flp, Tag* pred) {
    flp->pred = pred;
}

void ExpSetSucc(FreeListPtr* flp, Tag* succ) {
    flp->succ = succ;
}

// Heap function helpers
size_t ExpRoundUp(size_t size, size_t mult) {
	return (size + mult-1) & ~(mult-1);
}

void ExpInsertFreeBlock(Tag* currHeader) {
    FreeListPtr* newFLP = ExpGetFreeListPtr(currHeader);
    Tag* iterHeader = freeListStart;
    FreeListPtr* iterFLP = ExpGetFreeListPtr(iterHeader);
    
    // In this case we insert to front of list
    if (iterHeader > currHeader) {
        ExpSetPred(newFLP, iterFLP->pred);
        ExpSetSucc(newFLP, iterHeader);
        ExpSetPred(iterFLP, currHeader);
        freeListStart = currHeader;
    }

    // Otherwise we traverse the list
    while(iterHeader != NULL) {
        // Inserting to somewhere in the middle of the free list
        if (currHeader > iterHeader && currHeader < iterFLP->succ && iterFLP->succ != NULL) {
            FreeListPtr* iterNextFLP = ExpGetNextFreeListPtr(iterFLP);
            Tag* iterNextHeader = ExpGetHeader(iterNextFLP);
            ExpSetPred(iterNextFLP, currHeader);

            ExpSetPred(newFLP, iterHeader);
            ExpSetSucc(newFLP, iterNextHeader);

            ExpSetSucc(iterFLP, currHeader);
            break;
        }
        // Inserting to the end of the free list
        if (currHeader > iterHeader && iterFLP->succ == NULL) {
            ExpSetPred(newFLP, iterHeader);
            ExpSetSucc(newFLP, iterFLP->succ);
            
            ExpSetSucc(iterFLP, currHeader);
            break;
        }

        // Increment the free list pointer iterators
        iterHeader = iterFLP->succ;
        iterFLP = ExpGetFreeListPtr(iterHeader);

    }
}

void ExpRightCoalesce(Tag* currHeader) {
    Tag* nextHeader = ExpGetNextHeader(currHeader);

    // Check if next block is free and coalesce if it is
    if (ExpGetAlloc(nextHeader) == IS_FREE) {
        // Set header of base to new payload size
        size_t currPayloadSize = ExpGetPayloadSize(currHeader);
        size_t nextPayloadSize = ExpGetPayloadSize(nextHeader);
        size_t newPayloadSize = nextPayloadSize + currPayloadSize + TWO_TAG_SIZE;
        ExpSetPayloadSize(currHeader, newPayloadSize);
            
        // Create new Footer and set payload size and alloc status
        Tag* newFooter = (Tag*)((char*)currHeader + newPayloadSize + TAG_SIZE);
        ExpSetPayloadSize(newFooter, newPayloadSize);
        ExpSetFree(newFooter);

        // Update free list pointers
        FreeListPtr* currFLP = ExpGetFreeListPtr(currHeader);
        FreeListPtr* nextFLP = ExpGetFreeListPtr(nextHeader);
        FreeListPtr* nextPredFLP = ExpGetPrevFreeListPtr(nextFLP);
        FreeListPtr* nextSuccFLP = ExpGetNextFreeListPtr(nextFLP);
        
        // If predecessor to nextFLP doesnt exist, currHeader is new start to free list
        if (nextFLP->pred == NULL || nextFLP->pred == currHeader) {
            freeListStart = currHeader;
            ExpSetPred(currFLP, NULL);
        }
        // If predecessor to nextFLP does exist, make it currHeaders predecessor and currHeader should be its new successor
        else {
            ExpSetPred(currFLP, nextFLP->pred);
            ExpSetSucc(nextPredFLP, currHeader);
        }
        // If successor to nextFLP exists, make currHeader its predecessor and set nextFLP as its successor
        if (nextFLP->succ != NULL) {
            ExpSetSucc(currFLP, nextFLP->succ);
            ExpSetPred(nextSuccFLP, currHeader);
        }
        else {
            ExpSetSucc(currFLP, nextFLP->succ);
        }
    }
}

void ExpLeftCoalesce(Tag* currHeader) {
    Tag* prevFooter = ExpGetPrevFooter(currHeader);

    // Check if previous block is free and coalesce if it is
    if (ExpGetAlloc(prevFooter) == IS_FREE) {
        // Set header of previous block to new payload size
        Tag* prevHeader = ExpGetPrevHeader(prevFooter);
        size_t prevPayloadSize = ExpGetPayloadSize(prevFooter);
        size_t currPayloadSize = ExpGetPayloadSize(currHeader);
        size_t newPayloadSize = currPayloadSize + prevPayloadSize + TWO_TAG_SIZE;
        ExpSetPayloadSize(prevHeader, newPayloadSize);

        // Set footer of base to new payload size
        Tag* newFooter = (Tag*)((char*)currHeader + currPayloadSize + TAG_SIZE);
        ExpSetPayloadSize(newFooter, newPayloadSize);
        ExpSetFree(newFooter);

        // Update free list pointers
        FreeListPtr* currFLP = ExpGetFreeListPtr(currHeader);
        FreeListPtr* prevFLP = ExpGetFreeListPtr(prevHeader);
        FreeListPtr* currSuccFLP = ExpGetNextFreeListPtr(currFLP);

        // Set predecessor of successor to currHeader to prevHeader if it exists 
        if (currFLP->succ != NULL) {
            ExpSetPred(currSuccFLP, prevHeader);
        }

        // Set successor to new FLP
        ExpSetSucc(prevFLP, currFLP->succ);

        // Update freeListStart to prevHeader if currHeader was previously start of free list
        if (currFLP->pred == NULL) {
            freeListStart = prevHeader;
        }

    }
}

// Heap debugging
void PrintHeader(Tag* currHeader) {
	if (currHeader == NULL) return;
	size_t last3bits = ExpGetAlloc(currHeader);
    printf("Address : %p, payloadSize : %zu, Last 3 bits : %zu\n", (void*)currHeader, ExpGetPayloadSize(currHeader), last3bits);
    if (last3bits == IS_FREE) {
        FreeListPtr* flp = ExpGetFreeListPtr(currHeader);
        printf("pred_addr : %p, pred : %p\n", (void*)((char*)currHeader + PRED_OFFSET), (void*)flp->pred);
        printf("succ_addr : %p, succ : %p\n", (void*)((char*)currHeader + SUCC_OFFSET), (void*)flp->succ);
    }
}

void PrintFooter(Tag* currFooter) {
    if (currFooter == NULL) return;
    size_t last3bits = ExpGetAlloc(currFooter);
    printf("Address : %p, payloadSize : %zu, Last 3 bits : %zu\n", (void*)currFooter, ExpGetPayloadSize(currFooter), last3bits);
}

void IterateHeap() {
    printf("IterateHeap\n");
    printf("freeListStart: %p\n", freeListStart);
    void* curr = base;
    PrintFooter(base);
    curr = (Tag*)((char*)base + TAG_SIZE);
    PrintFooter(curr);
    curr = heapListPtr;

    while(curr < maxHeapAddr - TAG_SIZE) {
        Tag* currHeader = (Tag*)curr;
        size_t payloadSize = ExpGetPayloadSize(currHeader);
        Tag* currFooter = (Tag*)((char*)currHeader + TAG_SIZE + payloadSize);
        PrintHeader(currHeader);
        PrintFooter(currFooter);
        curr = (void*)((char*)currFooter + TAG_SIZE);
    }

    PrintFooter((Tag*)((char*)maxHeapAddr - TAG_SIZE));
}

// Heap Functions
int ExpHeapInit(size_t size) {
    // Ensure that requested size is non-zero
    if (size == 0) {
        return 0;
    }

    // Allocate space for heap and ensure that enough space exists
    size_t adjustedSize = ExpRoundUp(size, ALIGN);
    
    // Allocate space for the heap and ensure that the allocation doesnt fail
    base = sbrk(adjustedSize);
    if (base == (void*)-1) {
        return 0; // sbrk failed
    }
    // Initialize heap pointers
    maxHeapAddr = (char*)base + adjustedSize;
    heapSize = adjustedSize;
    heapListPtr = (char*)base + TWO_TAG_SIZE;

	// Setup Prologue and Epilogue blocks
	Tag* prologueHeader = (Tag*)base;
	Tag* prologueFooter = (Tag*)((char*)base + TAG_SIZE);
	Tag* epilogue = (Tag*)((char*)maxHeapAddr - TAG_SIZE);
	ExpSetPayloadSize(prologueHeader, TWO_TAG_SIZE);
	ExpSetPayloadSize(prologueFooter, TWO_TAG_SIZE);
	ExpSetAlloc(prologueHeader);
	ExpSetAlloc(prologueFooter);
	ExpSetPayloadSize(epilogue, 0);
	ExpSetAlloc(epilogue);
	
    // Set up initial header
    Tag* initialHeader = (Tag*)heapListPtr;
    size_t payloadSize = adjustedSize - TWO_TAG_SIZE - SPECIAL_TAG_SIZES;
    ExpSetPayloadSize(initialHeader, payloadSize);
    ExpSetFree(initialHeader);

    // Set up initial free list
    FreeListPtr* flp = ExpGetFreeListPtr(initialHeader);
    Tag* initPtr = NULL;
    ExpSetPred(flp, initPtr);
    ExpSetSucc(flp, initPtr);
    freeListStart = initialHeader;

    // Set up footer tag
    Tag* initialFooter = ExpGetFooter(initialHeader);
    ExpSetPayloadSize(initialFooter, payloadSize);
    ExpSetFree(initialFooter);
	
    return 1;
}

void* ExpMalloc(size_t reqSize) {
    // Ensure that requested size is non zero
    if (reqSize == 0) {
        return NULL;
    }

    // Allocate space for new block and ensure that it is aligned and atleast MIN_BLOCK_SIZE
    size_t totalSize = ExpRoundUp(reqSize + TWO_TAG_SIZE, ALIGN);
    totalSize = (totalSize >= MIN_BLOCK_SIZE) ? totalSize : MIN_BLOCK_SIZE;

    Tag* iterHeader = freeListStart;

    // Iterate through free list until we find a space to insert new block
    while (iterHeader != NULL) {
        FreeListPtr* iterFLP = ExpGetFreeListPtr(iterHeader);
        size_t payloadSize = ExpGetPayloadSize(iterHeader);

        if (payloadSize >= totalSize) {
            // Case where we can alloc requested block and create new free block with remaining space
            if (payloadSize >= totalSize + MIN_BLOCK_SIZE) {
                // Set payload and alloc status for current header
                ExpSetPayloadSize(iterHeader, totalSize - TWO_TAG_SIZE);
                ExpSetAlloc(iterHeader);

                // Set payload and alloc status for current footer
                Tag* iterFooter = ExpGetFooter(iterHeader);
                ExpSetPayloadSize(iterFooter, totalSize - TWO_TAG_SIZE);
                ExpSetAlloc(iterFooter);

                Tag* nextHeader = ExpGetNextHeader(iterHeader);
                ExpSetPayloadSize(nextHeader, payloadSize - totalSize);
                ExpSetFree(nextHeader);

                Tag* nextFooter = ExpGetFooter(nextHeader);
                ExpSetPayloadSize(nextFooter, payloadSize - totalSize);
                ExpSetFree(nextFooter);

                // Update Free List Pointers
                FreeListPtr* nextFLP = ExpGetFreeListPtr(nextHeader);
                // Case where we are at beginning of list
                if (iterFLP->pred == NULL) {
                    freeListStart = nextHeader;
                    ExpSetPred(nextFLP, iterFLP->pred);
                    // Modify successor if it exists
                    if (iterFLP->succ != NULL) {
                        FreeListPtr* succFLP = ExpGetFreeListPtr(iterFLP->succ);
                        Tag* succHeader = ExpGetHeader(succFLP);
                        ExpSetPred(succFLP, nextHeader);
                        ExpSetSucc(nextFLP, succHeader);
                    }
                }
                
                // Case where we are at end of list
                if (iterFLP->succ == NULL) {
                    ExpSetSucc(nextFLP, iterFLP->succ);
                    // Modify predecessor if it exists
                    if (iterFLP->pred != NULL) {
                        FreeListPtr* predFLP = ExpGetFreeListPtr(iterFLP->pred);
                        Tag* predHeader = ExpGetHeader(predFLP);
                        ExpSetSucc(predFLP, nextHeader);
                        ExpSetPred(nextFLP, predHeader);
                    }
                }
                
                // Case where we are at middle of list
                if (iterFLP->pred != NULL && iterFLP->succ != NULL) {
                    FreeListPtr* predFLP = ExpGetFreeListPtr(iterFLP->pred);
                    Tag* predHeader = ExpGetHeader(predFLP);
                    FreeListPtr* succFLP = ExpGetFreeListPtr(iterFLP->succ);
                    Tag* succHeader = ExpGetHeader(succFLP);

                    ExpSetSucc(predFLP, nextHeader);
                    ExpSetPred(nextFLP, predHeader);

                    ExpSetPred(succFLP, nextHeader);
                    ExpSetSucc(nextFLP, succHeader);
                }
            }
            // Case where we cant create new free block, so just use all of the current block
            else {
                // Set payload and alloc status for current header
                ExpSetPayloadSize(iterHeader, payloadSize);
                ExpSetAlloc(iterHeader);

                // Set payload and alloc status for current footer
                Tag* iterFooter = ExpGetFooter(iterHeader);
                ExpSetPayloadSize(iterFooter, payloadSize);
                ExpSetAlloc(iterFooter);

                // Update Free List Pointers
                // Case where we are at beginning of list
                if (iterFLP->pred == NULL) {
                    freeListStart = iterFLP->succ;
                }
                // Case where we are at end of list
                if (iterFLP->succ == NULL) {
                    FreeListPtr* predFLP = ExpGetFreeListPtr(iterFLP->pred);
                    ExpSetSucc(predFLP, iterFLP->succ);
                }

                // Case where we are at middle of list
                if (iterFLP->pred != NULL && iterFLP->succ != NULL) {
                    FreeListPtr* predFLP = ExpGetPrevFreeListPtr(iterFLP);
                    Tag* predHeader = ExpGetHeader(predFLP);
                    FreeListPtr* succFLP = ExpGetNextFreeListPtr(iterFLP);
                    Tag* succHeader = ExpGetHeader(succFLP);

                    ExpSetSucc(predFLP, succHeader);
                    ExpSetPred(succFLP, predHeader);
                }
            }
            return (char*)iterHeader + TAG_SIZE;
        }
        iterHeader = iterFLP->succ;
    }
    // If we reach end of list, then there is not a large enough block of free space for the allocation
    return NULL;
}

void ExpFree(void* ptr) {
    // Cant free NULL ptr, so print error message
    if (ptr == NULL) {
        return;
    }
    // Free the block
    Tag* currHeader = ExpGetHeader(ptr);
    Tag* currFooter = ExpGetFooter(currHeader);
    ExpSetFree(currHeader);
    ExpSetFree(currFooter);
    ExpInsertFreeBlock(currHeader);
    
    // coalesce
    ExpRightCoalesce(currHeader);
    ExpLeftCoalesce(currHeader);
}

void* ExpRealloc(void* oldPtr, size_t newPayloadSize) {
    Tag* currHeader = ExpGetHeader(oldPtr);
    size_t oldPayloadSize = ExpGetPayloadSize(currHeader);
    size_t totalNewSize = ExpRoundUp(newPayloadSize + TWO_TAG_SIZE, ALIGN);
    totalNewSize = (totalNewSize >= MIN_BLOCK_SIZE) ? totalNewSize : MIN_BLOCK_SIZE;
    newPayloadSize = totalNewSize - TWO_TAG_SIZE;

    // Shrink case where we can make new block inside realloced block
    if (newPayloadSize + MIN_BLOCK_SIZE <= oldPayloadSize) {
        // Set payload for currHeader
        ExpSetPayloadSize(currHeader, newPayloadSize);

        // Create footer for current block and set payload and alloc status
        Tag* currFooter = ExpGetFooter(currHeader);
        ExpSetPayloadSize(currFooter, newPayloadSize);
        ExpSetAlloc(currFooter);

        // Create header for new block and set payload and alloc status
        Tag* nextHeader = ExpGetNextHeader(currHeader);
        size_t nextPayloadSize = oldPayloadSize - newPayloadSize - TWO_TAG_SIZE;
        ExpSetPayloadSize(nextHeader, nextPayloadSize);
        ExpSetFree(nextHeader);

        // Create footer for new block and set payload and alloc status
        Tag* nextFooter = ExpGetFooter(nextHeader);
        ExpSetPayloadSize(nextFooter, nextPayloadSize);
        ExpSetFree(nextFooter);

        // Update Free List Pointers
        ExpInsertFreeBlock(nextHeader);

        return oldPtr;
    }

    // Shrink / Extend + merge with next block
    Tag* nextHeader = ExpGetNextHeader(currHeader);
    size_t nextPayloadSize = ExpGetPayloadSize(nextHeader);

    if (ExpGetAlloc(nextHeader) == IS_FREE) {
        // Shrink Case
        FreeListPtr* nextFLP = ExpGetFreeListPtr(nextHeader);
        FreeListPtr* nextPredFLP = ExpGetPrevFreeListPtr(nextFLP);
        FreeListPtr* nextSuccFLP = ExpGetNextFreeListPtr(nextFLP);
        Tag* predFreeBlock = ExpGetPred(nextFLP);
        Tag* succFreeBlock = ExpGetSucc(nextFLP);
        if (newPayloadSize <= oldPayloadSize) {
            // Set payload and alloc status for header of current block
            ExpSetPayloadSize(currHeader, newPayloadSize);

            // Set payload and alloc status for footer of current block
            Tag* currFooter = ExpGetFooter(currHeader);
            ExpSetPayloadSize(currFooter, newPayloadSize);
            ExpSetAlloc(currFooter);

            // Set payload and alloc status for header of next block
            nextPayloadSize = nextPayloadSize + (oldPayloadSize - newPayloadSize);
            Tag* nextHeader = ExpGetNextHeader(currHeader);
            ExpSetPayloadSize(nextHeader, nextPayloadSize);
            ExpSetFree(nextHeader);

            // Set payload and alloc status for footer of next block
            Tag* nextFooter = ExpGetFooter(nextHeader);
            ExpSetPayloadSize(nextFooter, nextPayloadSize);
            ExpSetFree(nextFooter);

            // Update Free List Pointers
            nextFLP = ExpGetFreeListPtr(nextHeader);
            ExpSetPred(nextFLP, predFreeBlock);
            ExpSetSucc(nextFLP, succFreeBlock);

            // Update predecessor block successor and successor block predecessor
            // Update predecessor block successor if it exists or make nextHeader beginning of free list
            if (predFreeBlock == NULL) {
                freeListStart = nextHeader;
            }
            else {
                ExpSetSucc(nextPredFLP, nextHeader);
            }
            // Update successor block predecessor if it exists
            if (succFreeBlock != NULL) {
                ExpSetPred(nextSuccFLP, nextHeader);    
            }

            return oldPtr;
        }
        // Extend Case
        if (newPayloadSize - oldPayloadSize <= nextPayloadSize - TWO_TAG_SIZE - FREE_LIST_POINTERS_SIZE) {
            // Set payload and alloc status for header of current block
            ExpSetPayloadSize(currHeader, newPayloadSize);

            // Set payload and alloc status for footer of current block
            Tag* currFooter = ExpGetFooter(currHeader);
            ExpSetPayloadSize(currFooter, newPayloadSize);
            ExpSetAlloc(currFooter);

            // Set payload and alloc status for header of next block
            nextPayloadSize = nextPayloadSize - (newPayloadSize - oldPayloadSize);
            Tag* nextHeader = ExpGetNextHeader(currHeader);
            ExpSetPayloadSize(nextHeader, nextPayloadSize);
            ExpSetFree(nextHeader);

            // Set payload and alloc status for footer of next block
            Tag* nextFooter = ExpGetFooter(nextHeader);
            ExpSetPayloadSize(nextFooter, nextPayloadSize);
            ExpSetFree(nextFooter);

            // Update free list pointers
            nextFLP = ExpGetFreeListPtr(nextHeader);
            ExpSetPred(nextFLP, predFreeBlock);
            ExpSetSucc(nextFLP, succFreeBlock);

            // Update predecessor block successor and successor block predecessor
            // Update predecessor block successor if it exists or make nextHeader beginning of free list
            if (predFreeBlock == NULL) {
                freeListStart = nextHeader;
            }
            else {
                ExpSetSucc(nextPredFLP, nextHeader);
            }
            if (succFreeBlock != NULL) {
                ExpSetPred(nextSuccFLP, nextHeader);
            }
            return oldPtr;
        }
    }
    // Malloc, memcpy, and free if we cant resize in place
    void* newPtr = ExpMalloc(newPayloadSize);
    memcpy(newPtr, oldPtr, oldPayloadSize);
    ExpFree(oldPtr);
    return newPtr;
}

void ExpHeapDestroy(void) {
	if (heapSize > 0) {
		sbrk(-heapSize); // Deletes heap if it has been allocated previously
		heapSize = 0;
	}
}
