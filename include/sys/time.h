#ifndef _TIME_H
#define _TIME_H
#include<sys/defs.h>

#define	SECOND_REG		0x00
#define	MIN_REG			0x02
#define	HOUR				0x04
#define DAY_REG			0x07
#define MON_REG			0x08
#define	YEAR_REG			0x09

#define CMOS_ADD			0x70
#define CMOS_DATA		0x71




struct tm{
	int    tm_sec ;  //seconds;  // [0,61]
	int    tm_min  ;//  minutes;  // [0,59]
	int    tm_hour ;// hour;     // [0,23]
	int    tm_mday ;// day of month [1,31]
	int    tm_mon  ;// month of year [0,11]
	int    tm_year ;// years since 1900
	int    tm_wday ;// day of week [0,6] (Sunday = 0)
	unsigned char    tm_yday ;// day of year [0,365]
	unsigned char    tm_isdst;// daylight savings flag}
};

//http://pubs.opengroup.org/onlinepubs/7908799/xsh/time.h.html
typedef int64_t time_t;

struct timespec {
    time_t tv_sec; //seconds
    long tv_nsec; // nanoseconds
};

void init_time();
void read_rtc(struct tm* utc);
void set_system_uptime(char* timestr);
void get_system_uptime(char *buff);


#endif
