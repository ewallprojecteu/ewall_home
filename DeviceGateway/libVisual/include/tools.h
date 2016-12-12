#include <iostream>

std::string getName (const std::string& str, std::string* ext=NULL);
int createFolder(const std::string& path);

//Returns false if folder does not exist, true if it exists
bool folderExists(const std::string& path);

