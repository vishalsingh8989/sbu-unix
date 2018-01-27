#ifndef _ENV_H
#define _ENV_H

//char *LOGNAME = "admin";
#include <sys/process.h>


#define ENV_PWD		0
#define ENV_PS1  	1
#define ENV_HOME 	2
#define ENV_USER 	3
#define ENV_PATH 	4






void init_env();
char* getenv(int env);
void setenv(int env, char* env_val);

char* getenvp(int env);

#endif
