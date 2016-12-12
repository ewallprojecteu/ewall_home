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

/* This file uses libcurl which comes with the following notice: 
*
** COPYRIGHT AND PERMISSION NOTICE
** 
** Copyright (c) 1996 - 2012, Daniel Stenberg, <daniel@haxx.se>.
** 
** All rights reserved.
** 
** Permission to use, copy, modify, and distribute this software for any purpose
** with or without fee is hereby granted, provided that the above copyright
** notice and this permission notice appear in all copies.
** 
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS. IN
** NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
** DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
** OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
** OR OTHER DEALINGS IN THE SOFTWARE.
** 
** Except as contained in this notice, the name of a copyright holder shall not
** be used in advertising or otherwise to promote the sale, use or other dealings
** in this Software without prior written authorization of the copyright holder.
** 
*
* This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
* KIND, either express or implied.
*
***************************************************************************/ 

// Note: if linking with the static version of CURL, make sure you have #defined CURL_STATICLIB
// #define CURL_STATICLIB

#ifdef  __cplusplus
extern "C" {
#endif

//Remove annoying Visual Studio warnings about unsafe functions
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif //_CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

/* Some HTTP return codes */
#define HTTP_OK 200
#define HTTP_CREATED 201
#define HTTP_BAD_REQUEST 400
#define HTTP_NOT_FOUND 404
#define HTTP_CONFLICT 409
#define HTTP_UNSUPPORTED_MEDIA 415
#define HTTP_SERVER_ERROR 500

/* Should be called to initialise all connections */
int curl_init(const char *url);

/* Sets the verbosity of the output */
int curl_set_debug_level(int debug_level);

/* Sends the data and returns the CURL error code */
int curl_send(const char * data, int len);

/* Can be called if curl_send() returns CURLE_OK to show any warnings */ 
void curl_show_warning();

/* Should be called if curl_send() returns error to return detailed error code */ 
int curl_show_error();

/* Should be called to close all connections */
void curl_cleanup();

/* Change the server address */
void curl_setURL(const char *url);

/* Retrieve the current server address */
const char* curl_getURL();

/* Receive data from server */
int curl_receive(char *data, size_t max_size);

#ifdef  __cplusplus
}
#endif
