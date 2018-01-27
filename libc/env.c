
#include <sys/env.h>
#include <stdio.h>


char PWD[MAX_NAME+1];
char PS1[MAX_NAME+1];
char HOME[MAX_NAME+1];
char USER[MAX_NAME+1];
char PATH[MAX_NAME+1];



char* getenvp(int env){


	switch(env){
	case ENV_PWD:
		return PWD;
	case ENV_PS1:
		//printf("get env :  %s.\n", PS1);
		return PS1;
	case ENV_HOME:
		return HOME;
	case ENV_USER:
		return USER;
	case ENV_PATH:
		printf("get HOME :  %s.\n", PATH);
		return PATH;
	}
	return  "";

}

char* getenv(int env){

	switch(env){
	case ENV_PWD:
		return PWD;
	case ENV_PS1:
		//printf("get env :  %s.\n", PS1);
		return PS1;
	case ENV_HOME:
		return HOME;
	case ENV_USER:
		return USER;
	case ENV_PATH:
		return PATH;
	}
	return  "";

}


void setenv(int env, char* env_val){
	//printf("set env : %s\n", env_val);
	switch(env){
	case ENV_PS1:
		memset(PS1, '\0', sizeof(PS1));
		strcpy(PS1,env_val);
		break;
	case ENV_HOME:
		memset(HOME, '\0', sizeof(HOME));
		strcpy(HOME,env_val);
		break;
	case ENV_USER:
		memset(USER, '\0', sizeof(USER));
		strcpy(USER,env_val);
		break;
	case ENV_PATH:
		memset(PATH, '\0', sizeof(PATH));
		strcpy(PATH,env_val);
		break;
	}
}
