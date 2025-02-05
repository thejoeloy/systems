#ifndef DISK_H
#define DISK_H

#include <stdint.h>

#define BLOCK_SIZE 512

/**
 * Structure representing the UNIX v6 superblock.
 */
struct superblock {
    uint16_t s_isize;   // Number of blocks allocated for inodes
    uint16_t s_fsize;   // Total number of blocks in the filesystem
    uint16_t s_nfree;   // Number of free blocks
    uint16_t s_free[100]; // Free block list
    uint16_t s_ninode;  // Number of free inodes
    uint16_t s_inode[100]; // Free inode list
    char s_flock;
    char s_ilock;
    char s_fmod;
    uint16_t s_time[2];
};

/**
 * Opens the disk image file.
 * @param filename The path to the disk image.
 * @return 0 on success, -1 on failure.
 */
int disk_open(const char *filename);

/**
 * Closes the disk image file.
 */
void disk_close();

/**
 * Reads a block from the disk image.
 * @param block_num The block number to read.
 * @param buffer The buffer to store the block data.
 * @return 0 on success, -1 on failure.
 */
int disk_read(uint16_t block_num, void *buffer);

/**
 * Writes a block to the disk image.
 * @param block_num The block number to write to.
 * @param buffer The buffer containing the block data.
 * @return 0 on success, -1 on failure.
 */
int disk_write(uint16_t block_num, const void *buffer);

/**
 * Reads the superblock into memory.
 * @return 0 on success, -1 on failure.
 */
int read_superblock();

/**
 * Returns a pointer to the global superblock.
 * @return Pointer to the superblock.
 */
struct superblock *get_superblock();

#endif // DISK_H

