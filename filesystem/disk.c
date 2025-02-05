#include "disk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 512

static FILE *disk_file = NULL;  // Static file pointer for the disk image
static struct superblock sb;    // Global superblock instance

/**
 * Open the disk image file.
 * @param filename The name of the disk image file.
 * @return 0 on success, -1 on failure.
 */
int disk_open(const char *filename) {
    disk_file = fopen(filename, "r+b"); // Open for reading and writing
    if (!disk_file) {
        perror("Failed to open disk image");
        return -1;
    }

    // Read the superblock immediately upon opening
    if (read_superblock() != 0) {
        printf("Error: Failed to read superblock.\n");
        fclose(disk_file);
        disk_file = NULL;
        return -1;
    }

    return 0;
}

/**
 * Close the disk image file.
 */
void disk_close() {
    if (disk_file) {
        fclose(disk_file);
        disk_file = NULL;
    }
}

/**
 * Read a block from the disk image.
 * @param block_num The block number to read.
 * @param buffer The buffer to store the read data.
 * @return 0 on success, -1 on failure.
 */
int disk_read(uint16_t block_num, void *buffer) {
    if (!disk_file) return -1;

    fseek(disk_file, block_num * BLOCK_SIZE, SEEK_SET);
    size_t bytes_read = fread(buffer, 1, BLOCK_SIZE, disk_file);

    return (bytes_read == BLOCK_SIZE) ? 0 : -1;
}

/**
 * Write a block to the disk image.
 * @param block_num The block number to write to.
 * @param buffer The buffer containing the data to write.
 * @return 0 on success, -1 on failure.
 */
int disk_write(uint16_t block_num, const void *buffer) {
    if (!disk_file) return -1;

    fseek(disk_file, block_num * BLOCK_SIZE, SEEK_SET);
    size_t bytes_written = fwrite(buffer, 1, BLOCK_SIZE, disk_file);
    fflush(disk_file); // Ensure data is written to disk

    return (bytes_written == BLOCK_SIZE) ? 0 : -1;
}

/**
 * Reads the superblock into memory.
 * @return 0 on success, -1 on failure.
 */
int read_superblock() {
    if (disk_read(1, &sb) != 0) {
        printf("Error: Could not read superblock\n");
        return -1;
    }

    printf("Successfully read the superblock.\n");
    printf("Filesystem size: %u blocks\n", sb.s_fsize);
    printf("Inode table size: %u blocks\n", sb.s_isize);

    return 0;
}

/**
 * Returns a pointer to the global superblock.
 * @return Pointer to the superblock.
 */
struct superblock *get_superblock() {
    return &sb;
}

