/*=================================================================================
Description: Sends messages to couchdb

Authors: Nikolaos Katsarakis (AIT) (nkat@ait.edu.gr)

Revision History:
    13.11.2014, ver 0.1
====================================================================================*/

#include <iostream>

class Communication
{
public:
	Communication(std::string url);
	~Communication();

	int sendMessage(long long timestamp, bool active, double luminance);

private:
	std::string _url;
	bool sentActive;
	double sentLuminance;
};

