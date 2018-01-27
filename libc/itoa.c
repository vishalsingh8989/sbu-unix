#include <stdio.h>
#include <sys/defs.h>
#include <sys/syscall.h>


/* itoa:  convert n to characters in s  from KR book*/
char *itoa(int value, char *str, int base)
{
    char *p_str = str;

    //int index = 0;
    int n = value;
    if (0 == n) {
        *p_str++ = '0';
        *p_str = '\0';
    } else {
        while (n) {
            if (16 == base) {
                int bval = (n % base);
                if (10 == bval) {
                    *p_str = 'A';
                } else if (11 == bval) {
                    *p_str = 'B';
                } else if (12 == bval) {
                    *p_str = 'C';
                } else if (13 == bval) {
                    *p_str = 'D';
                } else if (14 == bval) {
                    *p_str = 'E';
                } else if (15 == bval) {
                    *p_str = 'F';
                } else {
                    *p_str = (n % base) + '0';
                }
            } else {
                *p_str = (n % base) + '0';
            }
            n /= base;
            p_str++;
        }
        *p_str = '\0';
        int str_len = p_str - str;
        int i = 0;
        int ch = 0;
        for (i = 0; i < (str_len / 2); i++) {
            ch = *(str + i);
            *(str + i) = *(str + str_len - 1 - i);
            *(str + str_len - 1 - i) = ch;
        }
    }
    return str;
}
