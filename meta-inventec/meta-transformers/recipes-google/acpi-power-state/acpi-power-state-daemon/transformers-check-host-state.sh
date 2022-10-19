#!/bin/bash
TARGET_INTERFACE="xyz.openbmc_project.State.Chassis"
POWER_OFF_STATE="xyz.openbmc_project.State.Chassis.PowerState.Off"

dbus-monitor --system type='signal',interface='org.freedesktop.DBus.Properties',\
member='PropertiesChanged',arg0namespace=$TARGET_INTERFACE | \
while read -r line; do
  #if the message contains 'member' inside, it is signal info, skip
  if [[ "$line" =~ "member" ]]; then
    continue
  fi

  #check if the required state (power off) is in new property info
  if [[ "$line" =~ "$POWER_OFF_STATE" ]]; then
    echo "Setting failsafe assuming host is off" >&2
    #power is off, run required service
    systemctl start --no-block transformers-host-s5-set-failsafe
  fi
done
