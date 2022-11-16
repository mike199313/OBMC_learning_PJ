#!/bin/sh

# Init GPIO setting

gpioset `gpiofind BMC_READY`=0
echo BMC ready !!

