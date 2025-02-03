#include "implicit.h"
#include <stdio.h>

int main() {
    imp_heap_init(256);
    void* p1 = imp_malloc(9);
    void* p2 = imp_malloc(7);
    p1 = imp_realloc(p1, 14);
    imp_free(p2);
    void* p3 = imp_malloc(49);
    p1 = imp_realloc(p1, 8);
    p3 = imp_realloc(p3, 17);
    p3 = imp_realloc(p3, 25);
    imp_free(p1);
    void* p4 = imp_malloc(67);
    imp_free(p3);
    imp_free(p4);
    iterate_heap();
    imp_heap_destroy();
    return 0;
}
