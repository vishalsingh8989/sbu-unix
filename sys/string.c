#include <sys/defs.h>
#include <sys/string.h>

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

char *strcpy(char *dst, const char *src)
{
    char *dst_t = dst;

    while(*src)
        *dst++ = *src++;
    *dst = '\0';

    return dst_t;
}

uint64_t strlen( const char *s){
    uint64_t len = 0;

    while (*s  !=  '\0') {
    		len++;
        s++;
    }
    return len;
}

int strstr(char *a, char *b) {

  if (strlen(a) < strlen((const char *)b))
    return -1;
  int position = 0;
  char *x, *y;
  x = (char *)a;
  y = b;
  while (*a) {
    while (*x == *y) {
      x++;
      y++;
      if (*x == '\0' || *y == '\0')
        break;
    }
    if (*y == '\0')
      break;
    a++;
    position++;
    x = (char *)a;
    y = b;
  }
  if (*a)
    return position;
  else
    return -1;
}

char *strconcat(char *dst, const char *src) {

  int dst_len = 0;
  char *dest_iter = dst;
  const char *src_iter = src;

  dst_len = strlen(dst);
  // src_len = strlen(src);
  int idx = 0;
  while (idx < dst_len) {
    idx++;
    dest_iter++;
  }

  while (dest_iter && src_iter && *src_iter != '\0') {
    *dest_iter = *src_iter;
    dest_iter++;
    src_iter++;
  }
  *dest_iter = '\0';
  return dest_iter;
}
