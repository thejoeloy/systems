/* Name: Joseph Loy Assigment 4 CS107 Spring 2019                                                   
 * Program : mysort.c                                                                               
 * This program is used to print out the particular lines of a file                                  
 * given particular commands given at the command line.                                              
 */
#include "samples/prototypes.h"
#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAX_LINE_LEN 4096
#define MIN_NLINES 100

typedef int (*cmp_fn_t)(const void *p, const void *q);

// Compares array members according to lexographic order (earlier lexographic -> later lexographic   
int cmp_pstr(const void *p, const void *q) {
  char** string1 = (char**)p;
  char** string2 = (char**)q;
  return strcmp(*string1,*string2);
}

// Compares array members according to length of the line (fewer chars -> more chars)                
int cmp_pstr_len(const void *p, const void *q) {
  char** line1 = (char**)p;
  char** line2 = (char**)q;
  return (strlen(*line1) - strlen(*line2));
}

// Sort array members according to integer representation (small ints -> big ints)                   
int cmp_pstr_numeric(const void *p, const void *q) {
  char** string1 = (char**)p;
  char** string2 = (char**)q;
  return atoi(*string1) - atoi(*string2);
}

/* Function: sort_lines                                                                            
 -------------------------------------------------------------------                               
 * This function takes in a file(or input stream), a client supplied comparison function, and two  
 * boolean values that are set by command line arguments and sorts the elements according to the   
 * command line arguments(default is non-unique, lexographic, standard ordering). This function    
 * can filter out unique lines(-u), sort by length(-l), sort by numeric value(-n), or reverse the  
 * order of the input(-r)                                                                          
 */
void sort_lines(FILE *fp, cmp_fn_t cmp, bool uniq, bool reverse) {
  char line[MAX_LINE_LEN];
  char** lines = calloc(MIN_NLINES, sizeof(line));
  size_t numlines = 0;
  size_t curMax = MIN_NLINES;

  while(fgets(line, MAX_LINE_LEN, fp)) {
    if (numlines >= curMax) {
      // Double the size of lines, change curMax to proper val                                       
      lines = realloc(lines, 2 * numlines * sizeof(line));
      curMax *= 2;
    }
    if (uniq) {
      const void* key = strdup(line);
      binsert(&key, lines, &numlines, sizeof(char*), cmp);
    } else {
      // Add all strings in order                                                                    
      char* lineCopy = strdup(line);
      size_t newLine = strcspn(lineCopy, "\n");
      lineCopy[newLine] = '\0';
      lines[numlines] = lineCopy;
      numlines++;
    }
  }

  // If the array is called without -u flag, sort array using qsorts                                 
  if (!uniq) {
    qsort(lines, numlines, sizeof(lines), cmp);
  }

  // If reverse flag is given, print the array in reverse order                                      
  if (reverse) {
    for (int i = numlines - 1; i >= 0; i--) {
      printf("%s\n", lines[i]);
      free(lines[i]);
    }
  } else {
    // Print out the array in normal order                                                           
    for (int i = 0; i < numlines; i++) {
      printf("%s\n", lines[i]);
      free(lines[i]);
    }
  }
  free(lines);

}

int main(int argc, char *argv[]) {
    cmp_fn_t cmp = cmp_pstr;
    bool uniq = false;
    bool reverse = false;

    int opt = getopt(argc, argv, "lnru");
    while (opt != -1) {
        if (opt == 'l') {
            cmp = cmp_pstr_len;
        } else if (opt == 'n') {
            cmp = cmp_pstr_numeric;
        } else if (opt == 'r') {
            reverse = true;
        } else if (opt == 'u') {
            uniq = true;
        } else {
            return 1;
        }

        opt = getopt(argc, argv, "lnru");
    }

    FILE *fp = stdin;
    if (optind < argc) {
        fp = fopen(argv[optind], "r");
	if (fp == NULL) {
            error(1, 0, "cannot access %s", argv[optind]);
        }
    }
    sort_lines(fp, cmp, uniq, reverse);
    fclose(fp);
    return 0;
}

    while (opt != -1) {
        if (opt == 'l') {
            cmp = cmp_pstr_len;
        } else if (opt == 'n') {
            cmp = cmp_pstr_numeric;
        } else if (opt == 'r') {
            reverse = true;
        } else if (opt == 'u') {
            uniq = true;
        } else {
            return 1;
        }

        opt = getopt(argc, argv, "lnru");
    }

    FILE *fp = stdin;
    if (optind < argc) {
        fp = fopen(argv[optind], "r");
	if (fp == NULL) {
            error(1, 0, "cannot access %s", argv[optind]);
        }
    }
    sort_lines(fp, cmp, uniq, reverse);
    fclose(fp);
    return 0;
}

