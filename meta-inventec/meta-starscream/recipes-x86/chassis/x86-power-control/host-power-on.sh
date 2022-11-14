#!/bin/sh

echo "host-power-on start"

# Wait for CPU power on
sleep 10

# Re-start phosphor-pid-control
systemctl restart phosphor-pid-control.service

