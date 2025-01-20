// This file is for testing out my implicit heap allocator
#include "explicit.h"
#include <stdio.h>

int main() {
    ExpHeapInit(256);
    IterateHeap();
    void* p1 = ExpMalloc(40);
    void* p2 = ExpMalloc(40);
    void* p3 = ExpMalloc(16);
    IterateHeap();
    p1 = ExpRealloc(p1, 16);
    ExpFree(p2);
    void* p4 = ExpMalloc(24);
    p4 = ExpRealloc(p4, 40);
    IterateHeap();
    ExpFree(p4);
    ExpFree(p3);
    ExpFree(p1);
    IterateHeap();
    ExpHeapDestroy();
    return 0;
}
