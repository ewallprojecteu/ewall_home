#!/bin/bash

sudo killall face_processing -SIGKILL -SIGTERM 2>/dev/null
sudo killall sensorsToDB -SIGKILL -SIGTERM 2>/dev/null
sudo killall scrActKinect -SIGKILL -SIGTERM 2>/dev/null

trap trap_ctrlc SIGINT SIGTERM SIGKILL SIGHUP
wait

trap_ctrlc ()
{
    echo "Ctrl-C caught...performing clean up"
    sudo killall face_processing -SIGTERM -SIGTERM 2>/dev/null
    sudo killall sensorsToDB -SIGTERM -SIGTERM 2>/dev/null
    sudo killall scrActKinect -SIGTERM -SIGTERM 2>/dev/null
    sudo killall face_processing -SIGKILL -SIGTERM 2>/dev/null
    sudo killall sensorsToDB -SIGKILL -SIGTERM 2>/dev/null
    sudo killall scrActKinect -SIGKILL -SIGTERM 2>/dev/null
    echo "DGw clients script exiting..."
    exit 2
}

cd build/FaceProcessing 
sudo ./face_processing http://localhost:5984/visual 10 &
cd ../VisualProcessing
sleep 1
sudo ./scrActKinect &
cd ../libDGwClient
sleep 1
sudo ./sensorsToDB http://localhost:5984/ &

wait
