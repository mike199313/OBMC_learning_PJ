#!/bin/bash
HWMON_MAX=100
FAN_MAX=3
FAN_DRIVER_NAME="max31790"
COLDPLATE_MAX=2
COLDPLATE_DRIVER_NAME="emc2305"
#duty:50%
DEF_PWM=127
#duty:60%
DEF_COLDPLATEPWM=153
FAN_CONTROLLER_NUM=2
found=0
COLDPLATE_CONTROLLER_NUM=1
COLDPLATEfound=0
for num in $(seq 0 $HWMON_MAX) ; do
        name=`cat /sys/class/hwmon/hwmon$num/name`
        if [ $name == $FAN_DRIVER_NAME ]; then
            for i in $(seq 1 $FAN_MAX) ; do
                echo $DEF_PWM > /sys/class/hwmon/hwmon$num/pwm$i
            done
            let found=found+1
            echo "fan driver found $found :$name "
        elif [ $name == $COLDPLATE_DRIVER_NAME ]; then
            for i in $(seq 1 $COLDPLATE_MAX) ; do
                echo 255 > /sys/class/hwmon/hwmon$num/pwm$i
            done
            sleep 2
            COLD_ORI_IN=$(cat /sys/class/hwmon/hwmon$num/fan*input)
            for i in $(seq 1 $COLDPLATE_MAX) ; do
                echo $DEF_COLDPLATEPWM > /sys/class/hwmon/hwmon$num/pwm$i
            done
            sleep 2
            COLD_NEW_IN=$(cat /sys/class/hwmon/hwmon$num/fan*input)
            if [[ $COLD_ORI_IN != $COLD_NEW_IN ]]; then
                echo "Switch to rmc cold plate config"
                cat /usr/share/swampd/config_rmc.json > /usr/share/swampd/config.json
            fi
            let COLDPLATEfound=COLDPLATEfound+1
            echo "coldplate driver found :$name "
        fi
         
        if [ $found -eq $FAN_CONTROLLER_NUM ] && [ $COLDPLATEfound -eq $COLDPLATE_CONTROLLER_NUM ]; then
            echo "fan def setting ok"
            exit 0
        fi
done
echo "$name is not a fan driver name"
exit 1

