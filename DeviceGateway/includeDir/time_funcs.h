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

#ifdef  __cplusplus
extern "C" {
#endif

//Remove annoying Visual Studio warnings about unsafe functions
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif //_CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <time.h>

/* Get the number of milliseconds since 01-01-1970 (UNIX epoch)*/
long long getMillis();

/* Convert milliseconds to string in xsd:dateTime format with UTC timezone
   i.e. [-]YYYY-MM-DDThh:mm:ss.nnnZ 
   where YYYY: year, MM: month, DD: day,
   hh: hour, mm: minute, ss: second, nnn: millisecond */
int millis2string(long long millis, char *date_time, size_t len);

/* Convert string to milliseconds in xsd:dateTime format with UTC timezone
   i.e. input YYYY-MM-DDThh:mm:ss.nnnZ 
   where YYYY: year, MM: month, DD: day,
   hh: hour, mm: minute, ss: second, nnn: millisecond */
int string2millis(const char *date_time, long long *millis);

#ifdef  __cplusplus
}
#endif
