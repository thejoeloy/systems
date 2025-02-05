#include "directory.h"
#include "disk.h"
#include "inode.h"
#include <stdio.h>
#include <string.h>

// Implement the function to create a new directory
// This will allocate an inode for the directory and initialize its metadata.
int directory_create(const char *dirname) {
    // Function implementation goes here
    return 0;
}

// Implement the function to open an existing directory
// This will look up the directory by its name, load the inode, and return the directory descriptor.
int directory_open(const char *dirname) {
    // Function implementation goes here
    return 0;
}

// Implement the function to close an open directory
// This will free any resources associated with the open directory and save any updates to the inode.
int directory_close(int dir_descriptor) {
    // Function implementation goes here
    return 0;
}

// Implement the function to read a directory
// This will read the next directory entry from the open directory.
int directory_read(int dir_descriptor, struct dir_entry *entry) {
    // Function implementation goes here
    return 0;
}

// Implement the function to write a directory entry to a directory
// This will write the given directory entry to the disk.
int directory_write(int dir_descriptor, const struct dir_entry *entry) {
    // Function implementation goes here
    return 0;
}

// Implement the function to delete a directory entry (file)
// This will remove the entry from the directory and free the associated inode.
int directory_delete(int dir_descriptor, const char *filename) {
    // Function implementation goes here
    return 0;
}

// Implement the function to check if a directory exists
// This will check if the given directory name exists and return 1 if it does, 0 if not.
int directory_exists(const char *dirname) {
    // Function implementation goes here
    return 0;
}

// Implement the function to look up a filename in a directory
// This will search for the filename in the directory and return the corresponding inode number if found.
int directory_lookup(int dir_descriptor, const char *filename) {
    // Function implementation goes here
    return -1; // Return -1 if not found
}

// Implement the function to get the inode associated with a directory entry
// This will read the inode of a directory entry into the provided inode structure.
int directory_get_inode(int dir_descriptor, const struct dir_entry *entry, struct inode *inode) {
    // Function implementation goes here
    return 0;
}

