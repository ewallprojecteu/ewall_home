#!/bin/bash

. /lib/lsb/init-functions

PIDSLEEP=-1
MACOUT=$(cat ../noninMAC.txt)
echo "MACOUT =" $MACOUT
DAEMON_OPTS=""
DAEMON_USER=root
PIDDGW=/var/run/dgw_all.pid
PIDHEALTH=/var/run/test_healthd.pid
PIDNONIN=/var/run/nonin_agent.pid
DIRDGW=build/DeviceGatewayConsole
DIRHEALTH=antidote-master/apps
DIRNONIN=build/DeviceGatewayConsole

MON=$(xrandr --query | grep " connected")
echo $MON
KIN=$(lsusb | grep "ID 045e:02d[8-9]")
echo $KIN
if [ -z "$MON" ] || [ -z "$KIN" ]; then
	RUNDGW="mkfifo /tmp/dgw-tail-input; tail -f /tmp/dgw-tail-input | sudo ./dgw_console_all 3"
else 
	RUNDGW="mkfifo /tmp/dgw-tail-input; tail -f /tmp/dgw-tail-input | sudo ./dgw_console_all 7"
fi

RUNHEALTH="test_healthd.py"
RUNNONIN="mkfifo /tmp/nonin-tail-input; tail -f /tmp/nonin-tail-input | sudo ./nonin_ieee_agent $MACOUT"

cleanup ()
{
    sudo killall test_healthd.py -SIGTERM 2>/dev/null
    sudo killall nonin_ieee_agent -SIGTERM 2>/dev/null
    sudo killall dgw_console_all -SIGTERM 2>/dev/null
    sudo killall test_healthd.py -SIGKILL 2>/dev/null
    sudo killall nonin_ieee_agent -SIGKILL 2>/dev/null
    sudo killall dgw_console_all -SIGKILL 2>/dev/null
    sudo kill -SIGTERM $PIDSLEEP 2>/dev/null
    sudo pkill -f dgw-tail-input 2>/dev/null
    sudo pkill -f nonin-tail-input 2>/dev/null
}

cleanup

trap trap_ctrlc SIGINT SIGTERM SIGKILL SIGHUP
wait

trap_ctrlc ()
{
    echo "Ctrl-C caught...performing clean up"
    log_daemon_msg "Stopping dgw_all"
    start-stop-daemon --stop --pidfile $PIDDGW --retry=TERM/1/KILL/5
    log_end_msg $?
    log_daemon_msg "Stopping test_healthd"
    start-stop-daemon --stop --pidfile $PIDHEALTH --retry 10 --signal TERM
    log_end_msg $?
    log_daemon_msg "Stopping nonin_agent"
    start-stop-daemon --stop --pidfile $PIDNONIN --retry 10 --signal TERM
    log_end_msg $?
    cleanup
    echo "DGw script exiting..."
    exit 2
}

log_daemon_msg "Starting dgw_all"
start-stop-daemon --start -d $DIRDGW --background --pidfile $PIDDGW --make-pidfile --user $DAEMON_USER --chuid $DAEMON_USER --exec /bin/bash -- -c "$RUNDGW" -- $DAEMON_OPTS
log_end_msg $?

sleep 1
log_daemon_msg "Starting test_healthd"
start-stop-daemon --start -d $DIRHEALTH --background --pidfile $PIDHEALTH --make-pidfile --user $DAEMON_USER --chuid $DAEMON_USER --startas $RUNHEALTH -- $DAEMON_OPTS
log_end_msg $?

sleep 1
log_daemon_msg "Starting nonin_agent"
start-stop-daemon --start -d $DIRNONIN --background --pidfile $PIDNONIN --make-pidfile --user $DAEMON_USER --chuid $DAEMON_USER --exec /bin/bash -- -c "$RUNNONIN" -- $DAEMON_OPTS
log_end_msg $?

sleep infinity &
PIDSLEEP=$!
wait
