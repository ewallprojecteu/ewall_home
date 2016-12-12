#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include "json_spirit_dgw.h"

/*#ifndef JSON_SPIRIT_MVALUE_ENABLED
#error Please define JSON_SPIRIT_MVALUE_ENABLED for the mValue type to be enabled 
#endif*/

using namespace std;
//using namespace json_spirit_dgw;

int main()
{
    string file = "moteIn.cfg";
    ifstream moteifs(file.c_str());
    string jsonStr;
    std::stringstream buffer;
    buffer << moteifs.rdbuf();
    jsonStr = buffer.str();
    moteifs.close();

    cout << "Testing JsonToMoteInfo and moteInfoToJSON\n";
    
    moteInfo mote = JsonToMoteInfo(jsonStr);

    ofstream fileout;
    fileout.open("moteOut.cfg");
    fileout << moteInfoToJSON(mote);
    fileout.close();

    file = "avDevIn.cfg";
    moteifs.open(file.c_str());
    jsonStr.erase();
    buffer.str(string());
    buffer << moteifs.rdbuf();
    jsonStr = buffer.str();
    moteifs.close();

    cout << "Testing JsonToAVDeviceInfo and avDeviceInfoToJSON\n";
    
    avDeviceInfo avDevInfo = JsonToAVDeviceInfo(jsonStr);

    fileout.open("avDevOut.cfg");
    fileout << avDeviceInfoToJSON(avDevInfo);
    fileout.close();

    file = "avDevsIn.cfg";
    moteifs.open(file.c_str());
    jsonStr.erase();
    buffer.str(string());
    buffer << moteifs.rdbuf();
    jsonStr = buffer.str();
    moteifs.close();

    cout << "Testing JsonToVecAVDeviceInfo and vecAVDeviceInfoToJSON\n";

    vector<avDeviceInfo> avDevs = JsonToVecAVDeviceInfo(jsonStr);

    fileout.open("avDevsOut.cfg");
    fileout << vecAVDeviceInfoToJSON(avDevs);
    fileout.close();

    file = "moteDetailsIn.cfg";
    moteifs.open(file.c_str());
    jsonStr.erase();
    buffer.str(string());
    buffer << moteifs.rdbuf();
    jsonStr = buffer.str();
    moteifs.close();

    cout << "Testing JsonToVecMoteDetails and vecMoteDetailsToJSON\n";

    vector<moteDetails> moteDevs = JsonToVecMoteDetails(jsonStr);

    fileout.open("moteDetailsOut.cfg");
    fileout << vecMoteDetailsToJSON(moteDevs);
    fileout.close();

    file = "measIn.cfg";
    moteifs.open(file.c_str());
    jsonStr.erase();
    buffer.str(string());
    buffer << moteifs.rdbuf();
    jsonStr = buffer.str();
    moteifs.close();
    
    cout << "Testing JsonToMeasurements and measurementsToJSON\n";

    vector<measurement> meass = JsonToMeasurements(jsonStr);

    fileout.open("measOut.cfg");
    fileout << measurementsToJSON(meass);
    fileout.close();

    file = "sensIn.cfg";
    moteifs.open(file.c_str());
    jsonStr.erase();
    buffer.str(string());
    buffer << moteifs.rdbuf();
    jsonStr = buffer.str();
    moteifs.close();

    cout << "Testing JsonToSensors and sensorsToJSON\n";
    
    vector<sensor> senss = JsonToSensors(jsonStr);

    fileout.open("sensOut.cfg");
    fileout << sensorsToJSON(senss);
    fileout.close();

    file = "streamConfIn.cfg";
    moteifs.open(file.c_str());
    jsonStr.erase();
    buffer.str(string());
    buffer << moteifs.rdbuf();
    jsonStr = buffer.str();
    moteifs.close();

    cout << "Testing JsonToStreamConfig and streamConfigToJSON\n";
    
    streamConfig conf = JsonToStreamConfig(jsonStr);

    fileout.open("streamConfOut.cfg");
    fileout << streamConfigToJSON(conf);
    fileout.close();
    
    return 1;
}
