#include <stdio.h>
#include <sys/defs.h>
#include <sys/syscall.h>
#include <sys/fs.h>
#include <stdarg.h>
#include <unistd.h>


char getnum (uint64_t value, int base) {
    if (value <= (base-1)) {
        if (value < 10)
        		return ((char) (value+48));
        else
        		return ((char) (value+87));
    }
    else {
    	getnum(value/base, base);
    	getnum(value - (value/base)*base, base);
    	return 0;
    }
}

void putstring (char * value, char* bufff, int idx) {
    while (*value != '\0') {
    bufff[idx++]= *value++;
    }
}

int printf(const char *fmt, ...)
{

	    char buff[64];
	    int count = 0;
	    char *space = " ";
	    va_list val;
	    va_start(val, fmt);
	    while (*fmt) {
	    	if(*fmt == '%'){
	        if (*(fmt + 1) == 's') {
	            char *str_ptr = va_arg(val, char *);
	            while (str_ptr && *str_ptr) {
	                write(1, str_ptr++, 1);
	            }
	            fmt += 2;
	        } else if (( *(fmt + 1) == 'd')||(isdigit(*(fmt + 1)) && *(fmt + 2) == 'd' )  ) {
	            memset(buff, 0, 64);
	            int num = va_arg(val, int);
	            int sp = 0;
	            if(isdigit(*(fmt + 1))){
	            		sp = atoi((char*)(fmt + 1));
	            }
	            if (num < 0) {
	                buff[0] = '-';
	                write(1, buff, 1);
	                buff[0] = 0;
	                num *= -1;
	                sp = sp - 1;
	            }
	            itoa(num, buff, 10);
	            sp = sp - strlen(buff);
	            write(1, buff, strlen(buff));
	            while(sp>0){
	            		write(1, space, 1);
	            		sp--;
	            }
	            fmt += 3;
	        } else if (*(fmt + 1) == 'c') {
	            int ch = va_arg(val, int);
	            write(1, &ch, 1);
	            fmt += 2;
	        } else if (*(fmt + 1) == 'x' || *(fmt + 1) == 'p') {
	            memset(buff, 0, 64);
	            int num = va_arg(val, int);
	            if (num < 0) {
	                buff[0] = '-';
	                write(1, buff, 1);
	                buff[0] = 0;
	                num *= -1;
	            }
	            itoa(num, buff, 16);
	            if (*(fmt + 1) == 'p') {
	                write(1, "0x", 2);
	            }
	            write(1, buff, strlen(buff));
	            fmt += 2;
	        }
	    }
	    else {
	            write(1, fmt, 1);
	            count++;
	            fmt++;
	        }
	    }
	    va_end(val);
	    return count;
	}



