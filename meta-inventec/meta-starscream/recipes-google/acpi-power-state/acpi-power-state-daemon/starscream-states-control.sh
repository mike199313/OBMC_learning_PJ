#check if args exists
if [ $# -lt 1 ]; then
  echo "Wrong number of arguments" >&2
  exit 1
fi

current_state="$1"

result=0

#variables for vhub commands
NETFN_OEM=0x32
CMD_VIRTUAL_USB=0xaa
CONFIG_ENABLE_VHUB=0x00

if [[ $current_state -eq 0 ]]; then
  echo "Detect power state s0"
    #enable vhub
    ipmitool raw ${NETFN_OEM} ${CMD_VIRTUAL_USB} ${CONFIG_ENABLE_VHUB}
fi

exit $result
