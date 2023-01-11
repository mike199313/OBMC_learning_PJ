#!/bin/sh


# Remove Riser and BP json config for entity-manager
ENTITY_MANAGER_CONIG_PATH="/usr/share/entity-manager/configurations"
ENTITY_MANAGER_RISER1A_JSON="$ENTITY_MANAGER_CONIG_PATH/riser1a.json"
ENTITY_MANAGER_RISER1B_JSON="$ENTITY_MANAGER_CONIG_PATH/riser1b.json"
ENTITY_MANAGER_RISER2_JSON="$ENTITY_MANAGER_CONIG_PATH/riser2.json"
ENTITY_MANAGER_BP1_JSON="$ENTITY_MANAGER_CONIG_PATH/bp1.json"
ENTITY_MANAGER_BP2_JSON="$ENTITY_MANAGER_CONIG_PATH/bp2.json"
CREATE_JSON_SH="/usr/sbin/create_json.sh"

rm -rf $ENTITY_MANAGER_RISER1A_JSON
rm -rf $ENTITY_MANAGER_RISER1B_JSON
rm -rf $ENTITY_MANAGER_RISER2_JSON
rm -rf $ENTITY_MANAGER_BP1_JSON
rm -rf $ENTITY_MANAGER_BP2_JSON

I2C_OFFSET=0
CURRENT_I2C=27

RISER1_MUX_I2C=21
RISER1_MUX_I2C_CH0=$(($CURRENT_I2C+1))
RISER1_MUX_I2C_CH1=$(($CURRENT_I2C+2))
RISER1_MUX_I2C_CH2=$(($CURRENT_I2C+3))
RISER1_MUX_I2C_CH3=$(($CURRENT_I2C+4))

# Wait for TCA9545 ready
ret=`i2cget -y -f $RISER1_MUX_I2C 0x71 0`
echo $ret

if [ -z $ret ]; then
    echo "device not ready or not exist, wait more second"
    # Give more second
    sleep 1
fi

echo pca9545 0x71 > /sys/bus/i2c/devices/i2c-$RISER1_MUX_I2C/new_device

if [ -d "/sys/bus/i2c/devices/i2c-$RISER1_MUX_I2C_CH0" ]
then
    RISER1_PRESENT="true"
    # PAC1934 U1
    echo pac1934 0x12 > /sys/bus/i2c/devices/i2c-$RISER1_MUX_I2C/new_device

    # Using pac1934 address to check is Riser1A or Riser 1B
    if [ -d "/sys/bus/i2c/devices/i2c-$RISER1_MUX_I2C/$RISER1_MUX_I2C-0012/hwmon" ]
    then
        echo "Riser1A detected"

        shunt_path=$(ls /sys/bus/i2c/devices/i2c-$RISER1_MUX_I2C/$RISER1_MUX_I2C-0012/hwmon/hwmon*/shunt_value)
        # shunt is 0.004 ohm
        echo 0 4000 > $shunt_path
        echo 1 4000 > $shunt_path
        echo 2 4000 > $shunt_path
        echo 3 4000 > $shunt_path

        echo 24c32 0x50 > /sys/bus/i2c/devices/i2c-$RISER1_MUX_I2C_CH2/new_device
        echo emc1403 0x3c > /sys/bus/i2c/devices/i2c-$RISER1_MUX_I2C_CH3/new_device

        # Create Riser1a json file
        bash $CREATE_JSON_SH riser1a $RISER1_MUX_I2C_CH0
        RISER1A_PATH="/sys/bus/i2c/devices/i2c-${RISER1_MUX_I2C_CH2}/${RISER1_MUX_I2C_CH2}-0050"
    else
        echo "Riser1B detected"
        echo 0x12 > /sys/bus/i2c/devices/i2c-$RISER1_MUX_I2C/delete_device

        echo 24c32 0x50 > /sys/bus/i2c/devices/i2c-$RISER1_MUX_I2C_CH0/new_device
        echo emc1403 0x3c > /sys/bus/i2c/devices/i2c-$RISER1_MUX_I2C_CH1/new_device

        # PAC1934 U1
        echo pac1934 0x12 > /sys/bus/i2c/devices/i2c-$RISER1_MUX_I2C_CH3/new_device
        shunt_path=$(ls /sys/bus/i2c/devices/i2c-$RISER1_MUX_I2C_CH3/$RISER1_MUX_I2C_CH3-0012/hwmon/hwmon*/shunt_value)
        # shunt is 0.004 ohm, 0.01 ohm
        echo 0 4000 > $shunt_path
        echo 1 10000 > $shunt_path

        # Create Riser1B json file
        bash $CREATE_JSON_SH riser1b $RISER1_MUX_I2C_CH0
        RISER1B_PATH="/sys/bus/i2c/devices/i2c-${RISER1_MUX_I2C_CH0}/${RISER1_MUX_I2C_CH0}-0050"
    fi

    CURRENT_I2C=$(($CURRENT_I2C+4))
else
    RISER1_PRESENT="false"
fi

RISER2_MUX_I2C=22
RISER2_MUX_I2C_CH0=$(($CURRENT_I2C+1))
RISER2_MUX_I2C_CH1=$(($CURRENT_I2C+2))
RISER2_MUX_I2C_CH2=$(($CURRENT_I2C+3))
RISER2_MUX_I2C_CH3=$(($CURRENT_I2C+4))

# Wait for TCA9545 ready
ret=`i2cget -y -f $RISER2_MUX_I2C 0x71 0`
echo $ret

if [ -z $ret ]; then
    echo "device not ready or not exist, wait more second"
    # Give more second
    sleep 1
fi

echo pca9545 0x71 > /sys/bus/i2c/devices/i2c-$RISER2_MUX_I2C/new_device
if [ -d "/sys/bus/i2c/devices/i2c-$RISER2_MUX_I2C_CH0" ]
then
    RISER2_PRESENT="true"

    # PAC1934 U1
    echo pac1934 0x12 > /sys/bus/i2c/devices/i2c-$RISER2_MUX_I2C/new_device
    shunt_path=$(ls /sys/bus/i2c/devices/i2c-$RISER2_MUX_I2C/$RISER2_MUX_I2C-0012/hwmon/hwmon*/shunt_value)
    # shunt is 0.004 ohm
    echo 0 4000 > $shunt_path
    echo 1 4000 > $shunt_path
    echo 2 4000 > $shunt_path
    echo 3 4000 > $shunt_path

    echo 24c32 0x50 > /sys/bus/i2c/devices/i2c-$RISER2_MUX_I2C_CH2/new_device
    echo emc1403 0x3c > /sys/bus/i2c/devices/i2c-$RISER2_MUX_I2C_CH3/new_device
    CURRENT_I2C=$(($CURRENT_I2C+4))

    # Create Riser2 json file
    bash $CREATE_JSON_SH riser2 $RISER2_MUX_I2C_CH0
    RISER2_PATH="/sys/bus/i2c/devices/i2c-${RISER2_MUX_I2C_CH2}/${RISER2_MUX_I2C_CH2}-0050"
else
    RISER2_PRESENT="false"
fi


echo "Riser1 $RISER1_PRESENT, Riser2 $RISER2_PRESENT"


echo "Init BP part"

BP1_MUX_U13_I2C=24
BP1_MUX_U13_I2C_CH0=$(($CURRENT_I2C+1))
BP1_MUX_U13_I2C_CH1=$(($CURRENT_I2C+2))
BP1_MUX_U13_I2C_CH2=$(($CURRENT_I2C+3))
BP1_MUX_U13_I2C_CH3=$(($CURRENT_I2C+4))
echo pca9545 0x71 > /sys/bus/i2c/devices/i2c-$BP1_MUX_U13_I2C/new_device
if [ -d "/sys/bus/i2c/devices/i2c-$BP1_MUX_U13_I2C_CH0" ]
then
    BP1_PRESENT="true"
    echo 24c32 0x50 > /sys/bus/i2c/devices/i2c-$BP1_MUX_U13_I2C/new_device

    # PAC1932 U7
    echo pac1934 0x12 > /sys/bus/i2c/devices/i2c-$BP1_MUX_U13_I2C/new_device
    shunt_path=$(ls /sys/bus/i2c/devices/i2c-$BP1_MUX_U13_I2C/$BP1_MUX_U13_I2C-0012/hwmon/hwmon*/shunt_value)
    # shunt is 0.0005, 0.01 ohm
    echo 0 500 > $shunt_path
    echo 1 10000 > $shunt_path

    echo pca9545 0x73 > /sys/bus/i2c/devices/i2c-$BP1_MUX_U13_I2C_CH0/new_device
    echo pca9545 0x73 > /sys/bus/i2c/devices/i2c-$BP1_MUX_U13_I2C_CH1/new_device
    echo pca9545 0x73 > /sys/bus/i2c/devices/i2c-$BP1_MUX_U13_I2C_CH2/new_device
    CURRENT_I2C=$(($CURRENT_I2C+16))

    # Create BP1 json file
    bash $CREATE_JSON_SH bp1 $BP1_MUX_U13_I2C_CH0
    BP1_NVME_BASIC_I2C=$BP1_MUX_U13_I2C_CH0
    BP1_PATH="/sys/bus/i2c/devices/i2c-${BP1_MUX_U13_I2C}/${BP1_MUX_U13_I2C}-0050"
else
    BP1_PRESENT="false"
    BP1_NVME_BASIC_I2C=0
fi

BP2_MUX_U13_I2C=25
BP2_MUX_U13_I2C_CH0=$(($CURRENT_I2C+1))
BP2_MUX_U13_I2C_CH1=$(($CURRENT_I2C+2))
BP2_MUX_U13_I2C_CH2=$(($CURRENT_I2C+3))
BP2_MUX_U13_I2C_CH3=$(($CURRENT_I2C+4))
echo pca9545 0x71 > /sys/bus/i2c/devices/i2c-$BP2_MUX_U13_I2C/new_device
if [ -d "/sys/bus/i2c/devices/i2c-$BP2_MUX_U13_I2C_CH0" ]
then
    BP2_PRESENT="true"
    echo 24c32 0x50 > /sys/bus/i2c/devices/i2c-$BP2_MUX_U13_I2C/new_device

    # PAC1932 U7
    echo pac1934 0x12 > /sys/bus/i2c/devices/i2c-$BP2_MUX_U13_I2C/new_device
    shunt_path=$(ls /sys/bus/i2c/devices/i2c-$BP2_MUX_U13_I2C/$BP2_MUX_U13_I2C-0012/hwmon/hwmon*/shunt_value)
    # shunt is 0.0005, 0.01 ohm
    echo 0 500 > $shunt_path
    echo 1 10000 > $shunt_path

    echo pca9545 0x73 > /sys/bus/i2c/devices/i2c-$BP2_MUX_U13_I2C_CH0/new_device
    echo pca9545 0x73 > /sys/bus/i2c/devices/i2c-$BP2_MUX_U13_I2C_CH1/new_device
    echo pca9545 0x73 > /sys/bus/i2c/devices/i2c-$BP2_MUX_U13_I2C_CH2/new_device
    CURRENT_I2C=$(($CURRENT_I2C+4))

    # Create BP2 json file
    bash $CREATE_JSON_SH bp2 $BP2_MUX_U13_I2C_CH0
    BP2_NVME_BASIC_I2C=$BP2_MUX_U13_I2C_CH0
    BP2_PATH="/sys/bus/i2c/devices/i2c-${BP2_MUX_U13_I2C}/${BP2_MUX_U13_I2C}-0050"
else
    BP2_PRESENT="false"
    BP2_NVME_BASIC_I2C=0
fi


echo "BP1 $BP1_PRESENT, BP2 $BP2_PRESENT"

# Create NvMe json
# Give BP1 basic I2C bus and BP2 basic I2C bus
# If not present, set to 0
bash $CREATE_JSON_SH nvme $BP1_NVME_BASIC_I2C $BP2_NVME_BASIC_I2C