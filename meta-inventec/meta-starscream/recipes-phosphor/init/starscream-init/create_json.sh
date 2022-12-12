#!/bin/sh

# create_json.sh $Target $bus

if [ -z $1 ]; then
    echo "I2C bus not given"
fi

if [ -z $2 ]; then
    echo "I2C bus not given"
fi

SPECIAL_CHAR="$"
BASIC_I2C_BUS=$2
ENTITY_MANAGER_CONIG_PATH="/usr/share/entity-manager/configurations"
ENTITY_MANAGER_RISER1A_JSON="$ENTITY_MANAGER_CONIG_PATH/riser1a.json"
ENTITY_MANAGER_RISER1B_JSON="$ENTITY_MANAGER_CONIG_PATH/riser1b.json"
ENTITY_MANAGER_RISER2_JSON="$ENTITY_MANAGER_CONIG_PATH/riser2.json"
ENTITY_MANAGER_BP1_JSON="$ENTITY_MANAGER_CONIG_PATH/bp1.json"
ENTITY_MANAGER_BP2_JSON="$ENTITY_MANAGER_CONIG_PATH/bp2.json"

NVME_CONFG_JSON="/etc/nvme/nvme_config.json"
NVME_EACH_BP=11

create_riser1a() {
# BASIC_I2C_BUS would expect to be 28 in full device
# Riser1A_FRU - 30(BASIC_I2C_BUS + 2) - 0x50
# PAC1934_U1 - 21(Fixed) - 0x12
# EMC1462T_U6 - 31(BASIC_I2C_BUS + 3) - 0x3c

cat <<EOF >$ENTITY_MANAGER_RISER1A_JSON
{
    "Exposes": [
        {
            "Bus": "21",
            "Address": "0x12",
            "Labels": [
                "in1",
                "in2",
                "in3",
                "in4"
            ],
            "Name": "RISER1A_PAC1934_U1",
            "in1_Scale" : 1000000,
            "in1_Name":"Riser1_1_12V",
            "in2_Scale" : 1000000,
            "in2_Name":"Riser1_1_3V3",
            "in3_Scale" : 1000000,
            "in3_Name":"Riser1_2_12V",
            "in4_Scale" : 1000000,
            "in4_Name":"Riser1_2_3V3",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Label": "in1",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 13.2
                },
                {
                    "Direction": "greater than",
                    "Label": "in1",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 12.6
                },
                {
                    "Direction": "less than",
                    "Label": "in1",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 11.4
                },
                {
                    "Direction": "less than",
                    "Label": "in1",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 10.8
                },
                {
                    "Direction": "greater than",
                    "Label": "in2",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 3.6
                },
                {
                    "Direction": "greater than",
                    "Label": "in2",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 3.5
                },
                {
                    "Direction": "less than",
                    "Label": "in2",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 3.1
                },
                {
                    "Direction": "less than",
                    "Label": "in2",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 3.0
                },
                {
                    "Direction": "greater than",
                    "Label": "in3",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 13.2
                },
                {
                    "Direction": "greater than",
                    "Label": "in3",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 12.6
                },
                {
                    "Direction": "less than",
                    "Label": "in3",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 11.4
                },
                {
                    "Direction": "less than",
                    "Label": "in3",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 10.8
                },
                {
                    "Direction": "greater than",
                    "Label": "in4",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 3.6
                },
                {
                    "Direction": "greater than",
                    "Label": "in4",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 3.5
                },
                {
                    "Direction": "less than",
                    "Label": "in4",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 3.1
                },
                {
                    "Direction": "less than",
                    "Label": "in4",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 3.0
                }
            ],
            "SensorInfo": [
                {
                    "Label": "in1",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x10",
                    "EntityId": "0x05",
                    "EntityInstance": "0"
                },
                {
                    "Label": "in2",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x11",
                    "EntityId": "0x05",
                    "EntityInstance": "1"
                },
                {
                    "Label": "in3",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x12",
                    "EntityId": "0x05",
                    "EntityInstance": "2"
                },
                {
                    "Label": "in4",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x13",
                    "EntityId": "0x05",
                    "EntityInstance": "3"
                }
            ],
            "Type": "PAC1934"
        },
        {
            "Name": "Riser1_1_12V",
            "SensorInfo": [
                {
                    "SensorModel": "Threshold",
                    "SensorNum": "0x10",
                    "EntityId": "0x05",
                    "EntityInstance": "0"
                }
             ]
        },
        {
            "Name": "Riser1_1_3V3",
            "SensorInfo": [
                {
                    "SensorModel": "Threshold",
                    "SensorNum": "0x11",
                    "EntityId": "0x05",
                    "EntityInstance": "1"
                }
             ]
        },
        {
            "Name": "Riser1_2_12V",
            "SensorInfo": [
                {
                    "SensorModel": "Threshold",
                    "SensorNum": "0x12",
                    "EntityId": "0x05",
                    "EntityInstance": "2"
                }
             ]
        },
        {
            "Name": "Riser1_2_3V3",
            "SensorInfo": [
                {
                    "SensorModel": "Threshold",
                    "SensorNum": "0x13",
                    "EntityId": "0x05",
                    "EntityInstance": "3"
                }
             ]
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+3))",
            "Address": "0x3C",
            "Name": "Riser1_Temp",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 56
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 55
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "Riser1_Temp",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x14",
                    "EntityId": "0x05",
                    "EntityInstance": "4"
                }
            ],
            "Type": "EMC1412"
        }
    ],
    "Name": "riser1a",
    "Probe": "xyz.openbmc_project.FruDevice({'BUS': $(($BASIC_I2C_BUS+2)), 'ADDRESS' : 80})",
    "Type": "Board",
    "xyz.openbmc_project.Inventory.Decorator.Asset": {
        "Manufacturer": "${SPECIAL_CHAR}BOARD_MANUFACTURER",
        "Model": "${SPECIAL_CHAR}BOARD_PRODUCT_NAME",
        "PartNumber": "${SPECIAL_CHAR}BOARD_PART_NUMBER",
        "SerialNumber": "${SPECIAL_CHAR}BOARD_SERIAL_NUMBER",
        "BuildDate": "${SPECIAL_CHAR}BOARD_MANUFACTURE_DATE"
    },
    "xyz.openbmc_project.Inventory.Decorator.FruDevice":{
        "Bus":$(($BASIC_I2C_BUS+2)),
        "Address":80
    },
    "xyz.openbmc_project.Inventory.Decorator.Ipmi": {
        "EntityId": "0x05",
        "EntityInstance": 0
    }
}
EOF
}

create_riser1b() {
# BASIC_I2C_BUS would expect to be 28 in full device
# Riser1B_FRU - 28(BASIC_I2C_BUS) - 0x50
# EMC1462T_U6 - 29(BASIC_I2C_BUS + 1) - 0x3c
# PAC1934_U1  - 31(BASIC_I2C_BUS + 3) - 0x12


cat <<EOF >$ENTITY_MANAGER_RISER1B_JSON
{
    "Exposes": [
        {
            "Bus": "$(($BASIC_I2C_BUS+3))",
            "Address": "0x12",
            "Labels": [
                "in1",
                "in2"
            ],
            "Name": "RISER1B_PAC1934_U1",
            "in1_Scale" : 1000000,
            "in1_Name":"Riser1_1_12V",
            "in2_Scale" : 1000000,
            "in2_Name":"Riser1_1_3V3",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Label": "in1",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 13.2
                },
                {
                    "Direction": "greater than",
                    "Label": "in1",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 12.6
                },
                {
                    "Direction": "less than",
                    "Label": "in1",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 11.4
                },
                {
                    "Direction": "less than",
                    "Label": "in1",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 10.8
                },
                {
                    "Direction": "greater than",
                    "Label": "in2",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 3.6
                },
                {
                    "Direction": "greater than",
                    "Label": "in2",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 3.5
                },
                {
                    "Direction": "less than",
                    "Label": "in2",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 3.1
                },
                {
                    "Direction": "less than",
                    "Label": "in2",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 3.0
                }
            ],
            "SensorInfo": [
                {
                    "Label": "in1",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x10",
                    "EntityId": "0x05",
                    "EntityInstance": "0"
                },
                {
                    "Label": "in2",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x11",
                    "EntityId": "0x05",
                    "EntityInstance": "1"
                }
            ],
            "Type": "PAC1934"
        },
        {
            "Name": "Riser1_1_12V",
            "SensorInfo": [
                {
                    "SensorModel": "Threshold",
                    "SensorNum": "0x10",
                    "EntityId": "0x05",
                    "EntityInstance": "0"
                }
             ]
        },
        {
            "Name": "Riser1_1_3V3",
            "SensorInfo": [
                {
                    "SensorModel": "Threshold",
                    "SensorNum": "0x11",
                    "EntityId": "0x05",
                    "EntityInstance": "1"
                }
             ]
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+1))",
            "Address": "0x3C",
            "Name": "Riser1_Temp",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 56
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 55
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "Riser1_Temp",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x14",
                    "EntityId": "0x05",
                    "EntityInstance": "4"
                }
            ],
            "Type": "EMC1412"
        }
    ],
    "Name": "riser1b",
    "Probe": "xyz.openbmc_project.FruDevice({'BUS': $(($BASIC_I2C_BUS)), 'ADDRESS' : 80})",
    "Type": "Board",
    "xyz.openbmc_project.Inventory.Decorator.Asset": {
        "Manufacturer": "${SPECIAL_CHAR}BOARD_MANUFACTURER",
        "Model": "${SPECIAL_CHAR}BOARD_PRODUCT_NAME",
        "PartNumber": "${SPECIAL_CHAR}BOARD_PART_NUMBER",
        "SerialNumber": "${SPECIAL_CHAR}BOARD_SERIAL_NUMBER",
        "BuildDate": "${SPECIAL_CHAR}BOARD_MANUFACTURE_DATE"
    },
    "xyz.openbmc_project.Inventory.Decorator.FruDevice":{
        "Bus":$(($BASIC_I2C_BUS)),
        "Address":80
    },
    "xyz.openbmc_project.Inventory.Decorator.Ipmi": {
        "EntityId": "0x05",
        "EntityInstance": 0
    }
}
EOF
}

create_riser2() {
# BASIC_I2C_BUS would expect to be 32 in full device
# Riser2_FRU - 34(BASIC_I2C_BUS + 2) - 0x50
# PAC1934_U1 - 22(Fixed) - 0x12
# EMC1462T_U6 - 35(BASIC_I2C_BUS + 3) - 0x3c

cat <<EOF >$ENTITY_MANAGER_RISER2_JSON
{
    "Exposes": [
        {
            "Bus": "22",
            "Address": "0x12",
            "Labels": [
                "in1",
                "in2",
                "in3",
                "in4"
            ],
            "Name": "RISER2_PAC1934_U1",
            "in1_Scale" : 1000000,
            "in1_Name":"Riser2_1_12V",
            "in2_Scale" : 1000000,
            "in2_Name":"Riser2_1_3V3",
            "in3_Scale" : 1000000,
            "in3_Name":"Riser2_2_12V",
            "in4_Scale" : 1000000,
            "in4_Name":"Riser2_2_3V3",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Label": "in1",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 13.2
                },
                {
                    "Direction": "greater than",
                    "Label": "in1",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 12.6
                },
                {
                    "Direction": "less than",
                    "Label": "in1",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 11.4
                },
                {
                    "Direction": "less than",
                    "Label": "in1",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 10.8
                },
                {
                    "Direction": "greater than",
                    "Label": "in2",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 3.6
                },
                {
                    "Direction": "greater than",
                    "Label": "in2",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 3.5
                },
                {
                    "Direction": "less than",
                    "Label": "in2",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 3.1
                },
                {
                    "Direction": "less than",
                    "Label": "in2",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 3.0
                },
                {
                    "Direction": "greater than",
                    "Label": "in3",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 13.2
                },
                {
                    "Direction": "greater than",
                    "Label": "in3",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 12.6
                },
                {
                    "Direction": "less than",
                    "Label": "in3",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 11.4
                },
                {
                    "Direction": "less than",
                    "Label": "in3",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 10.8
                },
                {
                    "Direction": "greater than",
                    "Label": "in4",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 3.6
                },
                {
                    "Direction": "greater than",
                    "Label": "in4",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 3.5
                },
                {
                    "Direction": "less than",
                    "Label": "in4",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 3.1
                },
                {
                    "Direction": "less than",
                    "Label": "in4",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 3.0
                }
            ],
            "SensorInfo": [
                {
                    "Label": "in1",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x15",
                    "EntityId": "0x05",
                    "EntityInstance": "5"
                },
                {
                    "Label": "in2",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x16",
                    "EntityId": "0x05",
                    "EntityInstance": "6"
                },
                {
                    "Label": "in3",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x17",
                    "EntityId": "0x05",
                    "EntityInstance": "7"
                },
                {
                    "Label": "in4",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x18",
                    "EntityId": "0x05",
                    "EntityInstance": "8"
                }
            ],
            "Type": "PAC1934"
        },
        {
            "Name": "Riser2_1_12V",
            "SensorInfo": [
                {
                    "SensorModel": "Threshold",
                    "SensorNum": "0x15",
                    "EntityId": "0x05",
                    "EntityInstance": "5"
                }
             ]
        },
        {
            "Name": "Riser2_1_3V3",
            "SensorInfo": [
                {
                    "SensorModel": "Threshold",
                    "SensorNum": "0x16",
                    "EntityId": "0x05",
                    "EntityInstance": "6"
                }
             ]
        },
        {
            "Name": "Riser2_2_12V",
            "SensorInfo": [
                {
                    "SensorModel": "Threshold",
                    "SensorNum": "0x17",
                    "EntityId": "0x05",
                    "EntityInstance": "7"
                }
             ]
        },
        {
            "Name": "Riser2_2_3V3",
            "SensorInfo": [
                {
                    "SensorModel": "Threshold",
                    "SensorNum": "0x18",
                    "EntityId": "0x05",
                    "EntityInstance": "8"
                }
             ]
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+3))",
            "Address": "0x3C",
            "Name": "Riser2_Temp",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 56
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 55
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "Riser2_Temp",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x19",
                    "EntityId": "0x05",
                    "EntityInstance": "9"
                }
            ],
            "Type": "EMC1412"
        }
    ],
    "Name": "riser2",
    "Probe": "xyz.openbmc_project.FruDevice({'BUS': $(($BASIC_I2C_BUS+2)), 'ADDRESS' : 80})",
    "Type": "Board",
    "xyz.openbmc_project.Inventory.Decorator.Asset": {
        "Manufacturer": "${SPECIAL_CHAR}BOARD_MANUFACTURER",
        "Model": "${SPECIAL_CHAR}BOARD_PRODUCT_NAME",
        "PartNumber": "${SPECIAL_CHAR}BOARD_PART_NUMBER",
        "SerialNumber": "${SPECIAL_CHAR}BOARD_SERIAL_NUMBER",
        "BuildDate": "${SPECIAL_CHAR}BOARD_MANUFACTURE_DATE"
    },
    "xyz.openbmc_project.Inventory.Decorator.FruDevice":{
        "Bus":$(($BASIC_I2C_BUS+2)),
        "Address":80
    },
    "xyz.openbmc_project.Inventory.Decorator.Ipmi": {
        "EntityId": "0x05",
        "EntityInstance": 1
    }
}
EOF
}


create_bp1() {
# BASIC_I2C_BUS would expect to be 36 in full device
# BP1_FRU - 24(Fixed) - 0x50
# PAC1932T_U7 - 24(Fixed) - 0x12
# EDSFF - 40~50(BASIC_I2C_BUS + 4~14) - 0x6a NvMe

cat <<EOF >$ENTITY_MANAGER_BP1_JSON
{
    "Exposes": [
        {
            "Bus": "24",
            "Address": "0x12",
            "Labels": [
                "in1",
                "in2"
            ],
            "Name": "BP1_PAC1932T_U7",
            "in1_Scale" : 1000000,
            "in1_Name":"BP1_12V",
            "in2_Scale" : 1000000,
            "in2_Name":"BP1_3V3",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Label": "in1",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 13.2
                },
                {
                    "Direction": "greater than",
                    "Label": "in1",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 12.6
                },
                {
                    "Direction": "less than",
                    "Label": "in1",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 11.4
                },
                {
                    "Direction": "less than",
                    "Label": "in1",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 10.8
                },
                {
                    "Direction": "greater than",
                    "Label": "in2",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 3.6
                },
                {
                    "Direction": "greater than",
                    "Label": "in2",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 3.5
                },
                {
                    "Direction": "less than",
                    "Label": "in2",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 3.1
                },
                {
                    "Direction": "less than",
                    "Label": "in2",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 3.0
                }
            ],
            "SensorInfo": [
                {
                    "Label": "in1",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x50",
                    "EntityId": "0x0F",
                    "EntityInstance": "0"
                },
                {
                    "Label": "in2",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x51",
                    "EntityId": "0x0F",
                    "EntityInstance": "1"
                }
            ],
            "Type": "PAC1932"
        },
        {
            "Name": "BP1_12V",
            "SensorInfo": [
                {
                    "SensorModel": "Threshold",
                    "SensorNum": "0x50",
                    "EntityId": "0x0F",
                    "EntityInstance": "0"
                }
             ]
        },
        {
            "Name": "BP1_3V3",
            "SensorInfo": [
                {
                    "SensorModel": "Threshold",
                    "SensorNum": "0x51",
                    "EntityId": "0x0F",
                    "EntityInstance": "1"
                }
             ]
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+4))",
            "Name": "BP1_EDSFF_J1",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP1_EDSFF_J1",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x60",
                    "EntityId": "0x1A",
                    "EntityInstance": "0"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+5))",
            "Name": "BP1_EDSFF_J2",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP1_EDSFF_J2",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x61",
                    "EntityId": "0x1A",
                    "EntityInstance": "1"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+6))",
            "Name": "BP1_EDSFF_J3",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP1_EDSFF_J3",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x62",
                    "EntityId": "0x1A",
                    "EntityInstance": "2"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+7))",
            "Name": "BP1_EDSFF_J4",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP1_EDSFF_J4",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x63",
                    "EntityId": "0x1A",
                    "EntityInstance": "3"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+8))",
            "Name": "BP1_EDSFF_J5",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP1_EDSFF_J5",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x64",
                    "EntityId": "0x1A",
                    "EntityInstance": "4"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+9))",
            "Name": "BP1_EDSFF_J6",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP1_EDSFF_J6",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x65",
                    "EntityId": "0x1A",
                    "EntityInstance": "5"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+10))",
            "Name": "BP1_EDSFF_J7",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP1_EDSFF_J7",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x66",
                    "EntityId": "0x1A",
                    "EntityInstance": "6"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+11))",
            "Name": "BP1_EDSFF_J8",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP1_EDSFF_J8",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x67",
                    "EntityId": "0x1A",
                    "EntityInstance": "7"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+12))",
            "Name": "BP1_EDSFF_J9",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP1_EDSFF_J9",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x68",
                    "EntityId": "0x1A",
                    "EntityInstance": "8"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+13))",
            "Name": "BP1_EDSFF_J10",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP1_EDSFF_J10",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x69",
                    "EntityId": "0x1A",
                    "EntityInstance": "9"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+14))",
            "Name": "BP1_EDSFF_J11",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP1_EDSFF_J11",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x6A",
                    "EntityId": "0x1A",
                    "EntityInstance": "10"
                }
            ],
            "Type": "NVME1000"
        }
    ],
    "Name": "bp1",
    "Probe": "xyz.openbmc_project.FruDevice({'BUS': 24, 'ADDRESS' : 80})",
    "Type": "Board",
    "xyz.openbmc_project.Inventory.Decorator.Asset": {
        "Manufacturer": "${SPECIAL_CHAR}BOARD_MANUFACTURER",
        "Model": "${SPECIAL_CHAR}BOARD_PRODUCT_NAME",
        "PartNumber": "${SPECIAL_CHAR}BOARD_PART_NUMBER",
        "SerialNumber": "${SPECIAL_CHAR}BOARD_SERIAL_NUMBER",
        "BuildDate": "${SPECIAL_CHAR}BOARD_MANUFACTURE_DATE"
    },
    "xyz.openbmc_project.Inventory.Decorator.FruDevice":{
        "Bus":24,
        "Address":80
    },
    "xyz.openbmc_project.Inventory.Decorator.Ipmi": {
        "EntityId": "0x0F",
        "EntityInstance": 0
    }
}
EOF
}


create_bp2() {
# BASIC_I2C_BUS would expect to be 52 in full device
# BP2_FRU - 25(Fixed) - 0x50
# PAC1932T_U7 - 25(Fixed) - 0x12
# EDSFF - 56~66(BASIC_I2C_BUS + 4~14) - 0x6a NvMe

cat <<EOF >$ENTITY_MANAGER_BP2_JSON
{
    "Exposes": [
        {
            "Bus": "25",
            "Address": "0x12",
            "Labels": [
                "in1",
                "in2"
            ],
            "Name": "BP2_PAC1932T_U7",
            "in1_Scale" : 1000000,
            "in1_Name":"BP2_12V",
            "in2_Scale" : 1000000,
            "in2_Name":"BP2_3V3",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Label": "in1",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 13.2
                },
                {
                    "Direction": "greater than",
                    "Label": "in1",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 12.6
                },
                {
                    "Direction": "less than",
                    "Label": "in1",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 11.4
                },
                {
                    "Direction": "less than",
                    "Label": "in1",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 10.8
                },
                {
                    "Direction": "greater than",
                    "Label": "in2",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 3.6
                },
                {
                    "Direction": "greater than",
                    "Label": "in2",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 3.5
                },
                {
                    "Direction": "less than",
                    "Label": "in2",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 3.1
                },
                {
                    "Direction": "less than",
                    "Label": "in2",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 3.0
                }
            ],
            "SensorInfo": [
                {
                    "Label": "in1",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x52",
                    "EntityId": "0x0F",
                    "EntityInstance": "2"
                },
                {
                    "Label": "in2",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x53",
                    "EntityId": "0x0F",
                    "EntityInstance": "3"
                }
            ],
            "Type": "PAC1932"
        },
        {
            "Name": "BP2_12V",
            "SensorInfo": [
                {
                    "SensorModel": "Threshold",
                    "SensorNum": "0x52",
                    "EntityId": "0x0F",
                    "EntityInstance": "2"
                }
             ]
        },
        {
            "Name": "BP2_3V3",
            "SensorInfo": [
                {
                    "SensorModel": "Threshold",
                    "SensorNum": "0x53",
                    "EntityId": "0x0F",
                    "EntityInstance": "3"
                }
             ]
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+4))",
            "Name": "BP2_EDSFF_J1",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP2_EDSFF_J1",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x6B",
                    "EntityId": "0x1A",
                    "EntityInstance": "11"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+5))",
            "Name": "BP2_EDSFF_J2",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP2_EDSFF_J2",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x6C",
                    "EntityId": "0x1A",
                    "EntityInstance": "12"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+6))",
            "Name": "BP2_EDSFF_J3",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP2_EDSFF_J3",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x6D",
                    "EntityId": "0x1A",
                    "EntityInstance": "13"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+7))",
            "Name": "BP2_EDSFF_J4",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP2_EDSFF_J4",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x6E",
                    "EntityId": "0x1A",
                    "EntityInstance": "14"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+8))",
            "Name": "BP2_EDSFF_J5",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP2_EDSFF_J5",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x6F",
                    "EntityId": "0x1A",
                    "EntityInstance": "15"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+9))",
            "Name": "BP2_EDSFF_J6",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP2_EDSFF_J6",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x70",
                    "EntityId": "0x1A",
                    "EntityInstance": "16"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+10))",
            "Name": "BP2_EDSFF_J7",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP2_EDSFF_J7",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x71",
                    "EntityId": "0x1A",
                    "EntityInstance": "17"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+11))",
            "Name": "BP2_EDSFF_J8",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP2_EDSFF_J8",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x72",
                    "EntityId": "0x1A",
                    "EntityInstance": "18"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+12))",
            "Name": "BP2_EDSFF_J9",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP2_EDSFF_J9",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x73",
                    "EntityId": "0x1A",
                    "EntityInstance": "19"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+13))",
            "Name": "BP2_EDSFF_J10",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP2_EDSFF_J10",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x74",
                    "EntityId": "0x1A",
                    "EntityInstance": "20"
                }
            ],
            "Type": "NVME1000"
        },
        {
            "Bus": "$(($BASIC_I2C_BUS+14))",
            "Name": "BP2_EDSFF_J11",
            "Thresholds": [
                {
                    "Direction": "greater than",
                    "Name": "upper critical",
                    "Severity": 1,
                    "Value": 70
                },
                {
                    "Direction": "greater than",
                    "Name": "upper non critical",
                    "Severity": 0,
                    "Value": 65
                },
                {
                    "Direction": "less than",
                    "Name": "lower non critical",
                    "Severity": 0,
                    "Value": 10
                },
                {
                    "Direction": "less than",
                    "Name": "lower critical",
                    "Severity": 1,
                    "Value": 5
                }
            ],
            "SensorInfo": [
                {
                    "Label": "BP2_EDSFF_J11",
                    "SensorModel": "Threshold",
                    "SensorNum": "0x75",
                    "EntityId": "0x1A",
                    "EntityInstance": "21"
                }
            ],
            "Type": "NVME1000"
        }
    ],
    "Name": "bp2",
    "Probe": "xyz.openbmc_project.FruDevice({'BUS': 25, 'ADDRESS' : 80})",
    "Type": "Board",
    "xyz.openbmc_project.Inventory.Decorator.Asset": {
        "Manufacturer": "${SPECIAL_CHAR}BOARD_MANUFACTURER",
        "Model": "${SPECIAL_CHAR}BOARD_PRODUCT_NAME",
        "PartNumber": "${SPECIAL_CHAR}BOARD_PART_NUMBER",
        "SerialNumber": "${SPECIAL_CHAR}BOARD_SERIAL_NUMBER",
        "BuildDate": "${SPECIAL_CHAR}BOARD_MANUFACTURE_DATE"
    },
    "xyz.openbmc_project.Inventory.Decorator.FruDevice":{
        "Bus":25,
        "Address":80
    },
    "xyz.openbmc_project.Inventory.Decorator.Ipmi": {
        "EntityId": "0x0F",
        "EntityInstance": 1
    }
}
EOF
}

create_nvme() {
echo -e \
"{\n\
        \"config\": [" > $NVME_CONFG_JSON

INDEX=0
if [ $BP1_BASIC_I2C -ne 0 ]; then
for i in $(seq 0 $(($NVME_EACH_BP-1)));
do

if [ $i -eq $(($NVME_EACH_BP-1)) ]; then
    DOT=""
else
    DOT=","
fi

if [ $BP2_BASIC_I2C -ne 0 ]; then
    DOT=","
fi

echo -e \
"\
            {\n\
                \"NVMeDriveIndex\": $INDEX,\n\
                \"NVMeDriveBusID\": $(($BP1_BASIC_I2C+$i+4))\n\
            }$DOT\
" >> $NVME_CONFG_JSON

    INDEX=$(($INDEX+1))
done
fi

INDEX=$NVME_EACH_BP
if [ $BP2_BASIC_I2C -ne 0 ]; then

for i in $(seq 0 $(($NVME_EACH_BP-1)));
do
if [ $i -eq $(($NVME_EACH_BP-1)) ]; then
    DOT=""
else
    DOT=","
fi

echo -e \
"\
            {\n\
                \"NVMeDriveIndex\": $INDEX,\n\
                \"NVMeDriveBusID\": $(($BP2_BASIC_I2C+$i+4))\n\
            }$DOT\
" >> $NVME_CONFG_JSON

    INDEX=$(($INDEX+1))
done
fi

echo -e \
"       ],\n\
        \"threshold\": [\n\
            {\n\
                \"criticalHigh\": 75,\n\
                \"criticalLow\": 0,\n\
                \"maxValue\": 127,\n\
                \"minValue\": -127\n\
            }\n\
    ]\n\
}\n" >> $NVME_CONFG_JSON

}

# Switch module
if [ $1 = "riser1a" ]; then
    create_riser1a
elif [ $1 = "riser1b" ]; then
    create_riser1b
elif [ $1 = "riser2" ]; then
    create_riser2
elif [ $1 = "bp1" ]; then
    create_bp1
elif [ $1 = "bp2" ]; then
    create_bp2
elif [ $1 = "nvme" ]; then
    BP1_BASIC_I2C=$2
    BP2_BASIC_I2C=$3
    create_nvme
else
    echo "No match module"
fi


