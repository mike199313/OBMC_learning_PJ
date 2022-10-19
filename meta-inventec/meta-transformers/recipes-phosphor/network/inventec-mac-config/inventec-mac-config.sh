#!/bin/sh
set -e
ETH0_NETWORK_FILE="00-bmc-eth0.network"
ETH0_OVERRIDE_PATH="/run/systemd/network/$ETH0_NETWORK_FILE.d"


mkdir -p $ETH0_OVERRIDE_PATH


MAC_STR=`/usr/bin/mac_util r eth0`

MAC_STR=`echo ${MAC_STR//0x/}`
MAC_STR=`echo ${MAC_STR// /:}`


printf '[Link]\nMACAddress=%s\n' "$MAC_STR" >"$ETH0_OVERRIDE_PATH"/50-mac.conf

