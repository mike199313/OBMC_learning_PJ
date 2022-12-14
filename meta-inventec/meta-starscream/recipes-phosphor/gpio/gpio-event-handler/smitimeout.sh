#!/bin/sh

TIMEOUT=$1

# waiting for timeout 
sleep ${TIMEOUT}

# if service unit is not STOPed. do system SEL

busctl call "xyz.openbmc_project.Logging.IPMI" "/xyz/openbmc_project/Logging/IPMI" \
 "xyz.openbmc_project.Logging.IPMI" "IpmiSelAdd" ssaybq "SMITimeoutCheck" \
 "/xyz/openbmc_project/sensors/oem_event_00h/oem_d5h/SMITimeoutCheck" 3 {0xFF,0xFF,0xFF} yes 0x20
