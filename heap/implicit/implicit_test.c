// This file is for testing out my implicit heap allocator
#include "implicit.h"
#include <stdio.h>

int main() {
    ImpHeapInit(256);
    void* p1 = ImpMalloc(9);
    void* p2 = ImpMalloc(7);
    p1 = ImpRealloc(p1, 14);
    ImpFree(p2);
    void* p3 = ImpMalloc(49);
    p1 = ImpRealloc(p1, 8);
    p3 = ImpRealloc(p3, 17);
    p3 = ImpRealloc(p3, 25);
    ImpFree(p1);
    void* p4 = ImpMalloc(67);
    ImpFree(p3);
    ImpFree(p4);
    IterateHeap();
    ImpHeapDestroy();
    return 0;
}
