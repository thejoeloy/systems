#include "explicit.h"
#include <stdio.h>

int main() {
    ExpHeapInit(256);
    void* p1 = ExpMalloc(9);
    void* p2 = ExpMalloc(7);
    p1 = ExpRealloc(p1, 14);
    ExpFree(p2);
    void* p3 = ExpMalloc(49);
    p1 = ExpRealloc(p1, 8);
    p3 = ExpRealloc(p3, 17);
    p3 = ExpRealloc(p3, 25);
    ExpFree(p1);
    void* p4 = ExpMalloc(67);
    ExpFree(p3);
    ExpFree(p4);
    IterateHeap();
    ExpHeapDestroy();
    return 0;
}
