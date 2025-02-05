#include "inode.h"
#include "disk.h"
#include <stdint.h>
#include <stdio.h>

/**
 * Reads an inode from disk given its inode number.
 * @param inumber The inode number.
 * @param inode A pointer to an inode structure where the data will be stored.
 * @return 0 on success, -1 on failure.
 */
int inode_read(uint16_t inumber, struct inode *inode) {
    // Step 1: Get the block containing the inode
    int inode_block = inode_lookup(inumber);
    if (inode_block == -1) {
        printf("Error: Inode block lookup failed\n");
        return -1;
    }

    // Step 2: Read the block containing the inode
    char buffer[BLOCK_SIZE];
    if (disk_read(inode_block, buffer) != 0) {
        printf("Error reading block %d\n", inode_block);
        return -1;
    }

    // Step 3: Calculate the position of the inode within the block
    int inode_offset = (inumber - 1) * INODE_SIZE;
    int inode_position = inode_offset % BLOCK_SIZE; // This is the position of the inode within the block

    // Step 4: Copy the inode from the block into the inode structure
    memcpy(inode, &buffer[inode_position], INODE_SIZE);

    // Step 5: Return success
    return 0;
}

/**
 * Writes an inode to disk given its inode number.
 * @param inumber The inode number.
 * @param inode A pointer to an inode structure containing the data to be written.
 * @return 0 on success, -1 on failure.
 */
int inode_write(uint16_t inumber, const struct inode *inode) {
    // Step 1: Get the block containing the inode
    int inode_block = inode_lookup(inumber);
    if (inode_block == -1) {
        printf("Error: Inode block lookup failed\n");
        return -1;
    }

    // Step 2: Read the block containing the inode
    char buffer[BLOCK_SIZE];
    if (disk_read(inode_block, buffer) != 0) {
        printf("Error reading block %d\n", inode_block);
        return -1;
    }

    // Step 3: Calculate the position of the inode within the block
    int inode_offset = (inumber - 1) * INODE_SIZE;
    int inode_position = inode_offset % BLOCK_SIZE; // This is the position of the inode within the block

    // Step 4: Copy the inode from the provided structure into the block
    memcpy(&buffer[inode_position], inode, INODE_SIZE);

    // Step 5: Write the block back to disk
    if (disk_write(inode_block, buffer) != 0) {
        printf("Error writing block %d\n", inode_block);
        return -1;
    }

    // Step 6: Return success
    return 0;
}

/**
 * Looks up an inode by its inode number and returns its block number.
 * @param inumber The inode number.
 * @return The block number containing the inode, or -1 on failure.
 */
int inode_lookup(uint16_t inumber) {
    int inode_offset = (inumber - 1) * INODE_SIZE;
    int block_offset = inode_offset / BLOCK_SIZE;
    int inode_block = INODE_TABLE_START + block_offset;
    return inode_block;
}

/**
 * Gets the block number of the nth data block in the inode's data.
 * @param inode A pointer to the inode structure.
 * @param block_index The index (n) of the block within the file.
 * @return The block number if found, or -1 if out of bounds.
 */
int inode_get_block(const struct inode *inode, uint16_t block_index) {
    // Case 1: The requested block is within the first 8 direct blocks.
    if (block_index < 8) {
        if (inode->direct[block_index] != 0) {
            return inode->direct[block_index];
        } else {
            return -1; // The block is not allocated.
        }
    }

    // Case 2: The requested block is in the indirect block.
    if (block_index >= 8) {
        uint16_t indirect_block = inode->indirect;
        if (indirect_block == 0) {
            return -1; // No indirect block, thus no more data blocks.
        }

        // Read the indirect block from the disk
        uint16_t indirect_block_data[BLOCK_SIZE / sizeof(uint16_t)];
        if (disk_read(indirect_block, indirect_block_data) != 0) {
            return -1; // Failed to read the indirect block.
        }

        // Calculate the index within the indirect block
        uint16_t indirect_idx = block_index - 8;

        // Return the block number from the indirect block
        if (indirect_idx < (BLOCK_SIZE / sizeof(uint16_t))) {
            return indirect_block_data[indirect_idx];
        } else {
            return -1; // Out of bounds in the indirect block.
        }
    }

    return -1; // In case of any other issues.
}
