#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/defs.h>

int strlen(const char *s){
    uint64_t len = 0;

    while (*s  !=  '\0') {
    		len++;
        s++;
    }
    return len;
}
