#include "disk.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    if (disk_open("v6root") < 0) {
        return EXIT_FAILURE;
    }

    // Read the superblock into global memory
    struct superblock *sb = get_superblock();
    if (sb) {
        printf("Successfully read the superblock.\n");
        printf("Filesystem size: %u blocks\n", sb->s_fsize);
        printf("Inode table size: %u blocks\n", sb->s_isize);
    } else {
        printf("Failed to retrieve superblock.\n");
    }

    disk_close();
    return EXIT_SUCCESS;
}

