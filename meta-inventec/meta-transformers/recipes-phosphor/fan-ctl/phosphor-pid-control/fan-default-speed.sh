#!/bin/bash
HWMON_MAX=15
FAN_MAX=7

for num in $(seq 0 $HWMON_MAX) ; do

        name=`cat /sys/class/hwmon/hwmon$num/name`
        if [ $name == "aspeed_pwm_tachometer" ]; then
            echo "fan driver found"
			
			for i in $(seq 0 $FAN_MAX) ; do
				echo 153 > /sys/class/hwmon/hwmon$num/pwm$i
			done
           exit 0
        fi

done
echo "$name is not a fan driver"
exit 1

