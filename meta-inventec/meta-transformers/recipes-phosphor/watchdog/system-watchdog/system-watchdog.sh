#!/bin/sh
source /etc/default/obmc/watchdog/system-watchdog

if [ -z $1 ]; then
    echo "No input"
    exit 0
fi

if [ $1 == "start" ]; then
    echo "Start system-watchdog on $DEVICE"
    /sbin/watchdog -T $TIMEOUT -t $INTERVAL -F $DEVICE
elif [ $1 == "stop" ]; then
    echo "Stop watchdog $DEVICE"
    echo V > $DEVICE
fi
