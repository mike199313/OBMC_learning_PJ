#!/bin/bash

if [ -z $1 ]
then
    echo "No target specific"
    exit 2
else
    TARGET=$1
fi

if [ -z $2 ]
then
    BITBAKE_TARGET=obmc-phosphor-image
else
    BITBAKE_TARGET=$2
fi

IMAGE_PATH="/openbmc/build/tmp/deploy/images/$TARGET"
IMAGE_NAME="obmc-phosphor-image-$TARGET.static.mtd"

echo "Build $TARGET"

rm -rf /openbmc/build/conf
source setup $TARGET
bitbake $BITBAKE_TARGET

md5sum $IMAGE_PATH/$IMAGE_NAME > $IMAGE_PATH/$IMAGE_NAME.md5sum


echo "Build done"



