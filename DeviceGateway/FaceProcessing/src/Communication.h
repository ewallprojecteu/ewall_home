/*=================================================================================
Description: Sends messages to couchdb

Authors: Nikolaos Katsarakis (AIT) (nkat@ait.edu.gr)

Revision History:
    13.11.2014, ver 0.1
====================================================================================*/

//Remove annoying Visual Studio warnings about unsafe functions
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif //_CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "target.h"


class Communication
{
public:
	Communication(std::string url);
	~Communication();

	int sendMessage(int64 frameTime, const std::vector<targetStruct>& targets);

private:
	std::string _url;
	std::vector<targetStruct> lastSentTargets;
	int64 sentTime;
	bool sending;
};

