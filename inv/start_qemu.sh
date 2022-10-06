TOP_PATH=`pwd`/..
INV_PATH=$TOP_PATH/inv
MECHINE=transformers
QEMU_MECHINE=ast2600-evb
QEMU_ARM=$INV_PATH/qemu-system-arm


function print_help() {
  echo "USAGE: $(basename ""$0"") [OPTIONS]"
  echo "Options:"
  echo "  -m"
  echo "    Declare opembmc machine, default would transformers"
  echo "  -p"
  echo "    Set port redirect offset, add offset to ssh,https,ipmi ports"
  echo "  --qm"
  echo "    Declare qemu machine, default would be ast2600-evb"
  echo "  --ref"
  echo "    Run the reference platform"
  exit 0
}

trap 'exit' ERR

opts=`getopt -o m:p: --long qm:,ref -- "$@"`

eval set -- "$opts"

while true; do
    case "$1" in
      -m) MECHINE=$2; shift 2;;
      -p) PORT_REDIRECT_OFFSET=$2; shift 2;;
      --qm) QEMU_MECHINE=$2; shift 2;;
      --ref) IMAGE_PATH=$INV_PATH; QEMU_MECHINE=romulus-bmc; MECHINE=romulus; shift 1;;
      --) shift; break;;
    esac
done



if [ -z $PORT_REDIRECT_OFFSET ]
then
    echo "OFFSET not set, auto detect"
    let PORT_REDIRECT_OFFSET=$(/bin/ps -aux|grep -c "qemu-system-arm" )
    PORT_REDIRECT_OFFSET=$(( ($PORT_REDIRECT_OFFSET+1) * 1000 ))
fi
echo "Your PORT OFFSET is $PORT_REDIRECT_OFFSET"


sleep 2

if [ -z $IMAGE_PATH ]
then
  IMAGE_PATH=$TOP_PATH/build/$MECHINE/tmp/deploy/images/$MECHINE
  echo $IMAGE_PATH
fi

MTD_IMAGE=$(ls $IMAGE_PATH/*$MECHINE.static.mtd)

echo $MTD_IMAGE


#CUSTOM_OPTION+="-net nic,model=e1000,netdev=netdev1 -netdev user,id=netdev1,"
#CUSTOM_OPTION+="-net nic -net user,"
#CUSTOM_OPTION+="-netdev user,id=ens1,"
#CUSTOM_OPTION+="-net nic,model=ftgmac100,netdev=netdev1 -netdev user,id=netdev1,"
#CUSTOM_OPTION+="-device e1000,netdev=net0 "
#CUSTOM_OPTION+="-netdev user,id=net0,netdev=net0"
#CUSTOM_OPTION+="if=mtd -nic tap,ifname=tap0,script=no,downscript=no"
#CUSTOM_OPTION+="-netdev user,id=mynet,"


DRIVE_OPTION+="-drive file=$MTD_IMAGE,format=raw,if=mtd "


USER_NETWORK_OPTION+="-net nic -net user,"

SSH_PORT=$(( $PORT_REDIRECT_OFFSET +22))
HTTPS_PORT=$(( $PORT_REDIRECT_OFFSET +443))
IPMI_PORT=$(( $PORT_REDIRECT_OFFSET +623))


#USER_NETWORK_OPTION+="hostfwd=:127.0.0.1:$SSH_PORT-:22,"
#USER_NETWORK_OPTION+="hostfwd=:127.0.0.1:$HTTPS_PORT-:443,"
#USER_NETWORK_OPTION+="hostfwd=udp:127.0.0.1:$IPMI_PORT-:623,"
USER_NETWORK_OPTION+="hostname=qemu"

TAP_NETWORK_OPTION+="-net nic -net tap,"
TAP_NETWORK_OPTION+="ifname=tap0,script=no,downscript=no "


CUSTOM_OPTION+="-nographic "
CUSTOM_OPTION+=$DRIVE_OPTION
CUSTOM_OPTION+=$USER_NETWORK_OPTION

MEMORY=1024

echo $QEMU_ARM -m $MEMORY -M $QEMU_MECHINE $CUSTOM_OPTION
$QEMU_ARM -m $MEMORY -M $QEMU_MECHINE $CUSTOM_OPTION


