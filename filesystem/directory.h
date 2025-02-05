#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <stdint.h>
#include "inode.h"
#include "file.h"

// Define maximum directory entry size (for simplicity, let's assume 32 bytes per entry)
#define DIR_ENTRY_SIZE 32

// Define the maximum number of directory entries per block
#define DIR_ENTRIES_PER_BLOCK (BLOCK_SIZE / DIR_ENTRY_SIZE)

// Define structure for directory entries
struct dir_entry {
    uint16_t inode_num;      // The inode number this entry points to
    char filename[28];       // The filename (max length of 28 bytes for simplicity)
};

// Define the structure for a directory
struct directory {
    struct dir_entry entries[DIR_ENTRIES_PER_BLOCK]; // Directory entries stored in blocks
};

// Declare the function to create a new directory
int directory_create(const char *dirname);

// Declare the function to open a directory (returns directory descriptor or inode number)
int directory_open(const char *dirname);

// Declare the function to close an open directory
int directory_close(int dir_descriptor);

// Declare the function to read a directory (returns directory entry)
int directory_read(int dir_descriptor, struct dir_entry *entry);

// Declare the function to write a directory entry to a directory
int directory_write(int dir_descriptor, const struct dir_entry *entry);

// Declare the function to delete a directory entry (file)
int directory_delete(int dir_descriptor, const char *filename);

// Declare the function to check if a directory exists
int directory_exists(const char *dirname);

// Declare the function to lookup a filename in a directory (returns inode number)
int directory_lookup(int dir_descriptor, const char *filename);

// Declare the function to get the inode associated with a directory entry
int directory_get_inode(int dir_descriptor, const struct dir_entry *entry, struct inode *inode);

#endif // DIRECTORY_H

