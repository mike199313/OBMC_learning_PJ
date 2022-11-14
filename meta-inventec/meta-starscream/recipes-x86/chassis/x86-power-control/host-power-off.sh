#!/bin/sh

echo "host-power-off start"

# Re-start phosphor-pid-control
systemctl stop phosphor-pid-control.service

