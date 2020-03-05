#!/bin/bash

## Synchronizes a node with a parent node and measures clock drift every 30
## seconds for 24 hours and prints the current time and drift.
##
## Takes in 1 parameter, the IP of the parent node. The parent node must be
## running the NTP daemon (/etc/init.d/ntpd start).

CHECK_PERIOD_NS=30000000000
TOTAL_EXP_TIME_NS=$((24*60*60*1000000000))

# Sync
echo "Synchronizing..."
ntpdate -b $1

echo "Measuring clock drift..."
startTimeNs=$(date +%s%N)
endTimeNs=$((startTimeNs+TOTAL_EXP_TIME_NS))
currTimeNs=$startTimeNs
while [ $currTimeNs -le $endTimeNs ]
do
    loopEndTime=$((currTimeNs+CHECK_PERIOD_NS))
    driftSeconds=$(output=$(ntpdate -d 10.0.0.4 2>&1) && echo $output | awk '{print $119}')
    echo $currTimeNs","$driftSeconds

    # Wait until loop time elapses.
    while [ $currTimeNs -le $loopEndTime ]
    do
        currTimeNs=$(date +%s%N)
    done
done

