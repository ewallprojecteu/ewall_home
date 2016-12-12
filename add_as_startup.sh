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

chmod +x DeviceGateway/run_dgw.sh
chmod +x DeviceGateway/run_dclients.sh
chmod +x DeviceGateway/run_dgw_old.sh
chmod +x DeviceGateway/run_dclients_old.sh
chmod +x DeviceGateway/dgw-daemon
chmod +x DeviceGateway/dgw-clients-daemon
chmod +x xdgw-restart

CURDIR=$(echo `pwd`)
CURDIR=$CURDIR/DeviceGateway
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
