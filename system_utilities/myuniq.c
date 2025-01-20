* Name: Joseph Loy Assigment 3 CS107 Spring 2019                               
 * Program : myuniq.c                                                           
 * This program is used to print out the uniq lines of a file in order of appea\
rance along with                                                                
 * the amount of times that the line appeared in the text                       
 */
#include "samples/prototypes.h"
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* initial estimate of number of uniq lines                                     
 * resize-as-you-go, add in increments of 100                                   
 */
#define ESTIMATE 100

// Struct to hold a unique line. Each entry has a pointer to a line of text and\
 occurences                                                                     
typedef struct unqlin {
  char* text;
  int Occurences;
} unqlin;

/* Function: print_uniq_lins                                                    
 * -------------------------------------------------                            
 * This function takes in a file or input stream and returns the text of each u\
nique line                                                                      
 * in chronological of appearance in the stream and also displays the amount of\
 times that line                                                                
 * shows up in the stream.                                                      
 */
void print_uniq_lines(FILE *fp) {
  unqlin* unqlins = malloc(sizeof(unqlin) * ESTIMATE);
  char* curLin;
  int i = 0;
  int size = 0;
  int maxsize = 100;
  // Build the heap of the data by reading in a line of the file at a time      
  while ((curLin = read_line(fp))) {
    // Resize the heap if it is full                                            
    if (size >= maxsize) {
      maxsize += 100;
      unqlins = realloc(unqlins, sizeof(unqlin) * maxsize);
    }
    unqlins[i].text = calloc(1,strlen(curLin));
    // Search the heap for the string and add an occurence if it is in the heap\
 and new entry if not                                                           
    for (int j = 0; j <= size; j++) {
      if ((!strcmp(unqlins[j].text, curLin))) {
        unqlins[j].Occurences++;
        free(curLin);
        free(unqlins[i].text);
        break;
      }
      // If we get to the end of the heap and the line isnt found, add it to th\
e heap                                                                          
      else if (j == size && strcmp(unqlins[j].text,curLin)) {
        free(unqlins[i].text);
        unqlins[i].text = curLin;
        unqlins[i].Occurences = 1;
        i++;
        size++;
        break;
      }
    }
  }

  // Print the data of the heap and free memory associated with the entry       
  for (int x = 0; x < size; x++) {
    printf("%7d %s\n", unqlins[x].Occurences, unqlins[x].text);
    free(unqlins[x].text);
  }
  free(unqlins);
}

int main(int argc, char *argv[]) {
    FILE *fp;

    if (argc == 1) {
	fp = stdin;
    } else {
	fp = fopen(argv[1], "r");
	if (fp == NULL) {
            error(1, errno, "cannot access '%s'", argv[1]);
	}
    }
    print_uniq_lines(fp);
    fclose(fp);
    return 0;
}


