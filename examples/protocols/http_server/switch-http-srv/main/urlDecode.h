#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* Function: urlDecode */
char *urlDecode(const char *str) {
  bool d = false; // whether or not the string is decoded
  char *dStr = malloc(strlen(str) + 1);
  char eStr[] = "00"; // for a hex code
  strcpy(dStr, str);

  while(!d) {
    d = true;
    for(int i = 0; i < strlen(dStr); i++) {
      if(dStr[i] == '%') {
        if(dStr[i+1] == 0) {
          return dStr;
        }
        if(isxdigit(dStr[i+1]) && isxdigit(dStr[i+2])) {
          d = false;
          /* combine the next to numbers into one */
          eStr[0] = dStr[i+1];
          eStr[1] = dStr[i+2];
          /* convert it to decimal */
          long int x = strtol(eStr, NULL, 16);
          /* remove the hex */
          memmove(&dStr[i+1], &dStr[i+3], strlen(&dStr[i+3])+1);
          dStr[i] = x;
        }
      }
    }
  }
  return dStr;
}

#ifdef __cplusplus
}
#endif
