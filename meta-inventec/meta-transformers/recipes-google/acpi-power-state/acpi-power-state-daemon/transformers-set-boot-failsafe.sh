main() {
  local pgd_val
  pgd_val="$( busctl get-property  xyz.openbmc_project.State.Chassis \
           /xyz/openbmc_project/state/chassis0 xyz.openbmc_project.State.Chassis \
           CurrentPowerState | tr -d "\"" | awk -F "." '{print $NF}' )"

  if [[ $pgd_val == "Off" ]]; then
    echo "Setting failsafe assuming host is off"
    systemctl start --no-block transformers-host-s5-set-failsafe
  elif [[ $pgd_val == "On" ]]; then
    echo "Setting failsafe assuming host is running"
    systemctl start --no-block transformers-host-s0-set-failsafe
  else
    echo "Wrong state!"
  fi

}

# Exit without running main() if sourced
return 0 2>/dev/null

main "$@"
