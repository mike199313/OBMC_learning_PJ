#!/bin/sh
DBUS_ASSET_INTERFACE="xyz.openbmc_project.Inventory.Decorator.Asset"
DBUS_ITEM_INTERFACE="xyz.openbmc_project.Inventory.Item"
DBUS_POWERSUPPLY_INTERFACE="xyz.openbmc_project.Inventory.Item.PowerSupply"

#Asset.Manufacturer
MFR_ID="0x99"
#Asset.Model
MFR_MODEL="0x9a"
#Revision.Version
MFR_REVISION="0x9b"
#LocationCode.LocationCode
MFR_LOCATION="0x9c"
#Asset.BuildDate
MFR_DATE="0x9d"
#Asset.SerialNumber
MFR_SERIAL="0x9e"
#Asset.PartNumber
IC_DEVICE_ID="0xad"

# Time for device to ready after plugin
DEVICE_READY_DELAY=1

# PMBUS message retry
PMBUS_RETRY=10
PMBUS_DELAY=1

if [ $# -lt 0 ]; then
    echo 'No PSU ID and Action is given' >&2
    exit 1
fi
INIT=${1:0:1}
PSU_ID=${1:1:1}
ACTION=${1:2}

if [ $PSU_ID == 1 ]; then
    echo "PSU ID 1"
    I2C_BUS=6
    I2C_ADDR=0x58
elif [ $PSU_ID == 2 ]; then
    echo "PSU ID 2"
    I2C_BUS=6
    I2C_ADDR=0x5A
else
    echo "Not supported PSU ID"
    exit 1
fi



string=''
pmbus_read() {
    string=''
    INDEX=0
    while [ $INDEX -lt $PMBUS_RETRY ]
    do
        ret=$(i2cget -f -y $1 $2 $3 i 1)

        if [[ -z "$ret" ]]; then
            echo "i2c-$1 device $2 command $3 error" >&2
            sleep $PMBUS_DELAY
            (( INDEX++ ))
            if [ $INDEX -eq $PMBUS_RETRY ]; then
                exit 1
            fi
        else
            INDEX=$PMBUS_RETRY
        fi
    done

    len_string=$(echo ${ret} | cut -d" " -f 2)

    if [ $len_string == "0xff" ]; then
        echo "Not support field $3"
        return
    fi

    len=$(($len_string))

    data=$(i2cget -f -y $1 $2 $3 i $(($len+1)))

    arry=$(echo ${data} | sed -e "s/$len_string//" | sed -e "s/\0x00//g" | sed -e "s/\0xff//g" | sed -e "s/\0x7f//g" | sed -e "s/\0x0f//g" | sed -e "s/\0x14//g")

    for d in ${arry}
    do
        hex=$(echo $d | sed -e "s/0\x//")
        string+=$(echo -e "\x${hex}");
    done
}


update_inventory() {
      INVENTORY_SERVICE='xyz.openbmc_project.Inventory.Manager'
      INVENTORY_OBJECT='/xyz/openbmc_project/inventory'
      INVENTORY_PATH='xyz.openbmc_project.Inventory.Manager'
      OBJECT_PATH="/system/chassis/motherboard/powersupply$1"

      if [ $3 == 0 ]; then
        busctl call \
            ${INVENTORY_SERVICE} \
            ${INVENTORY_OBJECT} \
            ${INVENTORY_PATH} \
            Notify a{oa{sa{sv}}} 1 \
            ${OBJECT_PATH} 1 $2 $3
      else
        busctl call \
            ${INVENTORY_SERVICE} \
            ${INVENTORY_OBJECT} \
            ${INVENTORY_PATH} \
            Notify a{oa{sa{sv}}} 1 \
            ${OBJECT_PATH} 1 $2 $3 \
            $4 $5 "$6"
      fi
}

if [ $ACTION = "plug" ]; then
    if  [ $INIT -ne "1" ]; then
        sleep $DEVICE_READY_DELAY
    fi

    # Delete the PSU before creating a new one, since the entity manager will attempt to 
    # create one during system boot up, and this may cause the creation process to fail.
    echo $I2C_ADDR > /sys/bus/i2c/devices/i2c-$I2C_BUS/delete_device
    echo "pmbus" $I2C_ADDR > /sys/bus/i2c/devices/i2c-$I2C_BUS/new_device

    update_inventory $PSU_ID $DBUS_ITEM_INTERFACE 1 "Present" "b" "true"
    update_inventory $PSU_ID $DBUS_ITEM_INTERFACE 1 "PrettyName" "s" "powersupply$PSU_ID"

    pmbus_read $I2C_BUS $I2C_ADDR $MFR_ID
    update_inventory $PSU_ID $DBUS_ASSET_INTERFACE 1 "Manufacturer" "s" "$string"

    pmbus_read $I2C_BUS $I2C_ADDR $MFR_MODEL
    update_inventory $PSU_ID $DBUS_ASSET_INTERFACE 1 "Model" "s" "$string"

    pmbus_read $I2C_BUS $I2C_ADDR $MFR_DATE
    update_inventory $PSU_ID $DBUS_ASSET_INTERFACE 1 "BuildDate" "s" "$string"

    pmbus_read $I2C_BUS $I2C_ADDR $MFR_SERIAL
    update_inventory $PSU_ID $DBUS_ASSET_INTERFACE 1 "SerialNumber" "s" "$string"

    pmbus_read $I2C_BUS $I2C_ADDR $IC_DEVICE_ID
    update_inventory $PSU_ID $DBUS_ASSET_INTERFACE 1 "PartNumber" "s" "$string"

    update_inventory $PSU_ID $DBUS_POWERSUPPLY_INTERFACE 0
else
    update_inventory $PSU_ID $DBUS_ITEM_INTERFACE 1 "Present" "b" "false"
    update_inventory $PSU_ID $DBUS_ITEM_INTERFACE 1 "PrettyName" "s" "powersupply$PSU_ID"

    update_inventory $PSU_ID $DBUS_ASSET_INTERFACE 1 "Manufacturer" "s" "nan"
    update_inventory $PSU_ID $DBUS_ASSET_INTERFACE 1 "Model" "s" "nan"
    update_inventory $PSU_ID $DBUS_ASSET_INTERFACE 1 "BuildDate" "s" "nan"
    update_inventory $PSU_ID $DBUS_ASSET_INTERFACE 1 "SerialNumber" "s" "nan"
    update_inventory $PSU_ID $DBUS_ASSET_INTERFACE 1 "PartNumber" "s" "nan"

    update_inventory $PSU_ID $DBUS_POWERSUPPLY_INTERFACE 0
    echo $I2C_ADDR > /sys/bus/i2c/devices/i2c-$I2C_BUS/delete_device

fi

# Restart psu dbus-sensor service
if  [ $INIT -ne "1" ]; then
    systemctl restart xyz.openbmc_project.psusensor.service
fi
