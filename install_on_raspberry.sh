#!/bin/bash

#to run
#chmod u+x install_dgw.sh
#./install_dgw.sh
#insert su password when needed
if [ "$USER" == "root" ]
then
    echo "This script should not be run with the sudo command! Exiting..."
    exit 0
fi

echo -n "[sudo] password for $USER: "
IFS= read -rs sudoPass
sudo -k
if sudo -lS &> /dev/null << EOF
$sudoPass
EOF
then
    echo
else
    echo
    echo 'Wrong password.'
    exit 0
fi

echo $sudoPass | sudo -S apt-get update -y
echo $sudoPass | sudo -S apt-get install libegl1-mesa libegl1-mesa-drivers libgl1-mesa-dri libglapi-mesa -y
echo $sudoPass | sudo -S apt-get install python-software-properties -y
echo $sudoPass | sudo -S apt-get install software-properties-common -y

echo $sudoPass | sudo -S sudo apt-get autoremove -y

echo $sudoPass | sudo -S apt-get install autoconf automake build-essential cmake couchdb doxygen freeglut3-dev g++ git-core libboost-all-dev libcurl4-gnutls-dev libdbus-1-dev libdbus-glib-1-2 libdbus-glib-1-dev libglib2.0-dev libopencv-dev libtool libusb-1.0-0-dev libxi-dev libxmu-dev libxrandr-dev libudev-dev libxtst-dev mesa-common-dev mesa-utils openjdk-7-jdk openjdk-7-jre-lib pkg-config vnstat libturbojpeg1-dev -f -y

echo $sudoPass | sudo -S dpkg-reconfigure tzdata
echo $sudoPass | sudo -S apt-get install ntp ntpdate -y
#echo $sudoPass | sudo -S apt-get install python-ntplib -y
echo $sudoPass | sudo -S apt-get install fake-hwclock -y

#synchronize datetime
echo $sudoPass | sudo -S service ntp stop
linets=$(head -n 1 timeserv.cfg)
echo $sudoPass | sudo -S ntpdate -s $linets
echo $sudoPass | sudo -S service ntp start

echo $sudoPass | sudo -S apt-get install python-usb python-pip -y
echo $sudoPass | sudo -S pip install galileo
echo $sudoPass | sudo -S galileo
###############################################################

#### INSTALL LIBUSB ###########################################
cd Prerequisites/libusb-1.0.20/
./configure
make
echo $sudoPass | sudo -S make install
echo $sudoPass | sudo -S ldconfig /usr/local/lib64/
echo $sudoPass | sudo -S ldconfig
###############################################################

#### INSTALL LIFREENECT2 ######################################
cd ..
cd libfreenect2/depends
sh install_ubuntu.sh

cd ..
#echo $sudoPass | sudo -S rm -r build
mkdir build
cd build
cmake ..
make
echo $sudoPass | sudo -S make install
echo $sudoPass | sudo -S ldconfig /usr/local/lib64/
echo $sudoPass | sudo -S ldconfig
###############################################################

#### INSTALL LIFREENECT #######################################
cd ../..
cd libfreenect
#echo $sudoPass | sudo -S rm -r build
mkdir build
cd build
cmake -L .. #-DBUILD_REDIST_PACKAGE=OFF
make
echo $sudoPass | sudo -S make install
echo $sudoPass | sudo -S ldconfig /usr/local/lib64/
echo $sudoPass | sudo -S ldconfig
mkdir $HOME/.libfreenect
cp audios.bin $HOME/.libfreenect/
echo $sudoPass | sudo -S adduser $USER video

echo $sudoPass | sudo -S rm tmp.txt
echo "# ATTR{product}==\"Xbox NUI Motor\"" >> tmp.txt
echo "SUBSYSTEM==\"usb\", ATTR{idVendor}==\"045e\", ATTR{idProduct}==\"02b0\", MODE=\"0666\"" >> tmp.txt
echo "# ATTR{product}==\"Xbox NUI Audio\"" >> tmp.txt
echo "SUBSYSTEM==\"usb\", ATTR{idVendor}==\"045e\", ATTR{idProduct}==\"02ad\", MODE=\"0666\"" >> tmp.txt
echo "# ATTR{product}==\"Xbox NUI Camera\"" >> tmp.txt
echo "SUBSYSTEM==\"usb\", ATTR{idVendor}==\"045e\", ATTR{idProduct}==\"02ae\", MODE=\"0666\"" >> tmp.txt
echo "# ATTR{product}==\"Xbox NUI Motor\"" >> tmp.txt
echo "SUBSYSTEM==\"usb\", ATTR{idVendor}==\"045e\", ATTR{idProduct}==\"02c2\", MODE=\"0666\"" >> tmp.txt
echo "# ATTR{product}==\"Xbox NUI Motor\"" >> tmp.txt
echo "SUBSYSTEM==\"usb\", ATTR{idVendor}==\"045e\", ATTR{idProduct}==\"02be\", MODE=\"0666\"" >> tmp.txt
echo "# ATTR{product}==\"Xbox NUI Motor\"" >> tmp.txt
echo "SUBSYSTEM==\"usb\", ATTR{idVendor}==\"045e\", ATTR{idProduct}==\"02bf\", MODE=\"0666\"" >> tmp.txt
echo $sudoPass | sudo -S cp tmp.txt /etc/udev/rules.d/51-kinect.rules
echo $sudoPass | sudo -S rm tmp.txt
###############################################################

#### INSTALL ANTIDOTE (BLUEZ CA DEVICES) ######################
cd ../../../Prerequisites/bluez-4.101
./install_bluez_raspberry.sh
echo $sudoPass | sudo -S systemctl daemon-reload
echo $sudoPass | sudo -S /etc/init.d/bluetooth restart

cd ../../DeviceGateway
cd antidote-master
./autogen.sh
./configure
make
echo $sudoPass | sudo -S make install
echo $sudoPass | sudo -S cp apps/resources/healthd.conf /etc/dbus-1/system.d 
#echo $sudoPass | sudo -S service dbus restart
echo $sudoPass | sudo -S /etc/init.d/bluetooth restart
echo $sudoPass | sudo -S hciconfig hci0 up
echo $sudoPass | sudo -S hciconfig hci0 piscan
echo $sudoPass | sudo -S ldconfig /usr/local/lib64/
echo $sudoPass | sudo -S ldconfig

echo $sudoPass | sudo -S apt-get install libglib2.0-dev
###############################################################

### COMPILE AND INSTALL DGW WITH KINECT2 and BLUEZ OPTIONS ####
cd ..
#echo $sudoPass | sudo -S rm -r build
mkdir build
cd build
cmake ../ -DBUILD_BLUEZ=ON -DBUILD_KINECT2=ON
make
#echo $sudoPass | sudo -S make install
make install
echo $sudoPass | sudo -S ldconfig /usr/local/lib64/
echo $sudoPass | sudo -S ldconfig

#make running scripts runnable
cd .. #to DeviceGateway directory
chmod +x run_dgw.sh
chmod +x run_dclients.sh
chmod +x run_dgw_old.sh
chmod +x run_dclients_old.sh
chmod +x dgw-daemon
chmod +x dgw-clients-daemon
chmod +x ../xdgw-restart

CURDIR=$(echo `pwd`)
cp $CURDIR/dgw-daemon dgw-daemon-tmp
cp $CURDIR/dgw-clients-daemon dgw-clients-daemon-tmp
sed -i "/DIR=/c\DIR=$CURDIR" dgw-daemon-tmp
sed -i "/DIR=/c\DIR=$CURDIR" dgw-clients-daemon-tmp
echo $sudoPass | sudo -S cp dgw-daemon-tmp /etc/init.d/dgw-daemon
echo $sudoPass | sudo -S cp dgw-clients-daemon-tmp /etc/init.d/dgw-clients-daemon
echo $sudoPass | sudo -S rm dgw-daemon-tmp
echo $sudoPass | sudo -S rm dgw-clients-daemon-tmp
echo $sudoPass | sudo -S chmod +x /etc/init.d/dgw-daemon
echo $sudoPass | sudo -S chmod +x /etc/init.d/dgw-clients-daemon

cd .. #to wp3project directory
#build fitbit_sync
cd fitbit_sync
cc usbreset.c -o usbreset
chmod u+x fitbit_sync.sh
curDir=$(pwd)

echo "[Desktop Entry]" > fitbit-start.desktop
echo "Type=Application" >> fitbit-start.desktop
echo "Exec=bash -c 'echo $sudoPass | sudo -S $curDir/fitbit_sync.sh'" >> fitbit-start.desktop
echo "Hidden=false" >> fitbit-start.desktop
echo "NoDisplay=false" >> fitbit-start.desktop
echo "X-GNOME-Autostart-enabled=true" >> fitbit-start.desktop
echo "X-GNOME-Autostart-Delay=30" >> fitbit-start.desktop
echo "Name=FitbitSyncStartup" >> fitbit-start.desktop
echo "Comment=FitbitSync startup script" >> fitbit-start.desktop

#make fitbit_sync run at autostart
echo $sudoPass | sudo -S mkdir $HOME/.config/autostart
echo $sudoPass | sudo -S cp fitbit-start.desktop $HOME/.config/autostart/
echo $sudoPass | sudo -S rm fitbit-start.desktop
echo $sudoPass | sudo -S chmod +x $HOME/.config/autostart/fitbit-start.desktop
echo $sudoPass | sudo -S cp usbreset $HOME
echo $sudoPass | sudo -S chmod +x $HOME/usbreset
###############################################################

cd .. #to wp3project directory
#make application run at startup
echo $sudoPass | sudo -S cp xdgw-restart $HOME/.xdgw-start
echo $sudoPass | sudo -S chmod +x $HOME/.xdgw-start

echo "[Desktop Entry]" > xdgw-start.desktop
echo "Type=Application" >> xdgw-start.desktop
echo "Exec=bash -c 'echo $sudoPass | sudo -S $HOME/.xdgw-start'" >> xdgw-start.desktop
echo "Hidden=false" >> xdgw-start.desktop
echo "NoDisplay=false" >> xdgw-start.desktop
echo "X-GNOME-Autostart-enabled=true" >> xdgw-start.desktop
echo "X-GNOME-Autostart-Delay=30" >> xdgw-start.desktop
echo "Name=DGwStartup" >> xdgw-start.desktop
echo "Comment=DGw startup script" >> xdgw-start.desktop

echo $sudoPass | sudo -S mkdir $HOME/.config/autostart
echo $sudoPass | sudo -S cp xdgw-start.desktop $HOME/.config/autostart/
echo $sudoPass | sudo -S rm xdgw-start.desktop
echo $sudoPass | sudo -S chmod +x $HOME/.config/autostart/xdgw-start.desktop
###############################################################

filename="version.txt"
version=$(cat $filename)

echo "Version read from version.txt file: $version"

DB_COMMANDDEL="curl -s -X DELETE http://127.0.0.1:5984/version"
RESPONSEDEL=`$DB_COMMANDDEL`
DB_COMMAND="curl -s -X PUT http://127.0.0.1:5984/version"
RESPONSE=`$DB_COMMAND`
echo $RESPONSE

curl -s -i \
-H "Accept: application/json" \
-H "Content-Type:application/json" \
-X PUT --data '{"version":"'"$version"'"}' "http://127.0.0.1:5984/version/version" > /dev/null

gnome-session-quit --no-prompt
#etc.
