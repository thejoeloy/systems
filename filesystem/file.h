#ifndef FILE_H
#define FILE_H

#include <stdint.h>
#include "inode.h"

// Declare the function that creates a new file
int file_create(const char *filename);

// Declare the function that opens a file (returns the inode number or file descriptor)
int file_open(const char *filename);

// Declare the function that closes an open file (frees any resources, writes inode to disk)
int file_close(int file_descriptor);

// Declare the function to read data from a file
int file_read(int file_descriptor, void *buffer, size_t size);

// Declare the function to write data to a file
int file_write(int file_descriptor, const void *buffer, size_t size);

// Declare the function to seek to a specific position in a file (for random access)
int file_seek(int file_descriptor, size_t offset);

// Declare the function to delete a file
int file_delete(const char *filename);

// Declare the function to check if a file exists
int file_exists(const char *filename);

// Declare the function to update the inode of an open file (e.g., when modifying file size)
int file_update_inode(int file_descriptor, const struct inode *inode);

#endif // FILE_H

