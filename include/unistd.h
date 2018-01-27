#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/defs.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>

#define PROT_READ	0x1	/* page can be read */
#define PROT_WRITE	0x2	/* page can be written */
#define PROT_EXEC	0x4	/* page can be executed */
#define PROT_SEM	0x8	/* page may be used for atomic ops */
#define PROT_NONE	0x0	/* page can not be accessed */
#define PROT_GROWSDOWN	0x01000000	/* mprotect flag: extend change to start of growsdown vma */
#define PROT_GROWSUP	0x02000000	/* mprotect flag: extend change to end of growsup vma */

#define MAP_SHARED	0x01	/* Share changes */
#define MAP_PRIVATE	0x02	/* Changes are private */
#define MAP_TYPE	0x0f	/* Mask for type of mapping */
#define MAP_FIXED	0x10	/* Interpret addr exactly */
#define MAP_ANONYMOUS	0x20

int open(const char *pathname, int flags);
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int unlink(const char *pathname);

int chdir(const char *path);
char *getcwd(char *buf, size_t size);

pid_t fork();
int execvpe(const char *file, char *const argv[], char *const envp[]);
pid_t wait(int *status);
int waitpid(int pid, int *status);
int dup2(int old_fd, int new_fd);
int access(const char *pathname, int mode);

void yeild();
void shutdown(int code);

unsigned int sleep(unsigned int seconds);

pid_t getpid(void);
pid_t getppid(void);

// OPTIONAL: implement for ``on-disk r/w file system (+10 pts)''
off_t lseek(int fd, off_t offset, int whence);
//int mkdir(const char *pathname, mode_t mode); //TODO: define mode in sys/defs.h

// OPTIONAL: implement for ``signals and pipes (+10 pts)''
int pipe(int pipefd[2]);
fstat_t* stat(int fidx,fstat_t* statbuf);
void dirname(char* buff, char* par_buff);

uint64_t procps();
uint64_t gettime(struct tm* tm_time);
int setuid(int uid);
int  cls_term();
#endif
