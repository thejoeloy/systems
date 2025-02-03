#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "implicit.h"

void* sbrk(intptr_t increment);

static void* base;
static void* heap_list_ptr;
static void* max_heap_addr;
static size_t heap_size;

// Helper functions
tag* 
imp_get_header (void* ptr) 
{
	return (tag*)((char*)ptr - TAG_SIZE);
}

tag* 
imp_get_footer (tag* curr_header) 
{
	return (tag*)((char*)curr_header + imp_get_payload_size(curr_header) + TAG_SIZE);
}

tag* 
imp_get_next_header (tag* curr_header) {
	size_t payload_size = imp_get_payload_size(curr_header);
	tag* next_header = (tag*)((char*)curr_header + (payload_size + TWO_TAG_SIZE));
	return next_header;	
}

tag* 
imp_get_prev_footer (tag* curr_header) 
{
	return (tag*)((char*)curr_header - TAG_SIZE);	
}

tag* 
imp_get_prev_header (tag* prev_footer) 
{
	return (tag*)((char*)prev_footer - imp_get_payload_size(prev_footer) - TAG_SIZE);
}

size_t 
imp_get_alloc (tag* t) 
{
	return t->tag & IS_ALLOC;
}
 
size_t 
imp_get_payload_size (tag* t) 
{
	return (size_t)((t->tag & PAYLOAD_SIZE_MASK) >> 3);
}

void 
imp_set_free (tag* t) 
{
    t->tag &= PAYLOAD_SIZE_MASK;
}

void 
imp_set_alloc (tag* t) 
{
    t->tag |= IS_ALLOC;
}

void 
imp_set_payload_size (tag* t, size_t size) 
{
	size <<= 3;
	t->tag = (t->tag & IS_ALLOC) | size;
}

size_t 
imp_round_up (size_t size, size_t mult) 
{
	return (size + mult-1) & ~(mult-1);
}

void 
imp_right_coalesce (tag* curr_header) 
{
    tag* next_header = imp_get_next_header(curr_header);

    // Check if next block is free and coalesce if it is
    if (imp_get_alloc(next_header) == IS_FREE) {
        // Set header of base to new payload size
        size_t curr_payload_size = imp_get_payload_size(curr_header);
        size_t next_payload_size = imp_get_payload_size(next_header);
        size_t new_payload_size = next_payload_size + curr_payload_size + TWO_TAG_SIZE;
        imp_set_payload_size(curr_header, new_payload_size);
            
        // Create new Footer and set payload size and alloc status
        tag* new_footer = imp_get_footer(curr_header);
        imp_set_payload_size(new_footer, new_payload_size);
        imp_set_free(new_footer);
    }
}

void 
imp_left_coalesce (tag* curr_header) 
{
    tag* prev_footer = imp_get_prev_footer(curr_header);

    // Check if previous block is free and coalesce if it is
    if (imp_get_alloc(prev_footer) == IS_FREE) {
        // Set header of previous block to new payload size
        tag* prev_header = imp_get_prev_header(prev_footer);
        size_t prev_payload_size = imp_get_payload_size(prev_footer);
        size_t curr_payload_size = imp_get_payload_size(curr_header);
        size_t new_payload_size = curr_payload_size + prev_payload_size + TWO_TAG_SIZE;
        imp_set_payload_size(prev_header, new_payload_size);

        // Set footer of base to new payload size
        tag* new_footer = imp_get_footer(prev_header);
        imp_set_payload_size(new_footer, new_payload_size);
        imp_set_free(new_footer);
    }
}

// Debugging functions
void 
print_tag (tag* t) 
{
	if (t == NULL) return;
    size_t last3bits = imp_get_alloc(t);
	printf("Address : %p, payload_size : %zu, Last 3 bits : %zu\n", (void*)t, imp_get_payload_size(t), last3bits);
}

void 
iterate_heap () 
{
    printf("iterate_heap\n");
    void* curr = base;
    print_tag(base);
    curr = (tag*)((char*)base + TAG_SIZE);
    print_tag(curr);
    curr = heap_list_ptr;

    while(curr < max_heap_addr - TAG_SIZE) {
        tag* curr_header = (tag*)curr;
        size_t payload_size = imp_get_payload_size(curr_header);
        tag* curr_footer = (tag*)((char*)curr_header + TAG_SIZE + payload_size);
        print_tag(curr_header);
        print_tag(curr_footer);
        curr = (void*)((char*)curr_footer + TAG_SIZE);
    }

    print_tag((tag*)((char*)max_heap_addr - TAG_SIZE));
}

// Functions
int 
imp_heap_init (size_t size) 
{
    // Ensure that requested size is non-zero
    if (size == 0) {
        return 0;
    }

    // Allocate space for heap and ensure that enough space exists
    size_t adjusted_size = imp_round_up(size, ALIGN);
    
    // Allocate space for the heap and ensure that the allocation doesnt fail
    base = sbrk(adjusted_size);
    if (base == (void*)-1) {
        return 0; // sbrk failed
    }
    // Initialize heap pointers
    max_heap_addr = (char*)base + adjusted_size;
    heap_size = adjusted_size;
    heap_list_ptr = (char*)base + TWO_TAG_SIZE;

	// Setup Prologue and Epilogue blocks
	tag* prologue_header = (tag*)base;
	tag* prologue_footer = (tag*)((char*)base + TAG_SIZE);
	tag* epilogue = (tag*)((char*)max_heap_addr - TAG_SIZE);
	imp_set_payload_size(prologue_header, TWO_TAG_SIZE);
	imp_set_payload_size(prologue_footer, TWO_TAG_SIZE);
	imp_set_alloc(prologue_header);
	imp_set_alloc(prologue_footer);
	imp_set_payload_size(epilogue, 0);
	imp_set_alloc(epilogue);
	
    // Set up initial free block
    tag* initial_header = (tag*)heap_list_ptr;
    size_t payload_size = adjusted_size - TWO_TAG_SIZE - SPECIAL_TAG_SIZES;
    imp_set_payload_size(initial_header, payload_size);
    imp_set_free(initial_header);

    // Set up footer tag
    tag* initial_footer = (tag*)((char*)heap_list_ptr + payload_size + TAG_SIZE);
    imp_set_payload_size(initial_footer, payload_size);
    imp_set_free(initial_footer);
	
    return 1;
}

void* 
imp_malloc (size_t req_size) 
{
    // Ensure that requested size is non-zero, return NULL if it is 0
    if (req_size == 0) {
        return NULL;
    }

    // Requested size must be aligned, round it up if it isnt byte algined
    size_t total_size = imp_round_up(req_size + TWO_TAG_SIZE, ALIGN);

    // Create pointer to start of heap
    void* curr = heap_list_ptr;

    // Iterate through blocks until we find a large enough free block
    while (curr < max_heap_addr) {
        tag* curr_header = (tag*)curr;
        size_t payload_size = imp_get_payload_size(curr_header);

        // Check to see if block is free and size of block in order to allocate it
        if (imp_get_alloc(curr_header) == IS_FREE  && payload_size >= total_size) {
            // Case where we need to divide payload into an alloc block and a free block
            // Note that we need space for alloc block as well as footer and header for remaining space
            if (payload_size >= total_size + MIN_BLOCK_SIZE) {
                // Set payload and alloc status for current header
                imp_set_payload_size(curr_header, total_size - TWO_TAG_SIZE);
                imp_set_alloc(curr_header);

                // Set payload and alloc status for current header
                tag* curr_footer = imp_get_footer(curr_header);
                imp_set_payload_size(curr_footer, total_size - TWO_TAG_SIZE);
                imp_set_alloc(curr_footer);

                // Set Header for next block to use remaining space
                tag* next_header = imp_get_next_header(curr_header);
                imp_set_payload_size(next_header, payload_size - total_size);
                imp_set_free(next_header);
                
                // Set Footer for next block to use remaining space
                tag* next_footer = imp_get_footer(next_header);
                imp_set_payload_size(next_footer, payload_size - total_size);
                imp_set_free(next_footer);

            }
            // Perfect fit case, simply set tags
            else {
                // Set payload and alloc status for header
                imp_set_payload_size(curr_header, payload_size);
                imp_set_alloc(curr_header);

                // Set payload and alloc status for footer
                tag* curr_footer = imp_get_footer(curr_footer);
                imp_set_payload_size(curr_footer, payload_size);
                imp_set_alloc(curr_footer);

            }
            // Payload starts TAG_SIZE bytes after the header
            return (char*)curr + TAG_SIZE;
        }
        // Advance pointer to next block if current block doesnt have enough space
        curr = (void*)imp_get_next_header(curr_header);
    }
    // If we reach end of heap, then there is no space for the allocation in the heap
    return NULL;
}

void 
imp_free (void* ptr) 
{
    // Cant free NULL ptr, so print error message
    if (ptr == NULL) {
        return;
    }
    
    // Free the block
    tag* curr_header = imp_get_header(ptr);
    tag* curr_footer = imp_get_footer(curr_header);
    imp_set_free(curr_header);
    imp_set_free(curr_footer);
    
    // coalesce
    imp_right_coalesce(curr_header);
    imp_left_coalesce(curr_header);
}

void* 
imp_realloc (void* old_ptr, size_t new_payload_size) 
{
    tag* curr_header = imp_get_header(old_ptr);
    size_t old_payload_size = imp_get_payload_size(curr_header);
    size_t total_new_size = imp_round_up(new_payload_size + TWO_TAG_SIZE, ALIGN);
    new_payload_size = total_new_size - TWO_TAG_SIZE;

    // Shrink case where we can make new block inside realloced block
    if (new_payload_size + MIN_BLOCK_SIZE < old_payload_size) {

        // Set payload for curr_header
        imp_set_payload_size(curr_header, new_payload_size);

        // Create footer for current block and set payload and alloc status
        tag* curr_footer = imp_get_footer(curr_header);
        imp_set_payload_size(curr_footer, new_payload_size);
        imp_set_alloc(curr_footer);

        // Create header for new block and set payload and alloc status
        tag* next_header = imp_get_next_header(curr_header);
        size_t next_payload_size = old_payload_size - new_payload_size - TWO_TAG_SIZE;
        imp_set_payload_size(next_header, next_payload_size);
        imp_set_free(next_header);

        // Create footer for new block and set payload and alloc status
        tag* next_footer = imp_get_footer(next_header);
        imp_set_payload_size(next_footer, next_payload_size);
        imp_set_free(next_footer);

        return old_ptr;
    }

    // Shrink / Extend + merge with next block
    tag* next_header = imp_get_next_header(curr_header);
    size_t next_payload_size = imp_get_payload_size(next_header);

    if (imp_get_alloc(next_header) == IS_FREE) {
        // Shrink Case
        if (new_payload_size <= old_payload_size) {
            // Set payload and alloc status for header of current block
            imp_set_payload_size(curr_header, new_payload_size);

            // Set payload and alloc status for footer of current block
            tag* curr_footer = imp_get_footer(curr_header);
            imp_set_payload_size(curr_footer, new_payload_size);
            imp_set_alloc(curr_footer);

            // Set payload and alloc status for header of next block
            next_payload_size = next_payload_size + (old_payload_size - new_payload_size);
            tag* next_header = imp_get_next_header(curr_header);
            imp_set_payload_size(next_header, next_payload_size);
            imp_set_free(next_header);

            // Set payload and alloc status for footer of next block
            tag* next_footer = imp_get_footer(next_header);
            imp_set_payload_size(next_footer, next_payload_size);
            imp_set_free(next_footer);

            return old_ptr;
        }
        // Extend Case
        if (new_payload_size - old_payload_size <= next_payload_size - TWO_TAG_SIZE) {
            // Set payload and alloc status for header of current block
            imp_set_payload_size(curr_header, new_payload_size);

            // Set payload and alloc status for footer of current block
            tag* curr_footer = imp_get_footer(curr_header);
            imp_set_payload_size(curr_footer, new_payload_size);
            imp_set_alloc(curr_footer);

            // Set payload and alloc status for header of next block
            next_payload_size = next_payload_size - (new_payload_size - old_payload_size);
            tag* next_header = imp_get_next_header(curr_header);
            imp_set_payload_size(next_header, next_payload_size);
            imp_set_free(next_header);

            // Set payload and alloc status for footer of next block
            tag* next_footer = imp_get_footer(next_header);
            imp_set_payload_size(next_footer, next_payload_size);
            imp_set_free(next_footer);

            return old_ptr;
        }
    }

    // Malloc, memcpy, and free if we cant resize in place
    void* new_ptr = imp_malloc(new_payload_size);
    memcpy(new_ptr, old_ptr, old_payload_size);
    imp_free(old_ptr);
    return new_ptr;
}


void 
imp_heap_destroy (void) 
{
	if (heap_size > 0) {
		sbrk(-heap_size); // Deletes heap if it has been allocated previously
		heap_size = 0;
	}
}
