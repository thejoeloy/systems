/* Name: Joseph Loy Assigment 3 CS107 Spring 2019                               
 * Program : mytail.c                                                           
 * This program is used to print out the n lines of a file according to the val\
ue                                                                              
 * and file that the user specifies                                             
 */
#include "samples/prototypes.h"
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Function: print_last_n                                                       
 * ------------------------------------------------------                       
 * This function takes in a filestream and an integer representing the number o\
f the final                                                                     
 * lines of the file that the user wants printed to the screen.                 
 */
void print_last_n(FILE *fp, int n) {
  // Keep track of the lines being stored in                                    
  char* lines[n];
  for (int i = 0; i < n; i++) {
    lines[i] = NULL;
  }
  char* curLine;
  int i = 0;
  // While input can still be read from the file, we read it into curline and s\
tore it into lines                                                              
  while ((curLine = read_line(fp))) {
    // Free memory of line that no longer needs to be used                      
    if (lines[i] != NULL) {
      free(lines[i]);
    }
    lines[i] = curLine;
    i++;
    if (i >= n) {
      i = 0;
    }
  }
  // Print out the array                                                        
  for (int startVal = i; startVal < n; startVal++) {
    // Special case for an array smaller than the minimum allocated size        
    if (lines[n-1] == NULL) {
      break;
    }
    printf("%s\n", lines[startVal]);
  }
 for (int j = 0; j < i; j++) {
    printf("%s\n", lines[j]);
  }
  // Free memory                                                                
  for (int k = 0; k < n; k++) {
    if (lines[k] != NULL) {
      free(lines[k]);
    }
  }
}

int convert_arg(const char *str, int max) {
    char *end;
    long n = strtol(str, &end, 10);
    if (*end != '\0') {
	error(1, 0, "Invalid number '%s'", str);
    }
    if (n < 1 || n > max) {
        error(1, 0, "%s is not within the acceptable range [%d, %d]", str, 1, m\
ax);
    }
    return n;
}

int main(int argc, char *argv[]) {
    int num = 10;

    if (argc > 1 && argv[1][0] == '-') {
        num = convert_arg(argv[1] + 1, 100000);
        argv++;
        argc--;
    }

    FILE *fp;
    if (argc == 1) {
        fp = stdin;
    } else {
        fp = fopen(argv[1], "r");
        if (fp == NULL) {
            error(1, errno, "cannot access '%s'", argv[1]);
        }
    }
    print_last_n(fp, num);
    fclose(fp);
    return 0;
}


