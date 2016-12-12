Tool for processing provided accelerometer data from Device Gateway and reporting metadata

Usage:
 	accelerometer reporting_interval [couchdb_url]

Parameters:
	reporting_interval : reporting interval in milliseconds,
			defaults to 10000 if invalid input or <=0
	couchdb_url : URL to an existing CouchDB database to store metadata,
			disabled if not specified or set to 'localhost'

Example:
accelerometer -1 localhost:5984/activity/

Usage:
- Before running the executable, the Device Gateway should be initialized as follows:
- In a new terminal window, go to build/DeviceGatewayConsole and run ./DGWConsolePrRec
- Press 1. Initialize Device Gateway
- Press 2 : Connect a SensorMote to the DeviceGateway
- Type "mote.cfg" as Configuration File
- Type "sample-data.txt" as Results File
- Press 9 : Start periodic measurements for SensorMote
- Type "mote_prrec" as SensorMoteName
- Type "0.05" as ReportingPeriod (Note: This is the time between consecutive measurements in seconds)
- The screen should fill with a list of messages
- Switch to the accelerometer terminal window and run the executable

# For the Lilypad Accelerometer - UPB
#Go to folder /wp3project/DeviceGateway/build/DeviceGatewayConsole and run:
sudo ./DGWConsoleArduino
#Hit 1-> wait for the message. if there are no more errors hit 2-> wait for the message then write:
motesArduino.cfg
#Then all the data should flood your screen. 
#In orer to send the data to calculate the IMA ISA values and send it to the database go to /wp3project/DeviceGateway/build/Accelerometer and run :
sudo ./accelerometer -1 localhost:5984/activity/


---- If you wish to send data to a CouchDB server, you will have to create the appropriate database
For example for the "activity" metadata group:
- Go to the web interface of CouchDB:
	http://<server name>:<port number>/_utils/
- Login using an admin account
- Select "Create database", write "activity" as a name
- The database is ready for use at http://<server name>:<port number>/activity/