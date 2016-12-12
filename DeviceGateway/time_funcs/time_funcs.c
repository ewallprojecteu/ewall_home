/* 
* SMART FP7 - Search engine for MultimediA enviRonment generated contenT
* Webpage: http://smartfp7.eu
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
* The Original Code is Copyright (c) 2012-2013 Athens Information Technology
* All Rights Reserved
*
* Contributor:
*  Nikolaos Katsarakis nkat@ait.edu.gr
*/

#include "time_funcs.h"

#ifdef __linux__
#include <sys/time.h>  
#endif //__linux__

#include <math.h>     // for floor() 

int my_round(double d)
{
  return (int) floor(d + 0.5);
}

/* Get the number of milliseconds since 01-01-1970 (UNIX epoch) */
long long getMillis()
{
#ifdef __linux__
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (long long)tv.tv_sec*1000+tv.tv_usec/1000;
#else
	/* Currently crude approximation using standard C lib */
	/* Could be improved by using OS-specific functions   */
	static int called=0;
	static clock_t start_clocks;
	static time_t start_seconds;
	long long ret;
	time_t cur_seconds;
	clock_t cur_clocks;
	if (called++==0)
	{
		cur_clocks=clock();
		cur_seconds = time(NULL);
		/* Busy wait till the first change in seconds */
		do
		{
			start_clocks=clock();
			start_seconds = time(NULL);
		}
		while (start_seconds==cur_seconds);
		called=1;
		return (long long)start_seconds*1000;
	}	
	cur_clocks=clock();
	ret = 1000*(long long)start_seconds+1000*(cur_clocks - start_clocks)/CLOCKS_PER_SEC;
	if (called%2==0)
	{
		cur_seconds = time(NULL);
		if ((ret/1000)!=cur_seconds)
		{
			printf("actual:%ld calculated:%ld, recalculating time", cur_seconds, (long) (ret/1000));
			called=0;
		}
	}
	return ret;
#endif
}

/* Convert milliseconds to string in xsd:dateTime format with UTC timezone
   i.e. YYYY-MM-DDThh:mm:ss.nnnZ 
   where YYYY: year, MM: month, DD: day,
   hh: hour, mm: minute, ss: second, nnn: millisecond
   
   parameters: 
   millis: the number to convert
   date_time: string buffer to store the result
   len: length of string buffer 
   
   returns:
   0 on success
   -1 on error */
int millis2string(long long millis, char *date_time, size_t len)
{
	time_t seconds;
	long millisec;
	time_t this_time;
	struct tm * ptm;
	int time_len;

	if (len < 25)
	{
		fprintf(stderr, "millis2string: not big enough buffer size (%ld)\n", len);
		return -1;
	}

	if (millis < 0)
	{
		fprintf(stderr, "millis2string: called with negative ms value %lld\n", millis);
		return -1;
	}
	seconds = millis / 1000;
	millisec = millis % 1000;

	this_time = seconds;
	ptm = gmtime( &this_time );
	if (ptm == NULL)
	{
		fprintf(stderr, "millis2string: too large ms value %lld\n", millis);
		return -1;
	}

	time_len = strftime(date_time,len-5,"%Y-%m-%dT%H:%M:%S", ptm);
	if (time_len == 0)
	{
		fprintf(stderr, "millis2string: not big enough buffer size (%ld)\n", len);
		return -1;
	}
	sprintf(&date_time[time_len],".%03ldZ",millisec);
	return 0;
}


/* Convert string to milliseconds in xsd:dateTime format with UTC timezone
   i.e. input YYYY-MM-DDThh:mm:ss.nnnZ 
   where YYYY: year, MM: month, DD: day,
   hh: hour, mm: minute, ss: second, nnn: millisecond
   
   parameters: 
   date_time: input string with format YYYY-MM-DDThh:mm:ss.nnnZ
   millis: the number to convert
   
   returns:
   0 on success
   -1 on error */
int string2millis(const char *date_time, long long *millis)
{
	static time_t epoch_time_t=-1;
	static double diff=-1;
	struct tm stm;
	double seconds, d_sec, d_msec;

	//On first use estimate the seconds difference between local and utc time
	if (epoch_time_t==-1)
	{     
		struct tm epoch;
		time_t rawtime=time(0);
		 //Difference in seconds between local time and utc time
		diff = difftime(mktime(localtime(&rawtime)), mktime(gmtime(&rawtime)));
		epoch.tm_year=70; epoch.tm_mday=1; epoch.tm_mon=epoch.tm_hour=epoch.tm_min=epoch.tm_isdst=0;
		epoch.tm_sec=(int) diff; //add the difference in seconds to convert to local time 
		epoch_time_t=mktime(&epoch); //Calculate the time_t value equivalent of epoch, usually is 0 but we should not rely on it
		if (epoch_time_t==-1)
		{
			printf("string2millis: error calculating epoch time\n");
			return -1;
		}
	}

	// exit if not able to read the string
	if (sscanf(date_time,"%d-%d-%dT%d:%d:%lfZ",&stm.tm_year,&stm.tm_mon,&stm.tm_mday,&stm.tm_hour,&stm.tm_min,&seconds)!=6)
		return -1;

	stm.tm_year-=1900; //tm_year is number of years since 1900
	stm.tm_mon-=1; //tm_mon is from 0-11 instead of 1-12
	//Until now we have utc time in the structure without valid seconds
	
	//split the seconds to integer and float part
	d_msec = modf (seconds , &d_sec);
	stm.tm_isdst=0;

	//Add the integer number of seconds from input and convert it to local time
	stm.tm_sec=(int)d_sec + (int) diff;

	//Convert the local time structure into time_t and calculate difference from epoch
	*millis = (long long) difftime(mktime(&stm),epoch_time_t)*1000 + (long long) my_round(d_msec*1000);
	return 0;
}
