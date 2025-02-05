#ifndef PATHNAME_H
#define PATHNAME_H

#include <stdint.h>
#include "inode.h"
#include "file.h"
#include "directory.h"

// Define maximum length for a filename (typically 28 bytes in UNIX v6)
#define MAX_FILENAME_LENGTH 28

// Define maximum length for a complete pathname
#define MAX_PATHNAME_LENGTH 512

// Declare a structure for a pathname component (e.g., "dir1", "dir2", "file.txt")
struct pathname_component {
    char name[MAX_FILENAME_LENGTH];  // Component name (e.g., directory or file)
    struct pathname_component *next; // Pointer to next component in the path (if any)
};

// Declare the function to resolve a pathname to an inode
int pathname_resolve(const char *pathname, struct inode *inode);

// Declare the function to split a pathname into its components
int pathname_split(const char *pathname, struct pathname_component **components);

// Declare the function to free the list of pathname components
void pathname_free(struct pathname_component *components);

// Declare the function to look up a component in the current directory
int pathname_lookup(struct pathname_component *component, int current_dir_descriptor, struct inode *inode);

// Declare the function to handle relative pathnames (starting from current directory)
int pathname_resolve_relative(const char *pathname, struct inode *inode, int current_dir_descriptor);

// Declare the function to handle absolute pathnames (starting from root)
int pathname_resolve_absolute(const char *pathname, struct inode *inode);

#endif // PATHNAME_H

