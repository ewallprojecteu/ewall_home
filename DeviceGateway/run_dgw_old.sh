#!/bin/bash

sudo killall test_healthd.py -SIGKILL 2>/dev/null
sudo killall nonin_ieee_agent -SIGKILL 2>/dev/null
sudo killall dgw_console_all -SIGKILL 2>/dev/null
sudo pkill -f dgw-tail-input 2>/dev/null
sudo pkill -f nonin-tail-input 2>/dev/null

trap trap_ctrlc SIGINT SIGTERM SIGKILL SIGHUP
wait

trap_ctrlc ()
{
    echo "Ctrl-C caught...performing clean up"
    sudo killall test_healthd.py -SIGTERM 2>/dev/null
    sudo killall nonin_ieee_agent -SIGTERM 2>/dev/null
    sudo killall dgw_console_all -SIGTERM 2>/dev/null
    sudo killall test_healthd.py -SIGKILL 2>/dev/null
    sudo killall nonin_ieee_agent -SIGKILL 2>/dev/null
    sudo killall dgw_console_all -SIGKILL 2>/dev/null
    sudo pkill -f dgw-tail-input 2>/dev/null
    sudo pkill -f nonin-tail-input 2>/dev/null
    echo "DGw script exiting..."
    exit 2
}

MACOUT=$(cat ../noninMAC.txt)
cd build/DeviceGatewayConsole
mkfifo /tmp/dgw-tail-input; tail -f /tmp/dgw-tail-input | sudo ./dgw_console_all 7 &
cd ../../antidote-master/apps
sleep 1
sudo ../../antidote-master/apps/test_healthd.py &
sleep 1
cd ../../build/DeviceGatewayConsole 
mkfifo /tmp/nonin-tail-input; tail -f /tmp/nonin-tail-input | sudo ./nonin_ieee_agent $MACOUT &

wait

