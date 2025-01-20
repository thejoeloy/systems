* Name: Joseph Loy Assigment 4 CS107 Spring 2019                               
 * Program : myls.c                                                             
 * This program is used to print out the contents of a directory                
 * given a filepath and client provided comparison functions                   \
                                                                                
*/

#include <sys/types.h>
#include <dirent.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MINIMUM_SIZE 50

enum { SORT_BY_NAME, SORT_BY_TYPE };
enum { EXCLUDE_DOT, INCLUDE_DOT };

typedef int (*filter_fn)(const struct dirent* a);
typedef int (*compare_fn)(const struct dirent** a, const struct dirent** b);

/* On the myth filesystem, the only file type information that is accurate is   
 * directory/not-directory. Other type info in struct direct is not reliable.   
 */
bool is_dir(const struct dirent *dirptr) {
    return dirptr->d_type == DT_DIR;
}

/* Function: printSort
 *
 */							\
void printSort(struct dirent** namelist, int result) {
  for (int i = 0; i < result; i++) {
    char* pathname = (*namelist[i]).d_name;

    if (is_dir(namelist[i])) {
      pathname[strlen(pathname)] = '/';
      size_t end = strcspn(pathname,"/");
      pathname[end + 1] = '\0';
    }
    printf("%s\n", pathname);
  }

}

 * This function allows hidden directories to be included in the directory scan
 */
int filter_includehidden(const struct dirent* a) {
  return 1;
}

int filter_default(const struct dirent* a) {
  const char* name = (*a).d_name;

  if (*name == '.') {
    return 0;
  } else {
    return 1;
  }
}

int cmp_name(const struct dirent** a, const struct dirent** b) {
  const char* name1 = (**a).d_name;
  const char* name2 = (**b).d_name;
  return strcmp(name1,name2);
}
/* Function: cmp_type                                                           
 ----------------------------------------------------------------------         
 * This function returns postive if type1 is a directory and type2 is a file, n\
egative if                                                                      
 * type1 is a file and type2 is a directory, and compares the entries lexograph\
ically                                                                          
 * if they are of the same type and returns a value in the same manner as cmp_n\
ame                                                                             
 */
int cmp_type(const struct dirent** a, const struct dirent** b) {
  const char* name1 = (**a).d_name;
  const char* name2 = (**b).d_name;
  const unsigned char type1 = (**a).d_type;
  const unsigned char type2 = (**b).d_type;

  // Check if the types are not the same and return directory if they are diffe\
rent                                                                            
  if (type1 != type2) {
    if (is_dir(*a)) {
      return -1;
    } else {
      return 1;
    }
  }
  // If types are the same, return the larger string lexographically            
  else {
    return strcmp(name1,name2);
  }
}

/* Function: ls                                                                 
 --------------------------------------------------------------------           
 * This function is used for listing out directories of a directory path. If gi\
ven                                                                             
 * the flag -a at runtime, the program will include hidden directories in the r\
eturn. Otherwise                                                                
 * the function will print out only functions that dont begin with '.'. If the \
flag -z is given,                                                               
 * then the function sorts the current directory by type(directory or file) and\
 breaks ties between                                                            
 * types by lexographic ordering.                                               
 */
void ls(const char *dirpath, int filter, int order) {
  if (opendir(dirpath) == NULL) {
    printf("./myls: cannot access %s\n", dirpath);
  }

  filter_fn fltr = filter_default;
  compare_fn cmp = cmp_name;
  if (filter == INCLUDE_DOT) {
    fltr = filter_includehidden;
  }
  if (order == SORT_BY_TYPE) {
    cmp = cmp_type;
  }
  struct dirent** namelist;
  int result = scandir(dirpath, &namelist, fltr, cmp);
  printSort(namelist,result);
}

int main(int argc, char *argv[]) {
    int order = SORT_BY_NAME;
    int filter = EXCLUDE_DOT;
    // Determine what flags user enters in and modify comparison and filter fun\
ctions accordingly                                                              
    int opt = getopt(argc, argv, "az");
    while (opt != -1) {
        if (opt == 'a') {
            filter = INCLUDE_DOT;
        } else if (opt == 'z') {
            order = SORT_BY_TYPE;
        } else {
            return 1;
	}
        opt = getopt(argc, argv, "az");
    }

    if (optind < argc - 1) {
	for (int i = optind; i < argc; i++) {
            printf("%s:\n", argv[i]);
            ls(argv[i], filter, order);
            printf("\n");
	}
    } else {
	ls(optind == argc -1 ? argv[optind] : ".", filter, order);
    }

    return 0;
}

