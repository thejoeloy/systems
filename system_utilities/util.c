#include "samples/prototypes.h"
#include <string.h>

// Used to compare outcome of strncmp to see if the strings compared are equal  
#define STRINGS_MATCH 0
// Used when incrementing the pointer so that the value of the env variable is \
printed                                                                         
#define ENV_VAR_SHIFT_FACTOR 1
// Used as the delimiter to seperate env variable name with the env variable va\
lue                                                                             
#define DELIM '='

/* Function: get_env_value                                                      
  ---------------------------------------------------                           
  * This function takes in a character array containing pointers to             
  * different strings that represent the user's environment variables           
  * and a search key for a particular variables and returns the value           
  * of the variable if there is an entry for that variable, or null if          
  * there is no variable named after the entered key or the value of the        
  * variable itself is null.                                                    
  */
const char *get_env_value(const char *envp[], const char *key) {
  // Get size of the key                                                        
  size_t strLen = strlen(key);
  // Loop over the character array searching for the env variable               
  for (int i = 0; envp[i] != NULL; i++) {
    // Get size of the environment variable name                                
    size_t varLen = strcspn(envp[i],"=");
    // If first n bytes of the strings are the same and they are the same lengt\
h                                                                               
    // then the user has entered in a correct string for an environment variabl\
e                                                                               
    if ((strncmp(envp[i], key, strLen) == STRINGS_MATCH) && (strLen == varLen))\
 {
      // Use pointer arithmetic to return value of enivironment variable        
      return envp[i] + (strLen + ENV_VAR_SHIFT_FACTOR);
    }
  }
   return NULL;
}


/* Function : scan_token                                                        
  -----------------------------------------------------                         
  * This function takes in an input stream represented as a double char star, a\
 list of                                                                        
  * delimiters, a buffer. and an optional buflen argument that are used to scan\
 a c string                                                                     
  * for the input stream according to the delimiters and writes the token to th\
e array buf[]                                                                   
  * with a size specified by the buflen.                                        
 */
ool scan_token(const char **p_input, const char *delimiters, char buf[], size_\
t buflen) {
  // advance past initial/repeated delimiters                                   
  *p_input += strspn(*p_input, delimiters);
  buflen -= strspn(*p_input, delimiters);
  // If we are passed just delimiters, will return false                        
  if (!strcmp(*p_input, "")) {
    return false;
  }
  // Count spaces to next delimiter                                             
  size_t numChars = strcspn(*p_input,delimiters);
  // If numchars can fit into buf, write numchars into buf and advance pointer \
numChar spaces                                                                  
  if (numChars <= buflen - 1) {
    // Copy over characters in current token over to buf                        
    strncpy(buf,*p_input,numChars);
    // Place null terminating character at the end of buf                       
    buf[numChars] = '\0';
    // Advance pointer to point to delimiter after the token just made          
    *p_input += numChars;
    return true;
  }
}ool scan_token(const char **p_input, const char *delimiters, char buf[], size_\
t buflen) {
  // advance past initial/repeated delimiters                                   
  *p_input += strspn(*p_input, delimiters);
  buflen -= strspn(*p_input, delimiters);
  // If we are passed just delimiters, will return false                        
  if (!strcmp(*p_input, "")) {
    return false;
  }
  // Count spaces to next delimiter                                             
  size_t numChars = strcspn(*p_input,delimiters);
  // If numchars can fit into buf, write numchars into buf and advance pointer \
numChar spaces                                                                  
  if (numChars <= buflen - 1) {
    // Copy over characters in current token over to buf                        
    strncpy(buf,*p_input,numChars);
    // Place null terminating character at the end of buf                       
    buf[numChars] = '\0';
    // Advance pointer to point to delimiter after the token just made          
    *p_input += numChars;
    return true;
  }
}
