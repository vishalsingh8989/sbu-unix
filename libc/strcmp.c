#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/defs.h>

//TODO: should folow strcmp convention...should output binary difference
int strcmp(const char * str1, const char * str2) {
    char cmp1 = 1, cmp2 = 1;
    int result = 1;
    int idx = 0;
  
    if (str1 == NULL || str2 == NULL)
       return -1;

    //TODO: change to do while
    while (cmp1 != '\0' && cmp2 != '\0' && result != 0) {
        cmp1 = str1[idx];
        cmp2 = str2[idx];
        result = (cmp1 == cmp2);

        if (cmp1 == '\0' && cmp2 != '\0') result = 0;
        if (cmp2 == '\0' && cmp1 != '\0') result = 0;

        idx++;
    }
    return !result;
}
