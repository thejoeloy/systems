#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "implicit.h"

void* sbrk(intptr_t increment);

static void* base;
static void* heapListPtr;
static void* maxHeapAddr;
static size_t heapSize;

// Helper functions
Tag* ImpGetHeader(void* ptr) {
	return (Tag*)((char*)ptr - TAG_SIZE);
}

Tag* ImpGetFooter(Tag* currHeader) {
	return (Tag*)((char*)currHeader + ImpGetPayloadSize(currHeader) + TAG_SIZE);
}

Tag* ImpGetNextHeader(Tag* currHeader) {
	size_t payloadSize = ImpGetPayloadSize(currHeader);
	Tag* nextHeader = (Tag*)((char*)currHeader + (payloadSize + TWO_TAG_SIZE));
	return nextHeader;	
}

Tag* ImpGetPrevFooter(Tag* currHeader) {
	return (Tag*)((char*)currHeader - TAG_SIZE);	
}

Tag* ImpGetPrevHeader(Tag* prevFooter) {
	return (Tag*)((char*)prevFooter - ImpGetPayloadSize(prevFooter) - TAG_SIZE);
}

size_t ImpGetAlloc(Tag* t) {
	return t->tag & IS_ALLOC;
}
 
size_t ImpGetPayloadSize(Tag* t) {
	return (size_t)((t->tag & PAYLOAD_SIZE_MASK) >> 3);
}

void ImpSetFree(Tag* t) {
    t->tag &= PAYLOAD_SIZE_MASK;
}

void ImpSetAlloc(Tag* t) {
    t->tag |= IS_ALLOC;
}

void ImpSetPayloadSize(Tag* t, size_t size) {
	size <<= 3;
	t->tag = (t->tag & IS_ALLOC) | size;
}

size_t ImpRoundUp(size_t size, size_t mult) {
	return (size + mult-1) & ~(mult-1);
}

void ImpRightCoalesce(Tag* currHeader) {
    Tag* nextHeader = ImpGetNextHeader(currHeader);

    // Check if next block is free and coalesce if it is
    if (ImpGetAlloc(nextHeader) == IS_FREE) {
        // Set header of base to new payload size
        size_t currPayloadSize = ImpGetPayloadSize(currHeader);
        size_t nextPayloadSize = ImpGetPayloadSize(nextHeader);
        size_t newPayloadSize = nextPayloadSize + currPayloadSize + TWO_TAG_SIZE;
        ImpSetPayloadSize(currHeader, newPayloadSize);
            
        // Create new Footer and set payload size and alloc status
        //Tag* newFooter = (Tag*)((char*)currHeader + newPayloadSize + TAG_SIZE);
        Tag* newFooter = ImpGetFooter(currHeader);
        ImpSetPayloadSize(newFooter, newPayloadSize);
        ImpSetFree(newFooter);
    }
}

void ImpLeftCoalesce(Tag* currHeader) {
    Tag* prevFooter = ImpGetPrevFooter(currHeader);

    // Check if previous block is free and coalesce if it is
    if (ImpGetAlloc(prevFooter) == IS_FREE) {
        // Set header of previous block to new payload size
        Tag* prevHeader = ImpGetPrevHeader(prevFooter);
        size_t prevPayloadSize = ImpGetPayloadSize(prevFooter);
        size_t currPayloadSize = ImpGetPayloadSize(currHeader);
        size_t newPayloadSize = currPayloadSize + prevPayloadSize + TWO_TAG_SIZE;
        ImpSetPayloadSize(prevHeader, newPayloadSize);

        // Set footer of base to new payload size
        //Tag* newFooter = (Tag*)((char*)currHeader + currPayloadSize + TAG_SIZE);
        Tag* newFooter = ImpGetFooter(prevHeader);
        ImpSetPayloadSize(newFooter, newPayloadSize);
        ImpSetFree(newFooter);
    }
}

// Debugging functions
void PrintTag(Tag* t) {
	if (t == NULL) return;
    size_t last3bits = ImpGetAlloc(t);
	printf("Address : %p, payloadSize : %zu, Last 3 bits : %zu\n", (void*)t, ImpGetPayloadSize(t), last3bits);
}

void IterateHeap() {
    printf("IterateHeap\n");
    void* curr = base;
    PrintTag(base);
    curr = (Tag*)((char*)base + TAG_SIZE);
    PrintTag(curr);
    curr = heapListPtr;

    while(curr < maxHeapAddr - TAG_SIZE) {
        Tag* currHeader = (Tag*)curr;
        size_t payloadSize = ImpGetPayloadSize(currHeader);
        Tag* currFooter = (Tag*)((char*)currHeader + TAG_SIZE + payloadSize);
        PrintTag(currHeader);
        PrintTag(currFooter);
        curr = (void*)((char*)currFooter + TAG_SIZE);
    }

    PrintTag((Tag*)((char*)maxHeapAddr - TAG_SIZE));
}

// Functions
int ImpHeapInit(size_t size) {
    // Ensure that requested size is non-zero
    if (size == 0) {
        return 0;
    }

    // Allocate space for heap and ensure that enough space exists
    size_t adjustedSize = ImpRoundUp(size, ALIGN);
    
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
	ImpSetPayloadSize(prologueHeader, TWO_TAG_SIZE);
	ImpSetPayloadSize(prologueFooter, TWO_TAG_SIZE);
	ImpSetAlloc(prologueHeader);
	ImpSetAlloc(prologueFooter);
	ImpSetPayloadSize(epilogue, 0);
	ImpSetAlloc(epilogue);
	
    // Set up initial free block
    Tag* initialHeader = (Tag*)heapListPtr;
    size_t payloadSize = adjustedSize - TWO_TAG_SIZE - SPECIAL_TAG_SIZES;
    ImpSetPayloadSize(initialHeader, payloadSize);
    ImpSetFree(initialHeader);

    // Set up footer tag
    Tag* initialFooter = (Tag*)((char*)heapListPtr + payloadSize + TAG_SIZE);
    ImpSetPayloadSize(initialFooter, payloadSize);
    ImpSetFree(initialFooter);
	
    return 1;
}

void* ImpMalloc(size_t reqSize) {
    // Ensure that requested size is non-zero, return NULL if it is 0
    if (reqSize == 0) {
        return NULL;
    }

    // Requested size must be aligned, round it up if it isnt byte algined
    size_t totalSize = ImpRoundUp(reqSize + TWO_TAG_SIZE, ALIGN);

    // Create pointer to start of heap
    void* curr = heapListPtr;

    // Iterate through blocks until we find a large enough free block
    while (curr < maxHeapAddr) {
        Tag* currHeader = (Tag*)curr;
        size_t payloadSize = ImpGetPayloadSize(currHeader);

        // Check to see if block is free and size of block in order to allocate it
        if (ImpGetAlloc(currHeader) == IS_FREE  && payloadSize >= totalSize) {
            // Case where we need to divide payload into an alloc block and a free block
            // Note that we need space for alloc block as well as footer and header for remaining space
            if (payloadSize >= totalSize + MIN_BLOCK_SIZE) {
                // Set payload and alloc status for current header
                ImpSetPayloadSize(currHeader, totalSize - TWO_TAG_SIZE);
                ImpSetAlloc(currHeader);

                // Set payload and alloc status for current header
                Tag* currFooter = ImpGetFooter(currHeader);
                ImpSetPayloadSize(currFooter, totalSize - TWO_TAG_SIZE);
                ImpSetAlloc(currFooter);

                // Set Header for next block to use remaining space
                Tag* nextHeader = ImpGetNextHeader(currHeader);
                ImpSetPayloadSize(nextHeader, payloadSize - totalSize);
                ImpSetFree(nextHeader);
                
                // Set Footer for next block to use remaining space
                Tag* nextFooter = ImpGetFooter(nextHeader);
                ImpSetPayloadSize(nextFooter, payloadSize - totalSize);
                ImpSetFree(nextFooter);

            }
            // Perfect fit case, simply set tags
            else {
                // Set payload and alloc status for header
                ImpSetPayloadSize(currHeader, payloadSize);
                ImpSetAlloc(currHeader);

                // Set payload and alloc status for footer
                Tag* currFooter = ImpGetFooter(currFooter);
                ImpSetPayloadSize(currFooter, payloadSize);
                ImpSetAlloc(currFooter);

            }
            // Payload starts TAG_SIZE bytes after the header
            return (char*)curr + TAG_SIZE;
        }
        // Advance pointer to next block if current block doesnt have enough space
        curr = (void*)ImpGetNextHeader(currHeader);
    }
    // If we reach end of heap, then there is no space for the allocation in the heap
    return NULL;
}

void ImpFree(void* ptr) {
    // Cant free NULL ptr, so print error message
    if (ptr == NULL) {
        return;
    }
    
    // Free the block
    Tag* currHeader = ImpGetHeader(ptr);
    Tag* currFooter = ImpGetFooter(currHeader);
    ImpSetFree(currHeader);
    ImpSetFree(currFooter);
    
    // coalesce
    ImpRightCoalesce(currHeader);
    ImpLeftCoalesce(currHeader);
}

void* ImpRealloc(void* oldPtr, size_t newPayloadSize) {
    Tag* currHeader = ImpGetHeader(oldPtr);
    size_t oldPayloadSize = ImpGetPayloadSize(currHeader);
    size_t totalNewSize = ImpRoundUp(newPayloadSize + TWO_TAG_SIZE, ALIGN);
    newPayloadSize = totalNewSize - TWO_TAG_SIZE;

    // Shrink case where we can make new block inside realloced block
    if (newPayloadSize + MIN_BLOCK_SIZE < oldPayloadSize) {

        // Set payload for currHeader
        ImpSetPayloadSize(currHeader, newPayloadSize);

        // Create footer for current block and set payload and alloc status
        //Tag* currFooter = (Tag*)((char*)currHeader + TAG_SIZE + newPayloadSize);
        Tag* currFooter = ImpGetFooter(currHeader);
        ImpSetPayloadSize(currFooter, newPayloadSize);
        ImpSetAlloc(currFooter);

        // Create header for new block and set payload and alloc status
        Tag* nextHeader = ImpGetNextHeader(currHeader);
        size_t nextPayloadSize = oldPayloadSize - newPayloadSize - TWO_TAG_SIZE;
        ImpSetPayloadSize(nextHeader, nextPayloadSize);
        ImpSetFree(nextHeader);

        // Create footer for new block and set payload and alloc status
        //Tag* nextFooter = (Tag*)((char*)nextHeader + TAG_SIZE + nextPayloadSize);
        Tag* nextFooter = ImpGetFooter(nextHeader);
        ImpSetPayloadSize(nextFooter, nextPayloadSize);
        ImpSetFree(nextFooter);

        return oldPtr;
    }

    // Shrink / Extend + merge with next block
    Tag* nextHeader = ImpGetNextHeader(currHeader);
    size_t nextPayloadSize = ImpGetPayloadSize(nextHeader);

    if (ImpGetAlloc(nextHeader) == IS_FREE) {
        // Shrink Case
        if (newPayloadSize <= oldPayloadSize) {
            // Set payload and alloc status for header of current block
            ImpSetPayloadSize(currHeader, newPayloadSize);

            // Set payload and alloc status for footer of current block
            //Tag* currFooter = (Tag*)((char*)currHeader + TAG_SIZE + newPayloadSize);
            Tag* currFooter = ImpGetFooter(currHeader);
            ImpSetPayloadSize(currFooter, newPayloadSize);
            ImpSetAlloc(currFooter);

            // Set payload and alloc status for header of next block
            nextPayloadSize = nextPayloadSize + (oldPayloadSize - newPayloadSize);
            //Tag* nextHeader = (Tag*)((char*)currFooter + TAG_SIZE);
            Tag* nextHeader = ImpGetNextHeader(currHeader);
            ImpSetPayloadSize(nextHeader, nextPayloadSize);
            ImpSetFree(nextHeader);

            // Set payload and alloc status for footer of next block
            //Tag* nextFooter = (Tag*)((char*)nextHeader + TAG_SIZE + nextPayloadSize);
            Tag* nextFooter = ImpGetFooter(nextHeader);
            ImpSetPayloadSize(nextFooter, nextPayloadSize);
            ImpSetFree(nextFooter);

            return oldPtr;
        }
        // Extend Case
        if (newPayloadSize - oldPayloadSize <= nextPayloadSize - TWO_TAG_SIZE) {
            // Set payload and alloc status for header of current block
            ImpSetPayloadSize(currHeader, newPayloadSize);

            // Set payload and alloc status for footer of current block
            //Tag* currFooter = (Tag*)((char*)currHeader + TAG_SIZE + newPayloadSize);
            Tag* currFooter = ImpGetFooter(currHeader);
            ImpSetPayloadSize(currFooter, newPayloadSize);
            ImpSetAlloc(currFooter);

            // Set payload and alloc status for header of next block
            nextPayloadSize = nextPayloadSize - (newPayloadSize - oldPayloadSize);
            //Tag* nextHeader = (Tag*)((char*)currFooter + TAG_SIZE);
            Tag* nextHeader = ImpGetNextHeader(currHeader);
            ImpSetPayloadSize(nextHeader, nextPayloadSize);
            ImpSetFree(nextHeader);

            // Set payload and alloc status for footer of next block
            //Tag* nextFooter = (Tag*)((char*)nextHeader + TAG_SIZE + nextPayloadSize);
            Tag* nextFooter = ImpGetFooter(nextHeader);
            ImpSetPayloadSize(nextFooter, nextPayloadSize);
            ImpSetFree(nextFooter);

            return oldPtr;
        }
    }

    // Malloc, memcpy, and free if we cant resize in place
    void* newPtr = ImpMalloc(newPayloadSize);
    memcpy(newPtr, oldPtr, oldPayloadSize);
    ImpFree(oldPtr);
    return newPtr;
}


void ImpHeapDestroy(void) {
	if (heapSize > 0) {
		sbrk(-heapSize); // Deletes heap if it has been allocated previously
		heapSize = 0;
	}
}
