#!/bin/bash

. /lib/lsb/init-functions

PIDSLEEP=-1
DAEMON_OPTS=""
DAEMON_USER=root
PIDFACEPROC=/var/run/face_proc.pid
PIDSENS2DB=/var/run/sensors2db.pid
PIDSRCACTKINECT=/var/run/srcActKinect.pid
DIRFACEPROC=build/FaceProcessing
DIRSENS2DB=build/libDGwClient
DIRSRCACTKINECT=build/VisualProcessing
RUNFACEPROC='face_processing http://localhost:5984/visual 10'
RUNSENS2DB='sensorsToDB http://localhost:5984/'
RUNSRCACTKINECT='scrActKinect'

cleanup ()
{
    sudo killall face_processing -SIGTERM -SIGTERM 2>/dev/null
    sudo killall sensorsToDB -SIGTERM -SIGTERM 2>/dev/null
    sudo killall scrActKinect -SIGTERM -SIGTERM 2>/dev/null
    sudo killall face_processing -SIGKILL -SIGTERM 2>/dev/null
    sudo killall sensorsToDB -SIGKILL -SIGTERM 2>/dev/null
    sudo killall scrActKinect -SIGKILL -SIGTERM 2>/dev/null
    sudo kill -SIGTERM $PIDSLEEP 2>/dev/null
}

cleanup

trap trap_ctrlc SIGINT SIGTERM SIGKILL SIGHUP
wait

trap_ctrlc ()
{
    echo "Ctrl-C caught...performing clean up"
    log_daemon_msg "Stopping face_processing"
    start-stop-daemon --stop --pidfile $PIDFACEPROC --retry=TERM/1/KILL/5
    log_end_msg $?
    log_daemon_msg "Stopping sensorsToDB"
    start-stop-daemon --stop --pidfile $PIDSENS2DB --retry 10 --signal TERM
    log_end_msg $?
    log_daemon_msg "Stopping srcActKinect"
    start-stop-daemon --stop --pidfile $PIDSRCACTKINECT --retry 10 --signal TERM
    log_end_msg $?
    cleanup
    echo "DGw clients script exiting..."
    exit 2
}

#log_daemon_msg "Starting face_processing"
#start-stop-daemon --start -d $DIRFACEPROC --background --pidfile $PIDFACEPROC --make-pidfile --user $DAEMON_USER --chuid $DAEMON_USER --startas $RUNFACEPROC -- $DAEMON_OPTS
#log_end_msg $?

#sleep 1
log_daemon_msg "Starting sensorsToDB"
start-stop-daemon --start -d $DIRSENS2DB --background --pidfile $PIDSENS2DB --make-pidfile --user $DAEMON_USER --chuid $DAEMON_USER --startas $RUNSENS2DB -- $DAEMON_OPTS
log_end_msg $?

#sleep 1
log_daemon_msg "Starting srcActKinect"
start-stop-daemon --start -d $DIRSRCACTKINECT --background --pidfile $PIDSRCACTKINECT --make-pidfile --user $DAEMON_USER --chuid $DAEMON_USER --startas $RUNSRCACTKINECT -- $DAEMON_OPTS
log_end_msg $?

sleep infinity &
PIDSLEEP=$!
wait
