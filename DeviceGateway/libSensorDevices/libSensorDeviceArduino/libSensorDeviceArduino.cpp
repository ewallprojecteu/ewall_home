/*=================================================================================
 Basic explanation:

 Device Gateway cpp code

 Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)
          Maria Mitoi, Razvan Craciunescu (UPB) (maria.mitoi@radio.pub.ro)
 Revision History:
 25.08.2014, ver 0.1
 ====================================================================================*/

#include "libSensorDeviceArduino.h"
#include "time_funcs.h"
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include<sys/ioctl.h>
#include<fcntl.h>
#include<termios.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/ioctl.h>
#include <linux/serial.h>
#include "time_funcs.h"


#define START_l 1
#define LENGTH_l 2 //length field - 2bytes
#define TYPE_l 1   //packet type field - 1byte
#define RADDR_l 8  //radio address field - 8bytes
#define NADDR_l 2  //network address field - 2bytes
#define ROPT_l 1   //receiving options field - 1byte
#define FLAG_l 1   //flags field - 1byte (uint_8 parameter)
#define TEMP_l 4   //temperature field -4bytes (float parameter)
#define HUM_l 4    //humidity field - 4bytes (float parameter)
#define LIGHT_l 4  //light field - 4bytes (float parameter)
#define LPG_l 1
#define NG_l 1
#define CO_l 1
//modul medical
#define BPM_l 2
#define SPO_l 2
typedef unsigned char uchar;

pthread_t hthreadArduino;

std::string byte_2_str(char* bytes, int size) {
	char const hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
			'A', 'B', 'C', 'D', 'E', 'F' };
	std::string str;
	for (int i = 0; i < size; ++i) {
		const char ch = bytes[i];
		str.append(&hex[(ch & 0xF0) >> 4], 1);
		str.append(&hex[ch & 0xF], 1);
	}
	return str;
}

void char_2_bin(char c) {
	for (int i = 7; i >= 0; --i) {
		putchar((c & (1 << i)) ? '1' : '0');
	}
	putchar('\n');
}

uchar packet_type(unsigned char buffer[]) {
	uchar type = buffer[0];
	return type;
}
;
string packet_radio_addr(unsigned char buffer[]) {
	char* raddr_bytes;
	raddr_bytes = new char[RADDR_l];
	for (int i = 0; i < RADDR_l; i++) {
		raddr_bytes[i] = buffer[TYPE_l + i];
	}
	string raddr_str = byte_2_str(raddr_bytes, RADDR_l); //final address
	delete [] raddr_bytes;
	return raddr_str;
}
;
string packet_ntw_addr(unsigned char buffer[]) {
	char* naddr_bytes;
	naddr_bytes = new char[NADDR_l];
	for (int i = 0; i < NADDR_l; i++) {
		naddr_bytes[i] = buffer[i + TYPE_l + RADDR_l];
	}
	string naddr_str = byte_2_str(naddr_bytes, NADDR_l); //final address
	delete [] naddr_bytes;
	return naddr_str;
}
;
int* packet_flags(unsigned char buffer[]) {
	static int flag_array[4];
	int flag = (int) buffer[TYPE_l + RADDR_l + NADDR_l + ROPT_l];
	for (int i = 0; i < 4; i++)
		flag_array[i] = flag & (1 << i) ? 1 : 0;
	return flag_array;
}
;
float packet_hum(unsigned char buffer[]) {
	union param_hum {
		uchar b_hum[4];
		float val_hum;
	} hum;
	for (int i = 0; i < HUM_l; i++) {
		hum.b_hum[i] = buffer[TYPE_l + RADDR_l + NADDR_l + ROPT_l + FLAG_l + i];
	}
	//test for nan
	if (hum.val_hum != hum.val_hum)
		return -1;
	else
		return hum.val_hum;
}
;
float packet_temp(unsigned char buffer[]) {
	union param_temp {
		uchar b_temp[4];
		float val_temp;
	} temp;
	for (int i = 0; i < TEMP_l; i++) {
		temp.b_temp[i] = buffer[TYPE_l + RADDR_l + NADDR_l + ROPT_l + FLAG_l
				+ HUM_l + i];
	}
	if (temp.val_temp != temp.val_temp)
		return -1;
	else
		return temp.val_temp;
}
;
float packet_light(unsigned char buffer[]) {
	union param_light {
		uchar b_light[4];
		float val_light;
	} light;
	for (int i = 0; i < LIGHT_l; i++) {
		light.b_light[i] = buffer[TYPE_l + RADDR_l + NADDR_l + ROPT_l + FLAG_l
				+ HUM_l + TEMP_l + i];
	}
	if (light.val_light != light.val_light)
		return -1;
	else
		return light.val_light;
}
;
uchar packet_LPG(unsigned char buffer[]){
	uchar LPG = buffer[TYPE_l+RADDR_l+NADDR_l+ROPT_l+FLAG_l+HUM_l+TEMP_l+LIGHT_l];
	return LPG;
};
uchar packet_NG(unsigned char buffer[]){
	uchar NG = buffer[TYPE_l+RADDR_l+NADDR_l+ROPT_l+FLAG_l+HUM_l+TEMP_l+LIGHT_l+LPG_l];
	return NG;
};
uchar packet_CO(unsigned char buffer[]){
	uchar CO = buffer[TYPE_l+RADDR_l+NADDR_l+ROPT_l+FLAG_l+HUM_l+TEMP_l+LIGHT_l+LPG_l+NG_l];
	return CO;
};
uchar packet_door(unsigned char buffer[]){
	uchar door_open = buffer[TYPE_l+RADDR_l+NADDR_l+ROPT_l+FLAG_l+HUM_l+TEMP_l+LIGHT_l+LPG_l+NG_l+CO_l];
	return door_open;
};

//medical module
unsigned short packet_bpm(unsigned char buffer[]){
	unsigned short bpm=(buffer[TYPE_l+RADDR_l+NADDR_l+ROPT_l+FLAG_l+1]<<8 | buffer[TYPE_l+RADDR_l+NADDR_l+ROPT_l+FLAG_l]);
	if (bpm != bpm) return -1;
	else return bpm;
};

unsigned short packet_spo(unsigned char buffer[]){
	unsigned short spo=(buffer[TYPE_l+RADDR_l+NADDR_l+ROPT_l+FLAG_l+BPM_l+1]<<8 | buffer[TYPE_l+RADDR_l+NADDR_l+ROPT_l+FLAG_l+BPM_l]);
		if (spo != spo) return -1;
		else return spo;
};



void send_packet(int file_descriptor, unsigned char ch, int len) {
	uchar buf[len];
	for (int i = 0; i < len; i++) {
		read(file_descriptor, &ch, 1);
		buf[i] = ch;
		//printf("0x%02x\n",buf[i]);

	}

	uchar type = packet_type(buf);
	//printf("Packet type: 0x%02x\n", type);

	//testing whether the packet is of interest,
	//or any other type of packets exchanged in xbees' communication
	if (type == 0x90) {

		string radio_address = packet_radio_addr(buf);
		string network_address = packet_ntw_addr(buf);

		bool LPG_flag, NG_flag, CO_flag, PIR_flag;
		int *flags = packet_flags(buf);
		if (*flags == 1)
			LPG_flag = true;
		else
			LPG_flag = false;
		if (*(flags + 1) == 1)
			NG_flag = true;
		else
			NG_flag = false;
		if (*(flags + 2) == 1)
			CO_flag = true;
		else
			CO_flag = false;
		if (*(flags + 3) == 1)
			PIR_flag = true;
		else
			PIR_flag = false;

		float hum = packet_hum(buf);
		float temp = packet_temp(buf);
		float light = packet_light(buf);

		if ((hum == -1) || (temp == -1) || (light == -1))
			printf("Packet distorted.");
		else {
			/*printf("x:%.02f\n", hum);
			printf("y:%.02f\n", temp);
			printf("z:%.02f\n", light);*/
		}
	} else
		printf("Packet not worth decoding.");

}

int length(int fd, unsigned char ch) {
	unsigned int len = 0;
	for (int i = 0; i < LENGTH_l; i++) {
		read(fd, &ch, 1);
		//printf("0x%02x\n",ch);
		len |= ch;
	}
	return len;
}

inline void *threadRunArduino(void *object) {
	SensorDevicesArduino* mysm = (SensorDevicesArduino*) object;
	int connID = mysm->getConnID();
	unsigned char ch;
	bool rd = 1;
	int x = 0;
	while (1) {
		usleep(1000);
		int n = read(connID, &ch, 1);
		if (n != -1) {
			if (ch == 0x7e) {
				//printf("start packet %d\n", x++);
				int packet_length = length(connID, ch);
				//printf("Packet length: %d\n", packet_length);

				string moteID = "";
				int numSens = 0;

				uchar buf[packet_length];
				for (int i = 0; i < packet_length; i++) {
					read(connID, &ch,1);
					buf[i] = ch;
				}

				uchar type = packet_type(buf);
				//printf("Packet type: 0x%02x\n", type);

				//testing whether the packet is of interest,
				//or any other type of packets exchanged in xbees' communication
				if (type == 0x90) {

					string radio_address = packet_radio_addr(buf);
					string network_address = packet_ntw_addr(buf);

					vector < moteDetails > sensorMotes =
							mysm->getAllSensorMotesDetails();
					vector<moteDetails>::iterator it;
					string mac = packet_radio_addr(buf);
					it = sensorMotes.begin();
					for (it = sensorMotes.begin(); it != sensorMotes.end();
							it++) {
						if ((it)->smoteInfo.macAddress == mac) {
							moteID = (it)->smoteInfo.sensorMoteID;
							numSens = (it)->smoteInfo.numSensors;
							break;
						}
					}

					if ((moteID == "env_livingroom") || (moteID == "env_kitchen") || (moteID == "env_bathroom") || (moteID == "env_bedroom")) { //(mac == "0013A20040B40FE0") {

						bool LPG_flag, NG_flag, CO_flag, PIR_flag;

						int *flags = packet_flags(buf);
						if (*flags == 1)
							LPG_flag = true;
						else
							LPG_flag = false;
						if (*(flags + 1) == 1)
							NG_flag = true;
						else
							NG_flag = false;
						if (*(flags + 2) == 1)
							CO_flag = true;
						else
							CO_flag = false;
						if (*(flags + 3) == 1)
							PIR_flag = true;
						else
							PIR_flag = false;

						float PIR = (float) (*(flags + 3));

						float gases = 0;
						for (int i = 0; i < 3; i++) {
							gases += (float) ((*(flags + i)) * (2 ^ i));
						}

						float hum = packet_hum(buf);
						float temp = packet_temp(buf);
						float light = packet_light(buf);

						uchar LPG=packet_LPG(buf);
						uchar NG=packet_NG(buf);
						uchar CO=packet_CO(buf);
						uchar door = packet_door(buf);

						if ((hum == -1) || (temp == -1) || (light == -1))
							printf("Packet distorted.");

						else {
							long long timestamp = getMillis();
							vector < measurement > allMeas;
							vector<sensor>::iterator its;
							for (its = (it)->sensorsInfo.begin(); its != (it)->sensorsInfo.end(); its++)							
							{
								if ((its)->type == _TEMPERATURE) allMeas.push_back(measurement(moteID, _TEMPERATURE, temp+(its)->offset, timestamp));
								else if ((its)->type == _HUMIDITY) allMeas.push_back(measurement(moteID, _HUMIDITY, hum+(its)->offset, timestamp));
								else if ((its)->type == _LUMINOCITY) allMeas.push_back(measurement(moteID, _LUMINOCITY, light+(its)->offset, timestamp));
								else if ((its)->type == _ACTIVITY) allMeas.push_back(measurement(moteID, _ACTIVITY, PIR+(its)->offset, timestamp));
								else if ((its)->type == _LPG) allMeas.push_back(measurement(moteID, _LPG, LPG+(its)->offset, timestamp));
								else if ((its)->type == _NG) allMeas.push_back(measurement(moteID, _NG, NG+(its)->offset, timestamp));
								else if ((its)->type == _CO) allMeas.push_back(measurement(moteID, _CO, CO+(its)->offset, timestamp));
								else if ((its)->type == _DOOR) allMeas.push_back(measurement(moteID, _DOOR, door+(its)->offset, timestamp)); 
							}

							mysm->pushMeasurements(moteID, allMeas);
						}
					}
					else if (moteID == "mote_lillypad") { //(mac == "0013A2004089EC56") {
						float accelX = packet_hum(buf);
						float accelY = packet_temp(buf);
						float accelZ = packet_light(buf);

						if ((accelX == -1) || (accelY == -1) || (accelZ == -1))
							printf("Packet distorted.");

						else {
							long long timestamp = getMillis();
							vector < measurement > allMeas;
							vector<sensor>::iterator its;
							for (its = (it)->sensorsInfo.begin(); its != (it)->sensorsInfo.end(); its++)							
							{
								if ((its)->type == _ACCELEROMETER_X) allMeas.push_back(measurement(moteID, _ACCELEROMETER_X, accelX+(its)->offset, timestamp));
								else if ((its)->type == _ACCELEROMETER_Y) allMeas.push_back(measurement(moteID, _ACCELEROMETER_Y, accelY+(its)->offset, timestamp));
								else if ((its)->type == _ACCELEROMETER_Z) allMeas.push_back(measurement(moteID, _ACCELEROMETER_Z, accelZ+(its)->offset, timestamp));
							}

							mysm->pushMeasurements(moteID, allMeas);
						}
					}
					else if (moteID == "mote_medical") { //(mac == "0013A200408B4610") {
						unsigned short BPM=packet_bpm(buf);
						unsigned short SPO=packet_spo(buf);
						if ((SPO==-1)||(BPM==-1)) printf("Packet distorted.");
						else {
							long long timestamp = getMillis();
							vector < measurement > allMeas;
							vector<sensor>::iterator its;
							for (its = (it)->sensorsInfo.begin(); its != (it)->sensorsInfo.end(); its++)							
							{
								if ((its)->type == _BPM) allMeas.push_back(measurement(moteID, _BPM, BPM+(its)->offset, timestamp));
								else if ((its)->type == _SPO) allMeas.push_back(measurement(moteID, _SPO, SPO+(its)->offset, timestamp));
							}

							mysm->pushMeasurements(moteID, allMeas);
						}
					}
				}
			}
		}				//end while

	}				//end if
}				//end function

SensorDevicesArduino::~SensorDevicesArduino() {
}

SensorDevicesArduino::SensorDevicesArduino() {
}

int SensorDevicesArduino::initializeDevice(vector<moteDetails> sensorMotes) {
	int ret = initialize(sensorMotes);
	if (ret == 1) {
		string connStr = sensorMotes.begin()->smoteInfo.connectionStr;

		// open communication port

		connID = open(connStr.c_str(), O_RDWR | O_NOCTTY);

		if (connID < 0) {
			std::cout << "Error " << errno << " opening " << connStr << ": "
					<< strerror(errno) << std::endl;
		} else {
			std::cout << "Port successfully opened - USB: " << connID
					<< std::endl;
		}

		struct termios tio;
		memset(&tio, 0, sizeof(tio));
		tio.c_iflag = 0;
		tio.c_oflag = 0;
		tio.c_cflag = CS8 | CREAD | CLOCAL;           // 8n1
		tio.c_lflag = 0;
		tio.c_cc[VMIN] = 0;
		tio.c_cc[VTIME] = 50;
		cfsetospeed(&tio, B9600);
		cfsetispeed(&tio, B9600);
		tcsetattr(connID, TCSANOW, &tio);

		pthread_create(&hthreadArduino, NULL, &threadRunArduino, this);
		cout << "SensorDevices: Initialization success. " << sensorMotes.size() << " sensor motes registered!\n";
	} else {
		cout << "SensorDevices: Initialization failed!\n";
		return 0;
	}
	return 1;
}
