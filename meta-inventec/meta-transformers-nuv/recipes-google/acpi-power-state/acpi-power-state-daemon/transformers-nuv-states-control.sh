#check if args exists
if [ $# -lt 1 ]; then
  echo "Wrong number of arguments" >&2
  exit 1
fi

current_state="$1"

FAN_MAX=8
SET_PWM_RETRIES=100
PID_STATUS_RETRIES=100
result=0

service_status="$( systemctl is-active phosphor-pid-control.service )"

if [[ $current_state -eq 0 ]]; then
  echo "Detect power state s0"
fi

exit $result
