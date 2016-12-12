#########################################################
#                                                       #
# Installation intructions for project and dependencies #
#                                                       #
#########################################################

###Install basic dependencies 
sudo apt-get install automake build-essential cmake couchdb freeglut3-dev g++ git-core libboost-all-dev libcurl4-gnutls-dev libopencv-dev libusb-1.0-0-dev libxi-dev libxmu-dev pkg-config libudev-dev libxtst-dev
#Note: require aditional X11 Record extension library "libxtst-dev" i.e.: "sudo apt-get install libxtst-dev"

###(only for Ubuntu14.04) Manually install some basic dependencies that are not available through the repo system: libudev0:i386, libtiff.so.4, libavcodec.so.53, libavutil.so.51, libavformat.so.53
#Note: These packages need to be downloaded from
http://packages.ubuntu.com/precise/libudev0
http://launchpadlibrarian.net/149178897/libtiff4_3.9.7-2ubuntu1_amd64.deb
http://launchpadlibrarian.net/179993932/libavutil51_0.8.13-0ubuntu0.13.10.1_amd64.deb
http://launchpadlibrarian.net/153479182/libavcodec53_0.8.7-1ubuntu2_amd64.deb
http://launchpadlibrarian.net/153479184/libavformat53_0.8.7-1ubuntu2_amd64.deb
and installed manually:
sudo dpkg -i libudev0_175-0ubuntu9_amd64.deb
sudo dpkg -i libtiff4_3.9.7-2ubuntu1_amd64.deb
sudo dpkg -i libavutil51_0.8.13-0ubuntu0.13.10.1_amd64.deb
sudo dpkg -i libavcodec53_0.8.7-1ubuntu2_amd64.deb
sudo dpkg -i libavformat53_0.8.7-1ubuntu2_amd64.deb


###Manually install libusb (version >= 1.0.18)
#Note: the libusb version in the Ubuntu repos is currently too old!
#Note: the libusb version here (https://github.com/libusb/libusb) is currently too old!
#Download the source code from here: https://sourceforge.net/projects/libusb/files/latest/download?source=files
tar jxf libusb-1.0.19.tar.bz2
cd libusb-1.0.19
./configure
make
sudo make install

###Install libfreenect following mainly the steps described here (http://openkinect.org/wiki/Getting_Started#Clone_libfreenect_Repo) and reproduced below
#Note: before trying to install libfreenect uninstall/remove any previous versions to avoid further errors
sudo apt-get remove freenect
sudo apt-get remove libfreenect
sudo rm -rf /usr/local/include/libfreenect/
sudo rm -rf /usr/local/lib/libfreenect

###Install libfreenect2 (You may skip this if you set BUILD_KINECT2 to off, see below)
#Tested on Ubuntu 14.04 LTS, 64 bit, VGA: NVIDIA Corporation GK107GL [Quadro K2000].
#The following steps most commonly repeat those from the original libfrenect2 repository: https://github.com/OpenKinect/libfreenect2
#Clone libfreenect2:
git clone https://github.com/OpenKinect/libfreenect2.git
#Install dependencies:
> sudo apt-get install build-essential libturbojpeg libjpeg-turbo8-dev libtool autoconf libudev-dev cmake mesa-common-dev freeglut3-dev libxrandr-dev doxygen libxi-dev libopencv-dev automake
> cd libfreenect2/depends
> sh install_ubuntu.sh
> sudo dpkg -i libglfw3*_3.0.4-1_*.deb

#check video graphic adapter and OpenGL version
> lspci | grep VGA
> glxinfo | grep "OpenGL version"

#if GPU is NVIDIA (version below nvidia-346) and OpenGL (version below 4.5.0), go to the link bellow and follow the instructions: http://ubuntuguide.net/install-nvidia-proprietary-drivers-in-ubuntu-14-04
> apt-add-repository ppa:xorg-edgers
> apt-get install nvidia-opencl-dev opencl-headers

#Note: In our setup environment, after drivers update and call: 
glxinfo | grep "OpenGL version"
we have: OpenGL version string: 4.5.0 NVIDIA 346.72

#For other GPU models, follow the instructions in section 3 (OpenCL dependency) from: https://github.com/OpenKinect/libfreenect2

#Build the actual protonect executable and test
> cd ../examples/protonect/
> cmake CMakeLists.txt
> make && sudo make install
> ./bin/Protonect

###Switch between Kinect v1 and Kinect v2 using DeviceGateway (KINECT_V2 is automatically defined if BUILD_KINECT2=ON)
Open: /wp3project/DeviceGateway/libAVDevices/libAVKinect.cpp and:
to use Kinect v2, uncoment line 14: define KINECT_V2 
or
to use Kinect v1, comment line 14: //#define KINECT_V2 

#Clone and compile libfreenect
git clone git://github.com/OpenKinect/libfreenect.git
cd libfreenect
mkdir build
cd build
#to have audio support you should cmake as follows
cmake -L .. -DBUILD_REDIST_PACKAGE=OFF
make
#If you get the message "libusb.h: No such file or directory", the compiler is most probably looking for /usr/include/libusb.h instead of /usr/include/libusb-1.0/libusb.h, fix it using the command
sudo ln -s /usr/include/libusb-1.0/libusb.h /usr/include/libusb.h

#Install libfreenect
sudo make install
sudo ldconfig /usr/local/lib64/
sudo ldconfig
mkdir $HOME/.libfreenect
cp audios.bin $HOME/.libfreenect/
sudo freenect-glview
sudo freenect-micview

#Configure libfreenect so that is works without sudo
sudo adduser $USER video
sudo nano /etc/udev/rules.d/51-kinect.rules
# add the following lines
	# ATTR{product}=="Xbox NUI Motor"
	SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02b0", MODE="0666"
	# ATTR{product}=="Xbox NUI Audio"
	SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02ad", MODE="0666"
	# ATTR{product}=="Xbox NUI Camera"
	SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02ae", MODE="0666"
	# ATTR{product}=="Xbox NUI Motor"
	SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02c2", MODE="0666"
	# ATTR{product}=="Xbox NUI Motor"
	SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02be", MODE="0666"
	# ATTR{product}=="Xbox NUI Motor"
	SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02bf", MODE="0666"
#Log out and log in again


###Install OpenCV following the instructions here (denote that wp3project successfully builds on OpenCV2.4, errors when using OpenCV3.0):
https://help.ubuntu.com/community/OpenCV#Installation

#TODO: Enable/disable kinect and visual processing modules as cmake options
#Kinect2 can be disabled by setting BUILD_KINECT2=OFF

###If you wish to use Eclipse IDE
sudo apt-get install eclipse-cdt

###If you wish to run optimized version of pose face tracker
sudo apt-get install libopenblas-dev liblapack-dev

#Install antidote/signove (Required only if BUILD_BLUEZ=ON)
sudo apt-get install build-essential bluez git automake autoconf libtool libdbus-1-dev libdbus-glib-1-2 libdbus-glib-1-dev libglib2.0-dev libusb-1.0.0-dev libbluetooth-dev
#go to DeviceGateway directory
cd antidote-master
./autogen.sh
./configure
make
sudo make install

###Compile the Device Gateway
rm -r build
mkdir build
cd build
cmake ../    or
cmake ../ -DBUILD_BLUEZ=OFF -DBUILD_KINECT2=OFF (If using Kinect1 and not building Sensor Device BlueZ)

###Compile the Device Gateway with optimized version of poseFaceTracker for your cpu's architecture
cmake ../ -DUSE_SS2_INSTRUCTIONS=ON (if sse2 instructions set available)
cmake ../ -DUSE_SS4_INSTRUCTIONS=ON (if sse4 instructions set available)
cmake ../ -DUSE_AVX_INSTRUCTIONS=ON (if avx instructions set available)
#If not included, the tracker is extra slow.

make
sudo make install


########################
#                      #
# Running intructions  #
#                      #
########################

#The procedure to run the device gateway for Arduino sensors is as folows:
#Go to folder /wp3project/DeviceGateway/build/DeviceGatewayConsole and run:
sudo ./DGWConsole
#Hit 1-> wait for the message. if there are no more errors (Select Arduino from the list) hit 2 -> wait for the message then write:
motesArduino.cfg
#Then all the data should flood your screen. 
#In orer to send the data to the database go to folder /wp3project/DeviceGateway/build/libDGwClient and run (in our case it goes like this):
sudo ./sensorsToDB  http://141.85.151.162:5984/


#To test the Kinect and a simple processing function performed using OpenCV, follow the below steps:
# 1. Go to folder /wp3project/DeviceGateway/build/DeviceGatewayConsole and run:
sudo ./DGWConsole
# 2. Select "1 : Initialize DeviceGateway", you should see
#Listener on port 12345 
#DGwSockets: Thread run socket server started!!
# 3. Select "5 : Connect Kinect AVDevice to the DeviceGateway" and in the "Configuration File" prompt write
kinect.cfg
# 4. Select "12 : Start video streaming for AV device"
# 5. In the "KinectName" prompt write
kinect
# 5. In the "RGB streaming (1) or depth streaming (2)" prompt select 1
# You will see the following messages:
# DeviceGateway: AVDevice with ID = kinect queried to start streaming!
# AVDevices: AVKinect starting video streaming
# AVDevices: AVKinect with ID = Starting RGB thread!kinect started streaming with rate = 
# 25Hz and resolution of 640x480!
# Kinect started RGB streaming

# You can stop streaming by selecting "13 : Stop video streaming for AV device"
# Tyoe the name "kinect" and select RGB streaming (1) 

# 6. To test that kinect can send video, from another console go to folder
# /wp3project/DeviceGateway/build/VisualProcessing and run:
./simple_visual
# You should see a window with the image from Kinect

# 7a. To start the face tracker, from another console go to  folder
# /wp3project/DeviceGateway/build/FaceProcessing and run:
./face_processing http://192.38.55.74:5984/visual/
# You should have created a "visual" database in CouchDB
# If not, to disable sending messages to CouchDB, use the command
./face_processing localhost
# You should see a window tracking faces from Kinect

# 7b. Alternatively, to start the new face tracker (poseFaceTracker), ignore step 7a and from another console go to  folder
# /wp3project/DeviceGateway/build/poseFaceProcessing and run:
./pose_face_processing http://192.38.55.74:5984/visual/
# You should have created a "visual" database in CouchDB
# If not, to disable sending messages to CouchDB, use the command
./pose_face_processing localhost
# You should see a window tracking faces from Kinect

#testing the antidote 
cd /build/DeviceGatewayConsole
./DGwConsoleCA
#in another tab 
cd /antidote-master/apps
./test_healthd.py

########################
#                      #
# Project directories  #
#                      #
########################

Accelerometer
#class definition and implementation for the Accelerometer DGw client (processing component). 
#Connects to the DGw via sockets, receives accelerometer data processes the data into IMA, ISA, steps and stores the results in CouchDB.
                
DGwClientConsole
#example console to run DGw clients

DeviceGatewayConsole
#contains console applications to run DGw with Arduino (DeviceGatewayConsoleArduino.cpp), with WaspMotes (DeviceGatewayConsoleWM.cpp), 
#with Kinect (DGwKinectTest.cpp) and with prerecorded data (DGwConsolePrerecorded.cpp)
#configuration files are filled with json data, motesArduino.cfg for Arduino, motes.cfg for WaspMotes.

curl_helper
#curl_helper library

includeDir
#contains the interface classes, functions and structures definitions needed in all the DGw libraries

json_spirit
#library for JSON message parsing, included from the json_spirit project https://github.com/png85/json_spirit

json_spirit_dgw
#library for parsing dgw structures (defined in includeDir/DGWDefs.h into json strings). Needed for the socket interface and for reading of the configuration files.

libAVDevices
#contains the class definitions and implementations of AV Devices for the DGw: example AV device (libAVDeviceExample.h and libAVDeviceExample.cpp) 
and kinect AV device (libAVKinect.h and libAVKinect.cpp). freenectwrp.hpp is a wrapper for the libfreenect driver.

libDGwClient
#contains the class definitions and implementations of DGw clients utilizing the measurement and streaming data via sockets. 
#libDGwClient.h and libDGwClient.cpp represent example implementations of the DGw client
#libDGwClientSensors.h and libDGwClientSensors.cpp accept sensor measurements (temperature, humidity, gasses, liminocity...) and stores them into CouchDB.
#sensorsToDB.cpp is a console application to run the DGwClientSensors.

libDGwSockets
#contains the class definitions and implementations socket interface between the Device Gateway (socket server) and the DGw clients (socket clients).
#the data is transfered as json strings using the json_spirit_dgw library

libDeviceGateway
#contains the class definitions and implementations of the device gateway.
#The DGW can connect multiple sensor devices and multiple AV devices on one end and multiple clients (processing algorithms) via sockets on the other end.

libSensorDevices
#contains the class definitions and implementations of the sensor devices.
#\libSensorDeviceArduino subdirectory contains the implementation of the Arduino sensor devices
#\libSensorDeviceWaspMote subdirectory contains the implementation of the WaspMote sensor devices
#\libSensorDeviceExample subdirectory contains an example sensod devices implementation
#\libSensorDevicePrerecorded subdirectory contains the implementation of prerecorded data reading and streaming

time_funcs
#contains the c code time_funcs.c with three time-related function implementations: 
#1. getMillis to get the UNIX time in milliseconds
#2. millis2string to transform the UNIX time into string
#3. string2millis to transform the timestring into UNIX time

libDeviceGatewayJavaWrapper
#contains the DeviceGateway library wrapper for JAVA developed using SWIG
