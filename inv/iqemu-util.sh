#!/bin/bash
#
# A utility that easy to interact with iqemu container
#

set -e


#################
# Help Function #
#################
PROGRAM_NAME=$(basename ""$0"")
function print_help() {
  echo 
  echo "USAGE: $PROGRAM_NAME [COMMAND] <options..>"
  echo 
  echo "Commands:"
  echo "  help      Print this message"
  echo "  show      Show command examples"
  echo "  ssh       Connect to QEMU container via SSH"
  echo "  web       Open firefox and browse OpenBMC WebUI"
  echo "  ipmi      Run ipmitool command"
  echo "  scp       Copy file from host to guest"
  echo "  cleankey  Clean RSA host key for container"
  echo "  version   Show version of this script"
  echo 
  echo "Example:  $PROGRAM_NAME show"
  echo "          $PROGRAM_NAME ssh"
  echo "          $PROGRAM_NAME web"
  echo "          $PROGRAM_NAME ipmi mc info"
  echo "          $PROGRAM_NAME ipmi sdr"
  echo "          $PROGRAM_NAME scp file /guest/path"
  echo
  exit 0
}


#########################
# Show Version Function #
#########################
function print_version() {
  echo
  echo "Version: $VERSION"
  echo 
  exit 0
}


##############################
# Check Container is running #
##############################
function check_container_running() {
    CONTAINER_ID=$(docker ps -q -f name=$CONTAINER_NAME)
    if [ -z $CONTAINER_ID ]
    then
        echo "ERROR: Container $CONTAINER_NAME not found!!"
        echo "Please run start-iqemu.sh first."
        echo 
        exit 0
    fi
}


######################
# Check Container IP #
######################
function check_container_ip() {
    GUEST_IP=$(docker inspect -f '{{range.NetworkSettings.Networks}}{{.IPAddress}}{{end}}' $CONTAINER_NAME)
    if [ -z $GUEST_IP ]
    then
        echo "ERROR: Container IP not found!!"
        echo 
        exit 0
    fi
}


##################################
# Command Examples for Conatiner #
##################################
function print_cmd_example() {
  echo 
  echo " **************************************"
  echo " * [SSH]                              *"
  echo " * Connect to QEMU container via SSH  *"
  echo " **************************************"
  echo " ssh -p $SSH_PORT root@$GUEST_IP"
  echo 
  echo 
  echo " **************************"
  echo " * [Web UI]               *"
  echo " * Browse OpenBMC Web UI  *"
  echo " **************************"
  echo " firefox https://$GUEST_IP:$WEBUI_PORT/"
  echo 
  echo 
  echo " ******************************"
  echo " * [IPMI]                     *"
  echo " * Run IPMI command over LAN  *"
  echo " ******************************"
  echo " ipmitool -I lanplus -C 17 -p $IPMI_PORT -U $OBMC_LOGIN_ID -P $OBMC_LOGIN_PW -H $GUEST_IP mc info"
  echo 
  echo 
  echo " *********************************"
  echo " * [SCP]                         *"
  echo " * Copy file from host to guest  *"
  echo " *********************************"
  echo " scp -P $SSH_PORT [file] root@$GUEST_IP:[path to host]"
  echo
  echo 
  exit 0
}


##########################
# Default Configurations #
##########################
# Default Port
IPMI_PORT=2623
SSH_PORT=2222
WEBUI_PORT=2443
# Default Login Password
OBMC_LOGIN_ID=root
OBMC_LOGIN_PW=0penBmc
# Container IP
GUEST_IP=


#######################
# Auto Configurations #
#######################
VERSION=1.0.0
USER_NAME=$(id -u -n)
CONTAINER_NAME=${USER_NAME}_iqemu


########
# Main #
########
# Check
check_container_running
check_container_ip
# Run
case "$1" in
    show) print_cmd_example
        ;;
    ssh) ssh -p $SSH_PORT root@$GUEST_IP
        ;;
    web) firefox https://$GUEST_IP:$WEBUI_PORT/
        ;;
    ipmi) ipmitool -I lanplus -C 17 -p $IPMI_PORT -U $OBMC_LOGIN_ID -P $OBMC_LOGIN_PW -H $GUEST_IP ${@:2}
        ;;
    scp) scp -P $SSH_PORT $2 root@$GUEST_IP:$3
        ;;
    cleankey) ssh-keygen -f "/home/$USER_NAME/.ssh/known_hosts" -R "[$GUEST_IP]:$SSH_PORT"
        ;;
    version) print_version
        ;;
    *) print_help
esac


