#!/bin/sh

plug_type=$1
DELAY=2 #unit sec
STABLE_CNT=20
STABLE_CHECK_PERIOD=1

tsod_drv_op() {

    op=$1
    ADDR_MIN=16
    ADDR_MAX=21
    BUS_MIN=0
    BUS_MAX=3
    
    bus=$BUS_MIN

    while [ $bus -le $BUS_MAX ]; do
        addr=$ADDR_MIN
        while [ $addr -le  $ADDR_MAX ]; do

            if [ "$op" = "new" ]
            then
                echo tsod $addr > /sys/bus/i2c/devices/i2c-$bus/new_device
            else
                echo $addr > /sys/bus/i2c/devices/i2c-$bus/delete_device
            fi
            let addr=addr+1
        done
        let bus=bus+1
    done

}
if [ "$plug_type" = "insert" ]; then
    # wait host is power on stably, then set mux back to bmc
    counter=0
    while [ $counter -lt $STABLE_CNT ]; do
        hostState=$(busctl get-property xyz.openbmc_project.State.Host /xyz/openbmc_project/state/host0  xyz.openbmc_project.State.Host CurrentHostState)
        if [[ "$hostState" == *"Run"* ]]; then
            ((counter++))
        else
            break
        fi
        sleep $STABLE_CHECK_PERIOD
    done
    if [ $counter -ge 20 ]; then
        echo "dimm insert action"
        echo "set i3c mux to bmc"
        gpioset `gpiofind I3C_MUX_SELECT`=1
        sleep $DELAY
        tsod_drv_op "new"

        systemctl restart xyz.openbmc_project.tsodsensor.service
    fi

elif [ "$plug_type" = "remove" ]; then
    echo "dimm remove action"
    systemctl stop xyz.openbmc_project.tsodsensor.service
    tsod_drv_op "delete"

    echo "set i3c mux to cpu"
    gpioset `gpiofind I3C_MUX_SELECT`=0

elif [ "$plug_type" = "init" ]; then

    echo "dimm init action"
    echo "set i3c mux to bmc"
    gpioset `gpiofind I3C_MUX_SELECT`=1
    sleep $DELAY
    tsod_drv_op "new"
else
    echo "unknow type"
    exit 1;
fi

exit 0;


