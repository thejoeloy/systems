#include "explicit.h"
#include <stdio.h>

int main() {
    exp_heap_init(256);
    void* p1 = exp_malloc(9);
    void* p2 = exp_malloc(7);
    p1 = exp_realloc(p1, 14);
    exp_free(p2);
    void* p3 = exp_malloc(49);
    p1 = exp_realloc(p1, 8);
    p3 = exp_realloc(p3, 17);
    p3 = exp_realloc(p3, 25);
    exp_free(p1);
    void* p4 = exp_malloc(67);
    exp_free(p3);
    exp_free(p4);
    iterate_heap();
    exp_heap_destroy();
    return 0;
}
