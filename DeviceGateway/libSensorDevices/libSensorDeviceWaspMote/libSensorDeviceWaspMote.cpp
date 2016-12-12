/*=================================================================================
Basic explanation:

    Device Gateway cpp code

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    25.08.2014, ver 0.1
====================================================================================*/

#include "libSensorDeviceWaspMote.h"
#include "time_funcs.h"
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>   
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h> 
#include <termios.h>
#include <iostream>
#include <string>
#include <stdint.h>

pthread_t threadRunPortID;

float AccDataProcessing(uint8_t high, uint8_t low)
{
	//checking if the Acc_X_High_byte was changed
	uint8_t High_byte_indicator = high & 0x40;
	if(High_byte_indicator != 0)
	{
		high = high ^ 0xc0;
	}

	//checking if the Acc_X_Low_byte was changed
	uint8_t Low_byte_indicator = high & 0x20;
	if(Low_byte_indicator != 0)
	{
		high = high ^ 0x20;
		low = low ^ 0x80;
	}

	//concatenation of the Acc_X_High and Acc_X_Low
	uint16_t Acc_X_un = high;
	Acc_X_un = Acc_X_un << 8;
	Acc_X_un = Acc_X_un | low;
	//printf("Acc_X_un: %d\n",Acc_X_un);

	int16_t Acc_X = 0;

	//checking if the 1-st bit of Acc_X_un is 1
	uint16_t Sign_Indicator = Acc_X_un & 0x8000;
	//printf("Sign_Indicator: %d\n",Sign_Indicator);
	if(Sign_Indicator != 0)
	{
		Acc_X_un = Acc_X_un ^ 0x8000;
		Acc_X = (-1) * Acc_X_un;
	}
	else
	{
		Acc_X = Acc_X_un;
	}
	float Acc_float = (float)Acc_X;
	//printf("Acc: %f \n\n",Acc_float);
	return Acc_float;

}


void *threadRunPort(void *object)
{	
	SensorDevicesWaspMote* mysm = (SensorDevicesWaspMote*)object;
	int connID = mysm->getConnID();
	unsigned char buf_c = '\0';
	unsigned char buf[256];
	memset (&buf, '\0', sizeof buf);
	
	int flag_start_byte0 = 0;
	int flag_start_byte1 = 0;
	int flag_read_length = 0;
	int length = 0;
	int read_mode = 0;
	int counter = 0;

	/* *** READ *** */
	while(1)
	{
		usleep(1000);
		int n = read( connID, &buf_c , 1);
		if(n != -1)
		{	
			/*Check if it is a beginning of a packet (first byte 126, second byte 0)*/			
			if((int)buf_c==126)
			{
				flag_start_byte0 = 1;
				//cout << "StartByte0: " << (int)buf_c << endl;
			}
			else if((int)buf_c==0 && flag_start_byte0 == 1)
			{
				flag_start_byte1 = 1;
				//cout << "StartByte1: " << (int)buf_c << endl;
			}
			else if(flag_start_byte0 == 1 && flag_start_byte1 == 1 && flag_read_length == 0)
			{
				flag_start_byte0 = 0;
				flag_start_byte1 = 0;
				length = (int)buf_c;
				read_mode = 1;
				counter = 0;
				//cout << "Length: " << (int)buf_c << endl;
			}
			else if(read_mode == 1 && counter < length)
			{
				if((int)buf_c == 125)
				{
					length++;
					//cout << "Length after 125: " << length << endl;
				}
				buf[counter] = buf_c;
				//cout << "Packet[" << counter << "]: " << (int)buf[counter] << endl;
				counter++;
			}
			else if(read_mode == 1 && counter == length)
			{	
				counter = 0;
				read_mode = 0;
				
				
				/*cout << endl;
				cout << "-----------------------------------------------------------" << endl;
				cout << "--------------------New packet received--------------------" << endl;
				std::cout << "Number of bytes: " << length << std::endl;*/
				
				//Back conversion: (17->125;49   19->125;51   126->125;94   125->125;93)
				n = length;
				int NumBytesBeforeEschaped = n;
				for(int i=0; i<=(n-1); i++)
				{
					if((int)buf[i]==125)
					{
						NumBytesBeforeEschaped--;
					}
				}
				unsigned char BufBeforeEschaped[256];
				int NumCorrections = 0;
				for(int i=0; i<=(n-1); i++)
				{
					if((int)buf[i]!=125)
					{
						BufBeforeEschaped[i-NumCorrections] = buf[i];
					}
					else
					{
						switch( (int)buf[i+1] )
						{
							case 49 :	BufBeforeEschaped[i-NumCorrections] = 17;
							break;
							case 51 :	BufBeforeEschaped[i-NumCorrections] = 19;
							break;
							case 94 :	BufBeforeEschaped[i-NumCorrections] = 126;
							break;
							case 93 :	BufBeforeEschaped[i-NumCorrections] = 125;
							break;
						}
						NumCorrections++;
						i++;
					}	
				}
				//Get MAC address from the header
				int MACaddr_start = 1;
				unsigned char MAC_address[8];
				for(int i=MACaddr_start; i<=(MACaddr_start+7); i++)
				{
					MAC_address[i-MACaddr_start] = (int)BufBeforeEschaped[i];	
				}

				char const hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',   'B','C','D','E','F'};

	  			std::string mac;
	  			for (int i = 0; i < 8; ++i) 
				{
	    				const char ch = MAC_address[i];
	    				mac.append(&hex[(ch  & 0xF0) >> 4], 1);
	    				mac.append(&hex[ch & 0xF], 1);
				}
			
			
			
				//Check if the MAC address is from Waspmotes
				if((int)MAC_address[0] == 0x00 && (int)MAC_address[1] == 0x13 && (int)MAC_address[2] == 0xa2 && (int)MAC_address[3] == 0x00)
				{
					//Print the MAC address
					/*printf("Source MAC address: ");
					printf("%02X", (int)MAC_address[0]);
					printf("%02X", (int)MAC_address[1]);
					printf("%02X", (int)MAC_address[2]);
					printf("%02X", (int)MAC_address[3]);
					printf("%02X", (int)MAC_address[4]);
					printf("%02X", (int)MAC_address[5]);
					printf("%02X", (int)MAC_address[6]);
					printf("%02X\n", (int)MAC_address[7]);
					printf("\n");*/
			
			
					int firstDateByteIdx = 18;
					int numSens = 0;
					string moteID = "";

					vector<moteDetails> sensorMotes = mysm->getAllSensorMotesDetails();
					vector<moteDetails>::iterator it;
				
					it = sensorMotes.begin();
					//string connStr = (it)->smoteInfo.connectionStr;
					for (it = sensorMotes.begin(); it != sensorMotes.end(); it++)
					{
						if ((it)->smoteInfo.macAddress == mac)
						{
							moteID = (it)->smoteInfo.sensorMoteID;
							numSens = (it)->smoteInfo.numSensors;
							break;
						}
					}
	
					if((moteID == "env_wearable"))
					{
						//Odreduvanje na indeksot na prviot bajt od data Payload-ot
						int indeks_na_prv_bajt_od_Podatocite = 18;
			
						int Num_of_measurements_int = (int)BufBeforeEschaped[indeks_na_prv_bajt_od_Podatocite];
						float Num_of_measurements_float = (float)Num_of_measurements_int;
						//printf("Number of measurements = %f \n\n", Num_of_measurements_float);

						int Num_of_sensors = (int)BufBeforeEschaped[indeks_na_prv_bajt_od_Podatocite+1];
						float Num_of_sensors_float = (float)Num_of_sensors;
						//printf("Number of sensors = %f \n\n", Num_of_sensors_float);

						int Sensor1_type_int = (int)BufBeforeEschaped[indeks_na_prv_bajt_od_Podatocite+2];
						float Sensor1_type_float = (float)Sensor1_type_int;
						//printf("Sensor1_type = %f \n\n", Sensor1_type_float);

						int Sensor2_type_int = (int)BufBeforeEschaped[indeks_na_prv_bajt_od_Podatocite+3];
						float Sensor2_type_float = (float)Sensor2_type_int;
						//printf("Sensor2_type = %f \n\n", Sensor2_type_float);

						int Sensor3_type_int = (int)BufBeforeEschaped[indeks_na_prv_bajt_od_Podatocite+4];
						float Sensor3_type_float = (float)Sensor3_type_int;
						//printf("Sensor3_type = %f \n\n", Sensor3_type_float);

					
						int index = indeks_na_prv_bajt_od_Podatocite + 5;
						long long time = getMillis();
						for(int i=0; i<Num_of_measurements_int; i++)
						{
							vector<measurement> allMeas(3);

							float accX = AccDataProcessing((int)BufBeforeEschaped[index], (int)BufBeforeEschaped[index+1]);
							accX = (accX/1024)*9.81;

							float accY = AccDataProcessing((int)BufBeforeEschaped[index+2], (int)BufBeforeEschaped[index+3]);
							accY = (accY/1024)*9.81;

							float accZ = AccDataProcessing((int)BufBeforeEschaped[index+4], (int)BufBeforeEschaped[index+5]);
							accZ = (accZ/1024)*9.81;
	 						
							allMeas[0] = measurement(moteID, _ACCELEROMETER_X, accX, time-50*(Num_of_measurements_int-i));
							allMeas[1] = measurement(moteID, _ACCELEROMETER_Y, accY, time-50*(Num_of_measurements_int-i));
							allMeas[2] = measurement(moteID, _ACCELEROMETER_Z, accZ, time-50*(Num_of_measurements_int-i));
						
							mysm->pushMeasurements(moteID,allMeas);

							index = index + 6;
						}
					
			
					}
					else if ((moteID == "env_livingroom") || (moteID == "env_kitchen") || (moteID == "env_bathroom") || (moteID == "env_bedroom"))
					{
						//cout << "from moteID=" << moteID << endl;
						vector<measurement> allMeas;
						long long time = getMillis();
						int j = 0;
						int idxT = firstDateByteIdx + 5;
						int idxM = 0;
						for(int i=0; i<numSens; i++)
						{	
							string type = "";
							string meas = "";

							char *data = (char*)BufBeforeEschaped;
							string dataStr(data+firstDateByteIdx);
						
							while(BufBeforeEschaped[idxT] != '=')
							{
								type.append(&data[idxT],1);
								idxT++;
							}
							idxM = idxT + 1;
						
							while(BufBeforeEschaped[idxM] != ';')
							{
								meas.append(&data[idxM],1);
								idxM++;
							}
							idxT = idxM + 1;
							//cout << "Type: " << atoi(type.c_str()) << " and Value: " << atof(meas.c_str()) <<endl;
							vector<sensor>::iterator its;
							for (its = (it)->sensorsInfo.begin(); its != (it)->sensorsInfo.end(); its++)							
							{	
								if ((its)->type == (sensorType)atoi(type.c_str())) 										allMeas.push_back(measurement(moteID, (its)->type, atof(meas.c_str())+(its)->offset, time));
							}
	
						}
						//cout << endl;
				
						mysm->pushMeasurements(moteID,allMeas);
					}
				}
				else
				{
					cout << "The received packet is not data packet!" << endl;
				}
				
			}	
		}
		
	}
}

SensorDevicesWaspMote::~SensorDevicesWaspMote()
{
}

SensorDevicesWaspMote::SensorDevicesWaspMote()
{
}

int SensorDevicesWaspMote::initializeDevice(vector<moteDetails> sensorMotes)
{
	int ret = initialize(sensorMotes);
	if (ret == 1)
	{
		string connStr = sensorMotes.begin()->smoteInfo.connectionStr;

		// open communication port
		
		int USB = open( connStr.c_str(), O_RDWR | O_NDELAY );
		connID = USB;

		if ( USB < 0 )
		{
			std::cout << "Error " << errno << " opening " << connStr << ": " << strerror (errno) << std::endl;
		}
		else
		{
			std::cout << "Port successfully opened - USB: " << USB << std::endl;
		}
	
		struct termios tty;
		memset (&tty, 0, sizeof tty);
	
		if ( tcgetattr ( USB, &tty ) != 0 )
		{
			std::cout << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
		}
		
		/* Configure port */
		cfsetospeed (&tty, B38400);
		cfsetispeed (&tty, B38400);
		tty.c_cflag     &=  ~PARENB;        
		tty.c_cflag     &=  ~CSTOPB;
		tty.c_cflag     |=  CS8;
		tty.c_cflag     &=  ~CRTSCTS;       
		tty.c_cflag     |=  CREAD | CLOCAL;    
		tty.c_iflag     &=  ~(IXON | IXOFF | IXANY);
		tty.c_lflag     &=  ~(ICANON | ECHO | ECHOE | ISIG);
		tty.c_oflag     &=  ~OPOST;
	
		tcflush( USB, TCIFLUSH );
		
		if ( tcsetattr ( USB, TCSANOW, &tty ) != 0)
		{
			std::cout << "Error " << errno << " from tcsetattr" << std::endl;
		}

		pthread_create(&threadRunPortID, NULL, threadRunPort, this);
		cout << "SensorDevices: Initialization success. " << sensorMotes.size() << " sensor motes registered!\n";
	}
	else
	{
		cout << "SensorDevices: Initialization failed!\n";
		return 0;
	}
	return 1;
}
