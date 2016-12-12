/* 
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

#include "curl_helper.h"
#include <stdlib.h>

/* Curl handle */
CURL *curl_session = NULL;

/* Header list */
struct curl_slist *header = NULL;

/* Debug level */
int curl_debug_level = 0;

/* store result code from curl functions */
CURLcode res;

/* Store the response code from http */
long http_response_code = 0;

/* Store the current URL */
char * curl_address;

typedef struct {
  char *ptr;
  size_t len;
}buff;

void init_buffer(buff *s) {
  s->len = 0;
  s->ptr = (char*) malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

/* This dummy function can be used if we want to avoid printing the received data */
size_t dummy_write(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	return size * nmemb;
}

/* This function just prints the received data on stdout, should be used to reset after a GET */
size_t cout_write(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	printf("%s",ptr);
	return size * nmemb;
}

// This function is used for reading data from GET request into provided buffer
//based on http://stackoverflow.com/a/2329792
static size_t write_callback(void *received, size_t size, size_t nmemb, buff *s)
{
	size_t new_len = s->len + size*nmemb;
	s->ptr = (char*) realloc(s->ptr, new_len+1);
	if (s->ptr == NULL) {
		fprintf(stderr, "realloc() failed\n");
		exit(EXIT_FAILURE);
	}
	memcpy(s->ptr+s->len, received, size*nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size*nmemb;
}


int curl_init(const char *url)
{
	/* Check that we haven't initialised already */
	if(curl_session != NULL)
	{
		fprintf(stderr, "curl_init() has been called before, ignoring\n");
		return -1;
	}

	curl_address = NULL;

	/* In windows, this will init the winsock stuff */ 
	res = curl_global_init(CURL_GLOBAL_DEFAULT);
	/* Check for errors */ 
	if(res != CURLE_OK) 
	{
		fprintf(stderr, "curl_global_init() failed: %s\n",
			curl_easy_strerror(res));
		return -1;
	}

	/* get a curl handle */ 
	curl_session = curl_easy_init();

	if(curl_session == NULL)
	{
		fprintf(stderr, "curl_easy_init() failed\n");
		curl_global_cleanup();
		return -1;
	}

	/* We are now ready to perform operations on the curl handle */
	//fprintf(stdout, "curl_easy_init to server %s \n", url);

	/* Set the URL for the POST request */ 
	curl_address = (char *) malloc(strlen(url)*sizeof(char)+1);
	strcpy(curl_address,url);
	if (curl_address == NULL) {
		fprintf(stderr, "malloc() failed\n");
		return -1;
	}
	curl_easy_setopt(curl_session, CURLOPT_URL, curl_address);


	/* Set content-type to json */
	header = curl_slist_append(header, "Content-Type: application/json");
	curl_easy_setopt(curl_session, CURLOPT_HTTPHEADER, header);

	/* Set to fail if the HTTP request returns error */
	curl_easy_setopt(curl_session, CURLOPT_FAILONERROR, 1L);

	return 0;
}

/* Sets the verbosity of the output */
int curl_set_debug_level(int debug_level)
{
	/* Check that curl session is initialised */
	if (curl_session==NULL)
	{
		fprintf(stderr, "CURL has not been initialised, call curl_init() first\n");
		return -1;
	}
	curl_debug_level=debug_level;
	if (curl_debug_level>1)
	{
		/* get verbose debug output please */ 
		curl_easy_setopt(curl_session, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl_session, CURLOPT_WRITEFUNCTION, &cout_write);
	}
	else if (curl_debug_level==0)
	{
		/* avoid printing HTTP response */
		curl_easy_setopt(curl_session, CURLOPT_WRITEFUNCTION, &dummy_write);
	}
	return 0;
}



int curl_send(const char * data, int len)
{
	/* Check that curl session is initialised */
	if (curl_session==NULL)
	{
		fprintf(stderr, "CURL has not been initialised, call curl_init() first\n");
		return -1;
	}

	/* Configure the data to POST */
	curl_easy_setopt(curl_session, CURLOPT_POSTFIELDS, data);

	/* Set the request data size, since we know it beforehand */
	curl_easy_setopt(curl_session, CURLOPT_POSTFIELDSIZE, len);

	/* Perform the request, res will get the return code */ 
	res = curl_easy_perform(curl_session);

	/* Also get the HTTP Response code */
	curl_easy_getinfo(curl_session, CURLINFO_RESPONSE_CODE, &http_response_code);
	return res;
}

/* Should be called if curl_send() returns error to return detailed error code */ 
int curl_show_error()
{
	const char* error = curl_easy_strerror(res);

	/* If an HTTP error has occurred, return the http code*/
	if (strstr(error,"HTTP")!=NULL)
	{
		if (http_response_code == HTTP_CONFLICT)
		{
			fprintf(stderr,"Data already exists\n");
		}
		else if (http_response_code == HTTP_NOT_FOUND)
		{
			fprintf(stderr, "Address not found\n");
		}
		else if (http_response_code == HTTP_BAD_REQUEST)
		{
			fprintf(stderr, "Bad request (HTTP error %ld)\n", http_response_code);
		}
		else
		{
			fprintf(stderr,"Unknown HTTP response code %ld\n", http_response_code);
		}
		return http_response_code;
	}
	else /* Some other error occured, return the curl error code */
	{
		fprintf(stderr, "curl_easy_perform() failed: %s\n", error);
		return res;
	}
}

/* Can be called if curl_send() returns CURLE_OK to show any warnings */ 
void curl_show_warning()
{
	if (http_response_code == HTTP_CREATED)
	{
		/* Don't print all the time the success */
		if (curl_debug_level>0) printf("Data inserted successfully\n");
	}
	else
	{
		fprintf(stderr, "Operation was successful but response code is unknown %ld\n", http_response_code);
	}
}


void curl_cleanup()
{
	/* Check that curl session is initialised */
	if (curl_session==NULL)
	{
		fprintf(stderr, "CURL has not been initialised, call curl_init() first\n");
		return;
	}
	/* Free the header list*/
	curl_slist_free_all(header);

	/* Free the allocated url */
	free(curl_address);

	/* always cleanup */ 
	curl_easy_cleanup(curl_session);
	curl_session = NULL;
	curl_global_cleanup();
}

void curl_setURL(const char *url)
{
	curl_address = (char*) realloc(curl_address,strlen(url)*sizeof(char)+1);
	if (curl_address == NULL) {
		fprintf(stderr, "realloc() failed\n");
		exit(EXIT_FAILURE);
	}
	strcpy(curl_address,url);
	/* Set the URL for the POST request */ 
	curl_easy_setopt(curl_session, CURLOPT_URL, curl_address);
}

const char* curl_getURL()
{
	return curl_address;
}

int curl_receive(char *data, size_t max_size)
{
	buff read_buffer;
	init_buffer(&read_buffer);
	/* Check that curl session is initialised */
	if (curl_session==NULL)
	{
		fprintf(stderr, "CURL has not been initialised, call curl_init() first\n");
		return -1;
	}


	/* Configure the callback function to call on GET */
	curl_easy_setopt(curl_session, CURLOPT_WRITEFUNCTION, write_callback);

	/* Set the memory where the data will be retrieved */
	curl_easy_setopt(curl_session, CURLOPT_WRITEDATA, &read_buffer);

	/* Perform the request, res will get the return code */ 
	res = curl_easy_perform(curl_session);

	/* Also get the HTTP Response code */
	curl_easy_getinfo(curl_session, CURLINFO_RESPONSE_CODE, &http_response_code);
	
	strncpy(data, read_buffer.ptr, max_size);

	/* Easy way to reset the CURLOPT_WRITEFUNCTION */
	curl_set_debug_level(curl_debug_level);

	return res;
}
