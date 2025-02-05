#include "pathname.h"
#include "disk.h"
#include "inode.h"
#include "directory.h"
#include <stdio.h>
#include <string.h>

// Implement the function to resolve a pathname to an inode
int pathname_resolve(const char *pathname, struct inode *inode) {
    // Function implementation goes here
    return 0;
}

// Implement the function to split a pathname into its components
// This will break a pathname like "/dir1/dir2/file.txt" into "dir1", "dir2", "file.txt"
int pathname_split(const char *pathname, struct pathname_component **components) {
    // Function implementation goes here
    return 0;
}

// Implement the function to free the list of pathname components
void pathname_free(struct pathname_component *components) {
    // Function implementation goes here
}

// Implement the function to look up a component (like "dir1" or "file.txt") in the current directory
int pathname_lookup(struct pathname_component *component, int current_dir_descriptor, struct inode *inode) {
    // Function implementation goes here
    return 0;
}

// Implement the function to handle relative pathnames (starting from the current directory)
int pathname_resolve_relative(const char *pathname, struct inode *inode, int current_dir_descriptor) {
    // Function implementation goes here
    return 0;
}

// Implement the function to handle absolute pathnames (starting from the root)
int pathname_resolve_absolute(const char *pathname, struct inode *inode) {
    // Function implementation goes here
    return 0;
}

