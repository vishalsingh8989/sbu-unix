#ifndef _USER_H
#define _USER_H

#define admin_uid 0

typedef char * string;
//char users[10][50];
string users[5];
void init_users();
void set_uid(uint64_t user_id);
uint64_t get_uid();
#endif
