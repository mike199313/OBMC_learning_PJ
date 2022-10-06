#!/bin/bash
#
# A utility to run specific MTD image in iqemu docker 
#

set -e


#################
# Help Function #
#################
PROGRAM_NAME=$(basename ""$0"")
function print_help() {
  echo
  echo "USAGE: $PROGRAM_NAME <Image path> [OPTIONS]"
  echo 
  echo "Options:"
  echo "  -m     QEMU machine name, default is $QEMU_MECHINE"
  echo "  -p     Platform name, default is $PLATFORM"
  echo "  -v     Print version"
  echo "  -h     Print this message"
  echo 
  echo "Example:  $PROGRAM_NAME"
  echo "          $PROGRAM_NAME xxx.mtd"
  echo "          $PROGRAM_NAME -m transformers-bmc"
  echo "          $PROGRAM_NAME -m ast2600-evb"
  echo "          $PROGRAM_NAME -m ast2600-evb xxx.mtd"
  echo "          $PROGRAM_NAME -m transformers-bmc your/mtd/path/xxx.mtd"
  echo "          $PROGRAM_NAME -p transformers"
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


##########################
# Default Configurations #
##########################
TOP_PATH=$(dirname `pwd`)
DEFAULT_QEMU_MACHINE=transformers-bmc
DEFAULT_DOCKER_IMAGE=iqemu:latest
DEFAULT_PLATFORM=transformers


#######################
# Auto Configurations #
#######################
VERSION=1.0.0
QEMU_MECHINE=$DEFAULT_QEMU_MACHINE
PLATFORM=$DEFAULT_PLATFORM
DOCKER_IMAGE=$DEFAULT_DOCKER_IMAGE
USER_NAME=$(id -u -n)
CONTAINER_NAME=${USER_NAME}_iqemu


##################
# Option parsing #
##################
opts=`getopt -o hvm:p: -- "$@"`
eval set -- "$opts"

while true; do
    case "$1" in
      -m) QEMU_MECHINE=$2; shift 2;;
      -p) PLATFORM=$2; shift 2;;
      -v) print_version; exit 0;;
      -h) print_help; exit 0;;
      --) shift; break;;
    esac
done

DEFALUT_MTD_PATH=build/$PLATFORM/tmp/deploy/images/$PLATFORM

if [[ $PLATFORM == *-nuv ]]; then
    echo "Nuvoton platform detected"
    QEMU_MECHINE=npcm750-evb
    DOCKER_IMAGE=iqemu_nuvoton_v2:latest
fi


######################
# Image File Parsing #
######################
if [ $1 ]; then
    # User specified MTD file
    ABS_PATH=$(realpath $1)
    MTD_FILE=$(basename $ABS_PATH)
    MTD_PATH=$(dirname $ABS_PATH)
else
    # find the mtd file by search the default path
    MTD_PATH=$TOP_PATH/$DEFALUT_MTD_PATH
    if [ -d $MTD_PATH ]; then
        if [ -f `ls $MTD_PATH/*.mtd | tail -n 1` ]; then
            MTD_FILE=$(basename `ls $MTD_PATH/*.mtd | tail -n 1`)
        else
            echo "ERROR: There is no MDT file under $MTD_PATH. Please specify a MTD file."
            exit
        fi
    else
        echo "ERROR: There is no path $MTD_PATH. Please specify a MTD file"
        exit
    fi
fi

if [ -z MTD_FILE ]; then
    "ERROR: No MTD file specified!!"
    exit
fi


###################
# Run QEMU Docker #
###################
# Combine options
OPTION_BASIC+="-it --rm"
OPTION_CONTAINER_NAME+="--name $CONTAINER_NAME"
OPTION_VOLUME+="-v $MTD_PATH:/qemu"
OPTION_CONTAINER_CMD+="$QEMU_MECHINE $MTD_FILE"
OPTION_ALL+="$OPTION_BASIC $OPTION_CONTAINER_NAME $OPTION_VOLUME $DOCKER_IMAGE $OPTION_CONTAINER_CMD"
# Run docker
echo "docker run $OPTION_ALL"
docker run $OPTION_ALL

