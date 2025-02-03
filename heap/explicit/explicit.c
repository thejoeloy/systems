#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "explicit.h"

void* sbrk(intptr_t increment);

static void* base;
static void* heap_list_ptr;
static void* max_heap_addr;
static size_t heap_size;
static tag* free_list_start;

// tag helpers
tag* 
exp_get_header (void* ptr) 
{
	return (tag*)((char*)ptr - TAG_SIZE);
}

tag* 
exp_get_footer (tag* curr_header) 
{
	return (tag*)((char*)curr_header + exp_get_payload_size(curr_header) + TAG_SIZE);
}

tag* 
exp_get_next_header (tag* curr_header) 
{
	size_t payload_size = exp_get_payload_size(curr_header);
	tag* next_header = (tag*)((char*)curr_header + (payload_size + TWO_TAG_SIZE));
	return next_header;	
}

tag* 
exp_get_prev_footer (tag* curr_header) 
{
	return (tag*)((char*)curr_header - TAG_SIZE);	
}

tag* 
exp_get_prev_header (tag* prev_footer) 
{
	return (tag*)((char*)prev_footer - exp_get_payload_size(prev_footer) - TAG_SIZE);
}

size_t 
exp_get_payload_size (tag* t) 
{
	return (size_t)((t->tag & PAYLOAD_SIZE_MASK) >> 3);
}

size_t 
exp_get_alloc (tag* t) 
{
	return t->tag & IS_ALLOC;
}

void 
exp_set_free (tag* t) 
{
    t->tag &= PAYLOAD_SIZE_MASK;
}

void 
exp_set_alloc (tag* t) 
{
    t->tag |= IS_ALLOC;
}

void 
exp_set_payload_size (tag* t, size_t size) 
{
	size <<= 3;
	t->tag = (t->tag & IS_ALLOC) | size;
}

// free_list_ptr helpers
free_list_ptr* 
exp_get_flp (tag* curr_header) 
{
    return (free_list_ptr*)((char*)curr_header + TAG_SIZE);
}

free_list_ptr* 
exp_get_next_flp (free_list_ptr* curr_flp) 
{
    return exp_get_flp(curr_flp->succ);
}

free_list_ptr* 
exp_get_prev_flp (free_list_ptr* curr_flp) 
{
    return exp_get_flp(curr_flp->pred);
}

tag* 
exp_get_pred (free_list_ptr* flp) 
{
    return flp->pred;
}

tag* 
exp_get_succ (free_list_ptr* flp) 
{
    return flp->succ;
}

void 
exp_set_pred (free_list_ptr* flp, tag* pred) 
{
    flp->pred = pred;
}

void 
exp_set_succ (free_list_ptr* flp, tag* succ) 
{
    flp->succ = succ;
}

// Heap function helpers
size_t 
exp_round_up (size_t size, size_t mult) 
{
	return (size + mult-1) & ~(mult-1);
}

void 
exp_insert_free_block (tag* curr_header) 
{
    free_list_ptr* new_flp = exp_get_flp(curr_header);
    tag* iter_header = free_list_start;
    free_list_ptr* iter_flp = exp_get_flp(iter_header);
    
    // In this case we insert to front of list
    if (iter_header > curr_header) {
        exp_set_pred(new_flp, iter_flp->pred);
        exp_set_succ(new_flp, iter_header);
        exp_set_pred(iter_flp, curr_header);
        free_list_start = curr_header;
    }

    // Otherwise we traverse the list
    while(iter_header != NULL) {
        // Inserting to somewhere in the middle of the free list
        if (curr_header > iter_header && curr_header < iter_flp->succ && iter_flp->succ != NULL) {
            free_list_ptr* iter_next_flp = exp_get_next_flp(iter_flp);
            tag* iter_next_header = exp_get_header(iter_next_flp);
            exp_set_pred(iter_next_flp, curr_header);

            exp_set_pred(new_flp, iter_header);
            exp_set_succ(new_flp, iter_next_header);

            exp_set_succ(iter_flp, curr_header);
            break;
        }
        // Inserting to the end of the free list
        if (curr_header > iter_header && iter_flp->succ == NULL) {
            exp_set_pred(new_flp, iter_header);
            exp_set_succ(new_flp, iter_flp->succ);
            
            exp_set_succ(iter_flp, curr_header);
            break;
        }

        // Increment the free list pointer iterators
        iter_header = iter_flp->succ;
        iter_flp = exp_get_flp(iter_header);

    }
}

void 
exp_right_coalesce (tag* curr_header) 
{
    tag* next_header = exp_get_next_header(curr_header);

    // Check if next block is free and coalesce if it is
    if (exp_get_alloc(next_header) == IS_FREE) {
        // Set header of base to new payload size
        size_t curr_payload_size = exp_get_payload_size(curr_header);
        size_t next_payload_size = exp_get_payload_size(next_header);
        size_t new_payload_size = next_payload_size + curr_payload_size + TWO_TAG_SIZE;
        exp_set_payload_size(curr_header, new_payload_size);
            
        // Create new Footer and set payload size and alloc status
        tag* new_footer = (tag*)((char*)curr_header + new_payload_size + TAG_SIZE);
        exp_set_payload_size(new_footer, new_payload_size);
        exp_set_free(new_footer);

        // Update free list pointers
        free_list_ptr* curr_flp = exp_get_flp(curr_header);
        free_list_ptr* next_flp = exp_get_flp(next_header);
        free_list_ptr* next_pred_flp = exp_get_prev_flp(next_flp);
        free_list_ptr* next_succ_flp = exp_get_next_flp(next_flp);
        
        // If predecessor to next_flp doesnt exist, curr_header is new start to free list
        if (next_flp->pred == NULL || next_flp->pred == curr_header) {
            free_list_start = curr_header;
            exp_set_pred(curr_flp, NULL);
        }
        // If predecessor to next_flp does exist, make it curr_headers predecessor and curr_header should be its new successor
        else {
            exp_set_pred(curr_flp, next_flp->pred);
            exp_set_succ(next_pred_flp, curr_header);
        }
        // If successor to next_flp exists, make curr_header its predecessor and set next_flp as its successor
        if (next_flp->succ != NULL) {
            exp_set_succ(curr_flp, next_flp->succ);
            exp_set_pred(next_succ_flp, curr_header);
        }
        else {
            exp_set_succ(curr_flp, next_flp->succ);
        }
    }
}

void 
exp_left_coalesce (tag* curr_header) 
{
    tag* prev_footer = exp_get_prev_footer(curr_header);

    // Check if previous block is free and coalesce if it is
    if (exp_get_alloc(prev_footer) == IS_FREE) {
        // Set header of previous block to new payload size
        tag* prev_header = exp_get_prev_header(prev_footer);
        size_t prev_payload_size = exp_get_payload_size(prev_footer);
        size_t curr_payload_size = exp_get_payload_size(curr_header);
        size_t new_payload_size = curr_payload_size + prev_payload_size + TWO_TAG_SIZE;
        exp_set_payload_size(prev_header, new_payload_size);

        // Set footer of base to new payload size
        tag* new_footer = (tag*)((char*)curr_header + curr_payload_size + TAG_SIZE);
        exp_set_payload_size(new_footer, new_payload_size);
        exp_set_free(new_footer);

        // Update free list pointers
        free_list_ptr* curr_flp = exp_get_flp(curr_header);
        free_list_ptr* prev_flp = exp_get_flp(prev_header);
        free_list_ptr* curr_succ_flp = exp_get_next_flp(curr_flp);

        // Set predecessor of successor to curr_header to prev_header if it exists 
        if (curr_flp->succ != NULL) {
            exp_set_pred(curr_succ_flp, prev_header);
        }

        // Set successor to new FLP
        exp_set_succ(prev_flp, curr_flp->succ);

        // Update free_list_start to prev_header if curr_header was previously start of free list
        if (curr_flp->pred == NULL) {
            free_list_start = prev_header;
        }

    }
}

// Heap debugging
void 
print_header (tag* curr_header) 
{
	if (curr_header == NULL) return;
	size_t last3bits = exp_get_alloc(curr_header);
    printf("Address : %p, payload_size : %zu, Last 3 bits : %zu\n", (void*)curr_header, exp_get_payload_size(curr_header), last3bits);
    if (last3bits == IS_FREE) {
        free_list_ptr* flp = exp_get_flp(curr_header);
        printf("pred_addr : %p, pred : %p\n", (void*)((char*)curr_header + PRED_OFFSET), (void*)flp->pred);
        printf("succ_addr : %p, succ : %p\n", (void*)((char*)curr_header + SUCC_OFFSET), (void*)flp->succ);
    }
}

void 
print_footer (tag* curr_footer) 
{
    if (curr_footer == NULL) return;
    size_t last3bits = exp_get_alloc(curr_footer);
    printf("Address : %p, payload_size : %zu, Last 3 bits : %zu\n", (void*)curr_footer, exp_get_payload_size(curr_footer), last3bits);
}

void 
iterate_heap () 
{
    printf("iterate_heap\n");
    printf("free_list_start: %p\n", free_list_start);
    void* curr = base;
    print_footer(base);
    curr = (tag*)((char*)base + TAG_SIZE);
    print_footer(curr);
    curr = heap_list_ptr;

    while(curr < max_heap_addr - TAG_SIZE) {
        tag* curr_header = (tag*)curr;
        size_t payload_size = exp_get_payload_size(curr_header);
        tag* curr_footer = (tag*)((char*)curr_header + TAG_SIZE + payload_size);
        print_header(curr_header);
        print_footer(curr_footer);
        curr = (void*)((char*)curr_footer + TAG_SIZE);
    }

    print_footer((tag*)((char*)max_heap_addr - TAG_SIZE));
}

// Heap Functions
int 
exp_heap_init (size_t size) 
{
    // Ensure that requested size is non-zero
    if (size == 0) {
        return 0;
    }

    // Allocate space for heap and ensure that enough space exists
    size_t adjusted_size = exp_round_up(size, ALIGN);
    
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
	exp_set_payload_size(prologue_header, TWO_TAG_SIZE);
	exp_set_payload_size(prologue_footer, TWO_TAG_SIZE);
	exp_set_alloc(prologue_header);
	exp_set_alloc(prologue_footer);
	exp_set_payload_size(epilogue, 0);
	exp_set_alloc(epilogue);
	
    // Set up initial header
    tag* initial_header = (tag*)heap_list_ptr;
    size_t payload_size = adjusted_size - TWO_TAG_SIZE - SPECIAL_TAG_SIZES;
    exp_set_payload_size(initial_header, payload_size);
    exp_set_free(initial_header);

    // Set up initial free list
    free_list_ptr* flp = exp_get_flp(initial_header);
    tag* initPtr = NULL;
    exp_set_pred(flp, initPtr);
    exp_set_succ(flp, initPtr);
    free_list_start = initial_header;

    // Set up footer tag
    tag* initial_footer = exp_get_footer(initial_header);
    exp_set_payload_size(initial_footer, payload_size);
    exp_set_free(initial_footer);
	
    return 1;
}

void* 
exp_malloc (size_t req_size) 
{
    // Ensure that requested size is non zero
    if (req_size == 0) {
        return NULL;
    }

    // Allocate space for new block and ensure that it is aligned and atleast MIN_BLOCK_SIZE
    size_t total_size = exp_round_up(req_size + TWO_TAG_SIZE, ALIGN);
    total_size = (total_size >= MIN_BLOCK_SIZE) ? total_size : MIN_BLOCK_SIZE;

    tag* iter_header = free_list_start;

    // Iterate through free list until we find a space to insert new block
    while (iter_header != NULL) {
        free_list_ptr* iter_flp = exp_get_flp(iter_header);
        size_t payload_size = exp_get_payload_size(iter_header);

        if (payload_size >= total_size) {
            // Case where we can alloc requested block and create new free block with remaining space
            if (payload_size >= total_size + MIN_BLOCK_SIZE) {
                // Set payload and alloc status for current header
                exp_set_payload_size(iter_header, total_size - TWO_TAG_SIZE);
                exp_set_alloc(iter_header);

                // Set payload and alloc status for current footer
                tag* iter_footer = exp_get_footer(iter_header);
                exp_set_payload_size(iter_footer, total_size - TWO_TAG_SIZE);
                exp_set_alloc(iter_footer);

                tag* next_header = exp_get_next_header(iter_header);
                exp_set_payload_size(next_header, payload_size - total_size);
                exp_set_free(next_header);

                tag* next_footer = exp_get_footer(next_header);
                exp_set_payload_size(next_footer, payload_size - total_size);
                exp_set_free(next_footer);

                // Update Free List Pointers
                free_list_ptr* next_flp = exp_get_flp(next_header);
                // Case where we are at beginning of list
                if (iter_flp->pred == NULL) {
                    free_list_start = next_header;
                    exp_set_pred(next_flp, iter_flp->pred);
                    // Modify successor if it exists
                    if (iter_flp->succ != NULL) {
                        free_list_ptr* succ_flp = exp_get_flp(iter_flp->succ);
                        tag* succ_header = exp_get_header(succ_flp);
                        exp_set_pred(succ_flp, next_header);
                        exp_set_succ(next_flp, succ_header);
                    }
                }
                
                // Case where we are at end of list
                if (iter_flp->succ == NULL) {
                    exp_set_succ(next_flp, iter_flp->succ);
                    // Modify predecessor if it exists
                    if (iter_flp->pred != NULL) {
                        free_list_ptr* pred_flp = exp_get_flp(iter_flp->pred);
                        tag* pred_header = exp_get_header(pred_flp);
                        exp_set_succ(pred_flp, next_header);
                        exp_set_pred(next_flp, pred_header);
                    }
                }
                
                // Case where we are at middle of list
                if (iter_flp->pred != NULL && iter_flp->succ != NULL) {
                    free_list_ptr* pred_flp = exp_get_flp(iter_flp->pred);
                    tag* pred_header = exp_get_header(pred_flp);
                    free_list_ptr* succ_flp = exp_get_flp(iter_flp->succ);
                    tag* succ_header = exp_get_header(succ_flp);

                    exp_set_succ(pred_flp, next_header);
                    exp_set_pred(next_flp, pred_header);

                    exp_set_pred(succ_flp, next_header);
                    exp_set_succ(next_flp, succ_header);
                }
            }
            // Case where we cant create new free block, so just use all of the current block
            else {
                // Set payload and alloc status for current header
                exp_set_payload_size(iter_header, payload_size);
                exp_set_alloc(iter_header);

                // Set payload and alloc status for current footer
                tag* iter_footer = exp_get_footer(iter_header);
                exp_set_payload_size(iter_footer, payload_size);
                exp_set_alloc(iter_footer);

                // Update Free List Pointers
                // Case where we are at beginning of list
                if (iter_flp->pred == NULL) {
                    free_list_start = iter_flp->succ;
                }
                // Case where we are at end of list
                if (iter_flp->succ == NULL) {
                    free_list_ptr* pred_flp = exp_get_flp(iter_flp->pred);
                    exp_set_succ(pred_flp, iter_flp->succ);
                }

                // Case where we are at middle of list
                if (iter_flp->pred != NULL && iter_flp->succ != NULL) {
                    free_list_ptr* pred_flp = exp_get_prev_flp(iter_flp);
                    tag* pred_header = exp_get_header(pred_flp);
                    free_list_ptr* succ_flp = exp_get_next_flp(iter_flp);
                    tag* succ_header = exp_get_header(succ_flp);

                    exp_set_succ(pred_flp, succ_header);
                    exp_set_pred(succ_flp, pred_header);
                }
            }
            return (char*)iter_header + TAG_SIZE;
        }
        iter_header = iter_flp->succ;
    }
    // If we reach end of list, then there is not a large enough block of free space for the allocation
    return NULL;
}

void 
exp_free (void* ptr) 
{
    // Cant free NULL ptr, so print error message
    if (ptr == NULL) {
        return;
    }
    // Free the block
    tag* curr_header = exp_get_header(ptr);
    tag* curr_footer = exp_get_footer(curr_header);
    exp_set_free(curr_header);
    exp_set_free(curr_footer);
    exp_insert_free_block(curr_header);
    
    // coalesce
    exp_right_coalesce(curr_header);
    exp_left_coalesce(curr_header);
}

void* 
exp_realloc (void* old_ptr, size_t new_payload_size) 
{
    tag* curr_header = exp_get_header(old_ptr);
    size_t old_payload_size = exp_get_payload_size(curr_header);
    size_t total_new_size = exp_round_up(new_payload_size + TWO_TAG_SIZE, ALIGN);
    total_new_size = (total_new_size >= MIN_BLOCK_SIZE) ? total_new_size : MIN_BLOCK_SIZE;
    new_payload_size = total_new_size - TWO_TAG_SIZE;

    // Shrink case where we can make new block inside realloced block
    if (new_payload_size + MIN_BLOCK_SIZE <= old_payload_size) {
        // Set payload for curr_header
        exp_set_payload_size(curr_header, new_payload_size);

        // Create footer for current block and set payload and alloc status
        tag* curr_footer = exp_get_footer(curr_header);
        exp_set_payload_size(curr_footer, new_payload_size);
        exp_set_alloc(curr_footer);

        // Create header for new block and set payload and alloc status
        tag* next_header = exp_get_next_header(curr_header);
        size_t next_payload_size = old_payload_size - new_payload_size - TWO_TAG_SIZE;
        exp_set_payload_size(next_header, next_payload_size);
        exp_set_free(next_header);

        // Create footer for new block and set payload and alloc status
        tag* next_footer = exp_get_footer(next_header);
        exp_set_payload_size(next_footer, next_payload_size);
        exp_set_free(next_footer);

        // Update Free List Pointers
        exp_insert_free_block(next_header);

        return old_ptr;
    }

    // Shrink / Extend + merge with next block
    tag* next_header = exp_get_next_header(curr_header);
    size_t next_payload_size = exp_get_payload_size(next_header);

    if (exp_get_alloc(next_header) == IS_FREE) {
        // Shrink Case
        free_list_ptr* next_flp = exp_get_flp(next_header);
        free_list_ptr* next_pred_flp = exp_get_prev_flp(next_flp);
        free_list_ptr* next_succ_flp = exp_get_next_flp(next_flp);
        tag* pred_free_block = exp_get_pred(next_flp);
        tag* succ_free_block = exp_get_succ(next_flp);
        if (new_payload_size <= old_payload_size) {
            // Set payload and alloc status for header of current block
            exp_set_payload_size(curr_header, new_payload_size);

            // Set payload and alloc status for footer of current block
            tag* curr_footer = exp_get_footer(curr_header);
            exp_set_payload_size(curr_footer, new_payload_size);
            exp_set_alloc(curr_footer);

            // Set payload and alloc status for header of next block
            next_payload_size = next_payload_size + (old_payload_size - new_payload_size);
            tag* next_header = exp_get_next_header(curr_header);
            exp_set_payload_size(next_header, next_payload_size);
            exp_set_free(next_header);

            // Set payload and alloc status for footer of next block
            tag* next_footer = exp_get_footer(next_header);
            exp_set_payload_size(next_footer, next_payload_size);
            exp_set_free(next_footer);

            // Update Free List Pointers
            next_flp = exp_get_flp(next_header);
            exp_set_pred(next_flp, pred_free_block);
            exp_set_succ(next_flp, succ_free_block);

            // Update predecessor block successor and successor block predecessor
            // Update predecessor block successor if it exists or make next_header beginning of free list
            if (pred_free_block == NULL) {
                free_list_start = next_header;
            }
            else {
                exp_set_succ(next_pred_flp, next_header);
            }
            // Update successor block predecessor if it exists
            if (succ_free_block != NULL) {
                exp_set_pred(next_succ_flp, next_header);    
            }

            return old_ptr;
        }
        // Extend Case
        if (new_payload_size - old_payload_size <= next_payload_size - TWO_TAG_SIZE - FREE_LIST_POINTERS_SIZE) {
            // Set payload and alloc status for header of current block
            exp_set_payload_size(curr_header, new_payload_size);

            // Set payload and alloc status for footer of current block
            tag* curr_footer = exp_get_footer(curr_header);
            exp_set_payload_size(curr_footer, new_payload_size);
            exp_set_alloc(curr_footer);

            // Set payload and alloc status for header of next block
            next_payload_size = next_payload_size - (new_payload_size - old_payload_size);
            tag* next_header = exp_get_next_header(curr_header);
            exp_set_payload_size(next_header, next_payload_size);
            exp_set_free(next_header);

            // Set payload and alloc status for footer of next block
            tag* next_footer = exp_get_footer(next_header);
            exp_set_payload_size(next_footer, next_payload_size);
            exp_set_free(next_footer);

            // Update free list pointers
            next_flp = exp_get_flp(next_header);
            exp_set_pred(next_flp, pred_free_block);
            exp_set_succ(next_flp, succ_free_block);

            // Update predecessor block successor and successor block predecessor
            // Update predecessor block successor if it exists or make next_header beginning of free list
            if (pred_free_block == NULL) {
                free_list_start = next_header;
            }
            else {
                exp_set_succ(next_pred_flp, next_header);
            }
            if (succ_free_block != NULL) {
                exp_set_pred(next_succ_flp, next_header);
            }
            return old_ptr;
        }
    }
    // Malloc, memcpy, and free if we cant resize in place
    void* new_ptr = exp_malloc(new_payload_size);
    memcpy(new_ptr, old_ptr, old_payload_size);
    exp_free(old_ptr);
    return new_ptr;
}

void 
exp_heap_destroy (void) 
{
	if (heap_size > 0) {
		sbrk(-heap_size); // Deletes heap if it has been allocated previously
		heap_size = 0;
	}
}
