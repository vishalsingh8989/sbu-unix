#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/defs.h>
#include <sys/syscall.h>

struct dirent *readdir(DIR * dir)
{
    struct dirent *curr_dir = dir->ptr_dir;
    int entry = getdents(dir->dfd, curr_dir, sizeof(struct dirent));

    if (entry == -1 || entry == 0) {
        return NULL;
    }
    //TODO seek required??At this point , I don't care.
    return dir->ptr_dir;
}
