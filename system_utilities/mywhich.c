#include "samples/prototypes.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define SUCCESS 0

/* Program: mywhich                                                             
 ---------------------------------------------------------                      
 * This program is used to search for directory paths that contain the executab\
le files for a                                                                  
 * given input command. This program by default uses the path specified by the \
current path                                                                    
 * variable for environment, but can also be used with the MYPATH argument to s\
earch for                                                                       
 * paths given the custom MYPATH variable. This program will return the path fo\
r the file                                                                      
 * if a readable, executable version of the pathname of the desired command is \
found and                                                                       
 * nothing if no path is found.                                                 
 */
int main(int argc, char *argv[], const char *envp[]) {
  // Used to set search path to custom values specified by MYPATH argument      
  const char *searchpath = get_env_value(envp, "MYPATH");
    if (searchpath == NULL) {
        searchpath = get_env_value(envp, "PATH");
    }
    // Prints out the PATH variable if no arguments are passed to the function  
    if (argc == 1) {
        char dir[PATH_MAX];
        const char *remaining = searchpath;

        printf("Directories in search path:\n");
        while (scan_token(&remaining, ":", dir, sizeof(dir))) {
            printf("%s\n", dir);
        }
    }
    // If arguments are passed to the function, searches the specified searchpa\
th for the command                                                              
    // specified in the arguments passed to the function and prints out the dir\
ectory of the first                                                             
    // path it finds that contains the given file/directory and moves onto the \
next commands that                                                              
    // have been supplied to function. Prints out nothing if no path is found   
    else {
      char dir[PATH_MAX];
      // Loops over each command that is input in the function                  
      for (int i = 1; argv[i] != NULL; i++) {
        const char *remaining = searchpath;
        while(scan_token(&remaining, ":", dir, sizeof(dir))) {
          char slash[PATH_MAX] = "/";
          char* pathname = strcat(dir, strcat(slash,argv[i]));
          // Prints pathname and advancs loop to next argument supplied to prog\
ram                                                                             
          if ((access(pathname, R_OK) == SUCCESS) && (access(pathname, X_OK) ==\
 SUCCESS)) {
            printf("%s\n", pathname);
            break;
          }
        }
      }
    }

    return 0;
}

