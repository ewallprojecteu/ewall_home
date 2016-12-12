#!/bin/bash

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

echo $sudoPass | sudo -S apt-get purge hud

wget http://serv2.radio.pub.ro/gitlab/pilots/ewallhome/raw/master/ewallhome.tar.gz?private_token=6uxej1RJNS4DjNQiXCRD -O ewallhome.tar.gz
mkdir temp
tar -zxvf ewallhome.tar.gz -C temp/
rm ewallhome.tar.gz

cd temp
#removing files that must not be updated
rm noninMAC.txt
rm pom.xml
if [ ! -f ../timeserv.cfg ]; then
   cp timeserv.cfg ../
fi
find . -name '*.cfg' -delete
rm -r local-platform/logs
rm local-platform/config.properties
rm local-platform/phhue-config.properties
cp -a . ../
cd ..

#remove temporary file
rm -r temp
find . -name 'CMakeCache.txt' -delete

#add offset field in motesArduino.cfg if it does not exist
if grep "\"offset\"" DeviceGateway/DeviceGatewayConsole/motesArduino.cfg > /dev/null;
then
    echo "offset exists"
else
    echo "offset not existing"
    sed -i 's/\"accuracy\": \(.*\)/\"accuracy\": \1,\n\t\t\t\"offset\": 0/' DeviceGateway/DeviceGatewayConsole/motesArduino.cfg
fi

#navigate to DeviceGateway and rebuild solution
mkdir DeviceGateway/build
cd DeviceGateway/build/
cmake ../
make
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

cd .. #now, in the ewallhome directory
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
#echo $sudoPass | sudo -S mkdir $HOME/.config/autostart
echo $sudoPass | sudo -S cp fitbit-start.desktop $HOME/.config/autostart/
echo $sudoPass | sudo -S rm fitbit-start.desktop
echo $sudoPass | sudo -S chmod +x $HOME/.config/autostart/fitbit-start.desktop
echo $sudoPass | sudo -S cp usbreset $HOME
echo $sudoPass | sudo -S chmod +x $HOME/usbreset
###############################################################

cd ../ #now, in the ewallhome directory
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

#echo $sudoPass | sudo -S mkdir $HOME/.config/autostart
echo $sudoPass | sudo -S cp xdgw-start.desktop $HOME/.config/autostart/
echo $sudoPass | sudo -S rm xdgw-start.desktop
echo $sudoPass | sudo -S chmod +x $HOME/.config/autostart/xdgw-start.desktop
echo $sudoPass | sudo -S teamviewer passwd ewall13
###############################################################

#synchronize datetime
echo $sudoPass | sudo -S apt-get install ntp ntpdate -y --force-yes
echo $sudoPass | sudo -S service ntp stop
linets=$(head -n 1 timeserv.cfg)
echo $sudoPass | sudo -S ntpdate -s $linets
echo $sudoPass | sudo -S service ntp start

#restart local-platform to enforce changes
echo $sudoPass | sudo -S service local-platform restart

echo $sudoPass | sudo -S cp kiosk-start.desktop $HOME/.config/autostart/
echo $sudoPass | sudo -S rm kiosk-start.desktop
echo $sudoPass | sudo -S chmod +x $HOME/.config/autostart/kiosk-start.desktop
###############################################################

#update grub to load 3.19.0-25 kernel
echo $sudoPass | sudo -S sed -i 's/GRUB_DEFAULT=0/GRUB_DEFAULT="Advanced options for Ubuntu>Ubuntu, with Linux 3.19.0-25-generic"/g' /etc/default/grub;
echo $sudoPass | sudo -S update-grub

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

echo $sudoPass | sudo -S hciconfig all piscan 

#you can rebuild local-platform if you need here

#logging out to enforce DGw changes
gnome-session-quit --no-prompt
