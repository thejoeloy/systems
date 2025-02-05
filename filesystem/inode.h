#ifndef INODE_H
#define INODE_H

#include <stdint.h>

#define INODE_SIZE 32  // UNIX v6 inode size in bytes
#define INODE_TABLE_START 2
/**
 * Structure representing an inode in UNIX v6.
 */
struct inode {
    uint16_t mode;        // File type and permissions
    uint16_t nlinks;      // Number of links to this file
    uint16_t uid;         // User ID of owner
    uint16_t gid;         // Group ID (if applicable)
    uint32_t size;        // File size in bytes
    uint16_t direct[8];   // Direct data block pointers
    uint16_t indirect;    // Indirect block pointer
};

/**
 * Reads an inode from disk.
 */
int inode_read(uint16_t inumber, struct inode *inode);

/**
 * Writes an inode to disk.
 */
int inode_write(uint16_t inumber, const struct inode *inode);

/**
 * Looks up an inode by its inode number.
 */
int inode_lookup(uint16_t inumber);

/**
 * Gets the block number of the nth block in an inode's data.
 */
int inode_get_block(const struct inode *inode, uint16_t block_index);

#endif // INODE_H

