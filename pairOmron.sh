#!/bin/sh

DATA=$(hcitool scan | grep 'HEM')
if [ -z "$DATA" ];
then
	echo "Omron device not discovered, try again!"
else
	echo "Omron device discovered."
	echo "==>" $DATA
	MAC=$(echo $DATA | cut -d ' ' -f1)
	echo "Trying to pair Omron"
	bluez-test-device remove $MAC 2>/dev/null
	PAIR=$(bluez-simple-agent hci0 $MAC)
	echo "==>" $PAIR
	PSUCCESS=$(echo $PAIR | grep 'New device')
	if [ -z "$PSUCCESS" ];
	then 
		echo "Pairing failed, try again! :("
	else
		echo "Pairing successfull. :)"
	fi
fi
