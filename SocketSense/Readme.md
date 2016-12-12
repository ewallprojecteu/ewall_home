# Socket Sensing
Report the power consumption data collected by the Plugwise Stick to a Home PC database in CouchDB (Linux version)

## Operation
Plugwise does not provide any support for Linux. So a solution handling the Stick and Circles from the open source community in utilised. Clone the SevenW/Plugwise-2-py repository from https://github.com/SevenW/Plugwise-2-py and follow the installation instructions in their README.md. The web server is not needed. Effectively, this is a Python implementation of a Plugwise metadata logger.

Next you configure the logger and run the system.

### Configuring the logger
Configure reporting following the instructions in their README.md. In config/pw-hostconfig.json edit tmp_path, permanent_path and log_path. Leave serial as is. A typical config/pw-hostconfig.json is as follows:

```
{
	"permanent_path": "/home/apne/plugwise/datalog",
	"tmp_path": "/tmp",
	"log_path": "/home/apne/plugwise/pwlog",
	"serial": "/dev/ttyUSB0",
	"log_format": "epoch",
	"mqtt_ip": "127.0.0.1",
	"mqtt_port": "1883"
}
```

Next, edit config/pw-conf.json to describe the appliances connected to the plugwise network. The loginterval should be set to 1 minute, the mac to the MAC addresses of the circles and the always_on and production to false. The category, name and location properties are mapped onto the type, name and room properties of the reported JSON in the CouchDB. A typical config/pw-conf.json is as follows:

```
{
	"static": [
		{
			"mac": "000D6F0004A1D42E",
			"category": "Lamp",
			"name": "Lamp",
			"loginterval": "1",
			"always_on": "False",
			"production": "False",
			"location": "Lab"
		},
		{
			"mac": "000D6F0004529CC1",
			"category": "PC",
			"name": "PC",
			"loginterval": "1",
			"always_on": "False",
			"production": "False",
			"location": "Lab"
		}
]}
```

Finally, edit config/pw-control.json to describe the switching of the appliances connected to the plugwise network. Match correctly the "mac" and "name" properties. Make sure that you select "switch_state": "on", "savelog": "yes" and "monitor": "yes". A typical config/pw-control.json is as follows:

```
{
	"dynamic": [
		{
			"mac": "000D6F0004A1D42E",
			"switch_state": "on",
			"name": "Lamp",
			"schedule_state": "off",
			"schedule": "",
			"savelog": "yes",
			"monitor": "yes"
		},
		{
			"mac": "000D6F0004529CC1",
			"switch_state": "on",
			"name": "PC",
			"schedule_state": "off",
			"schedule": "",
			"savelog": "yes",
			"monitor": "yes"
		}
	], 
	"log_level": "info", 
	"log_comm": "no"
}
```

The three JSON files can be found at the root of the SocketSense project. Just copy them in the /config directory of Plugwise-2-py.

### Running the system
- Plug the Stick to a USB USB of the Home PC. Check that indeed it is connected as ttyUSB0 by invoking ls /dev/ttyUSB* before and after and seeing what is new. If necessary modify config/pw-hostconfig.json
- Launch the Plugwise logging mechanism by invoking sudo python Plugwise-2.py
- If running for the first time, wait for the initial logfile to be written. This may take a while. When the first file with the current date appears, then this process has completed.
- Generate the JAR file from the Java project. Check that the provided init.json matches your configuration file locations. Also provide the CouchDB URL and the database name. A typical init.json is like:

```
{
	"configFile" : "/home/apne/plugwise/Plugwise-2-py/config/pw-conf.json",
	"dataLogDir" : "/home/apne/plugwise/datalog/",
	"couchdburl" : "http://localhost:5984",
	"dbName" : "plugwise"
}
```

- Launch the JAR fle at any time, terminate it whenever needed and re-launch it. The program finds the latest entry in the CouchDB and spans the logfiles thereafter for new metadata. When there are no more entries in the logfiles, it waits for new entries to appear.
