#!/bin/bash
for pIDi in $(pgrep "fitbit_sync") ; do
    if [ "$$" -ne "$pIDi" ]; then
    	sudo kill -9 $pIDi
    fi	
done

for pID in $(pgrep "usbreset") ; do
    sudo kill -9 $pID
done

 while true; do
DBS2=`lsusb | grep fb01 | cut -d' ' -f 2`
DBS4=`lsusb | grep fb01 | cut -d' ' -f 4`
DBS4=`echo $DBS4 | cut -d':' -f 1`
COMMAND_RESET="sudo ./usbreset /dev/bus/usb/$DBS2/$DBS4"
COMMAND_SYNC="sudo galileo --force --no-dump sync"
COMMAND_STOP="sudo stop galileo"
eval $COMMAND_STOP
eval $COMMAND_RESET
eval $COMMAND_SYNC
#wait for one minute
sleep 1m

#synchronize datetime
sudo service ntp stop
linets=$(head -n 1 ../timeserv.cfg)
sudo ntpdate -s $linets
sudo service ntp start
 done
