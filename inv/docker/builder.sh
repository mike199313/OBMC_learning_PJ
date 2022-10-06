#!/bin/bash
TOPDIR=`pwd`/../..
USER=`pwd|awk -F"/" '{print $3}'`

DOCKER_IMAGE=${USER}_openbmc
CONTAINER_NAME=${USER}_openbmc
OPENBMC_DIR=/openbmc
PROXY_ENV_SETTING=
APT_CONF_FILE=apt.conf
GIT_CONF_FILE=.gitconfig
BITBAKE_TARGET=obmc-phosphor-image

function print_help() {
  echo "USAGE: $(basename ""$0"") [OPTIONS]"
  echo "Options:"
  echo "  -h | --help"
  echo "    Print this message"
  echo "  -n | --name"
  echo "    Set Container name"
  echo "  -t | --target"
  echo "    Build target directly"
  echo "  -b | --bitbake"
  echo "    Bitbake target"
  exit 0
}


trap 'exit' ERR

opts=`getopt -o hn:t:b: --long help,name:,taget:,bitbake: -- "$@"`

eval set -- "$opts"

while true; do
    case "$1" in
        -h | --help )
            print_help
            exit 0
            ;;
        -n | --name )
            CONTAINER_NAME=$2
            shift 2;;
        -t | --target )
            TARGET=$2
            shift 2;;
        -b | --bitbake )
            BITBAKE_TARGET=$2
            shift 2;;
        --) shift; break;;
    esac
done


BUILD_ARGS="--build-arg user=`id -un` --build-arg uid=`id -u` --build-arg guid=`id -g` "


if [ $http_proxy ]
then
    BUILD_ARGS+="--build-arg http_proxy=$http_proxy "
    PROXY_ENV_SETTING+="-e http_proxy=$http_proxy "
    cat <<EOF > $APT_CONF_FILE
Acquire::http::Proxy "$http_proxy";
EOF

fi

if [ $https_proxy ]
then
    BUILD_ARGS+="--build-arg https_proxy=$https_proxy"
    PROXY_ENV_SETTING+="-e https_proxy=$https_proxy"
    cat <<EOF >> $APT_CONF_FILE
Acquire::https::Proxy "$https_proxy";
EOF
fi

cp /home/$USER/$GIT_CONF_FILE .

echo docker build $BUILD_ARGS -t $DOCKER_IMAGE .
docker build $BUILD_ARGS -t $DOCKER_IMAGE .
rm -rf $APT_CONF_FILE
rm -rf $GIT_CONF_FILE




echo $CONTAINER_NAME
DOCKER_RUN_ARGS="--rm --net host --name $CONTAINER_NAME --workdir $OPENBMC_DIR $PROXY_ENV_SETTING -v $TOPDIR:$OPENBMC_DIR $DOCKER_IMAGE"

if [ -z "$TARGET" ]
then
    docker run -it $DOCKER_RUN_ARGS /bin/bash
else
    docker run -i $DOCKER_RUN_ARGS  /bin/bash /openbmc/inv/build_bmc.sh $TARGET $BITBAKE_TARGET
fi

