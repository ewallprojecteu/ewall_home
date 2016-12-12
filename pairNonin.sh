#!/bin/sh

DATA=$(hcitool scan | grep 'Nonin')
if [ -z "$DATA" ];
then
	echo "Nonin device not discovered, try again!"
else
	echo "Nonin device discovered."
	echo "==>" $DATA
	MAC=$(echo $DATA | cut -d ' ' -f1)
	SN=$(echo $DATA | grep -o '[[:digit:]]*$')
	echo "Trying to pair Nonin"
	bluez-test-device remove $MAC 2>/dev/null
	PAIR=$(echo $SN | sudo -S bluez-simple-agent hci0 $MAC)
	echo "==>" $PAIR
	PSUCCESS=$(echo $PAIR | grep 'New device')
	if [ -z "$PSUCCESS" ];
	then 
		echo "Pairing failed, try again! :("
	else
		echo "Pairing successfull. :)"
		echo $MAC > noninMAC.txt
		MACOUT=$(cat noninMAC.txt)
		#echo $MACOUT
	fi
fi
