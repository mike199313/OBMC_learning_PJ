#get the cpld config and get cpld mb & scm version
CPLD_VERSION_PATH="/usr/share/version"
CPLD_MB_BUS=0
CPLD_MB_ADDR=0
CPLD_SCM_BUS=0
CPLD_SCM_ADDR=0
CLPD_SUCCESS=false

detect_cpld_main()
{
    get_cpld_config
    if [ $CLPD_SUCCESS == true ]
    then
        mkdir -p $CPLD_VERSION_PATH
        CPLD_MB_ADDR=`printf "0x%02x\n" "$(($CPLD_MB_ADDR*0x02))"`
        CPLD_SCM_ADDR=`printf "0x%02x\n" "$(($CPLD_SCM_ADDR*0x02))"`

        echo "HW Bus Addr"
        echo "CPLD_MB_BUS: $CPLD_MB_BUS CPLD_MB_ADDR: $CPLD_MB_ADDR"
        echo "CPLD_SCM_BUS: $CPLD_SCM_BUS CPLD_SCM_ADDR: $CPLD_SCM_ADDR"

        get_mb_version_data=`ipmitool i2c bus=$(echo $CPLD_MB_BUS) $(echo $CPLD_MB_ADDR) 0x04 0xc0 0x0 0x0 0x0`
        mb_version=""
        for (( k=1; k<=4; k=k+1 )); do
            mb_version_data="$(echo $get_mb_version_data | cut -d ' ' -f$k)"
            if [ $mb_version_data != "00" ]; then
                    mb_version=$mb_version$mb_version_data
            fi
        done
        echo $mb_version > $CPLD_VERSION_PATH/MB_CPLD_Version
        echo "MB CPLD Verion: $mb_version"

        get_scm_version_data=`ipmitool i2c bus=$(echo $CPLD_SCM_BUS) $(echo $CPLD_SCM_ADDR) 0x04 0xc0 0x0 0x0 0x0`
        scm_version=""
        for (( k=1; k<=4; k=k+1 )); do
            scm_version_data="$(echo $get_scm_version_data | cut -d ' ' -f$k)"
            if [ $scm_version_data != "00" ]; then
                    scm_version=$scm_version$scm_version_data
            fi
        done
        echo $scm_version > $CPLD_VERSION_PATH/SCM_CPLD_Version
        echo "SCM CPLD Verion: $scm_version"
    else
        echo "error to get cpld config"
    fi
}

CPLD_CONFIG_PATH="/usr/share/cpldupdate-i2c/config.json"

get_cpld_config()
{
    if [ -f "$CPLD_CONFIG_PATH" ]; then
        CPLD_MB_BUS=`cat $CPLD_CONFIG_PATH | grep -A 4 'MB' | grep bus | cut -d ':' -f2 | cut -d ',' -f1`
        CPLD_MB_ADDR=`cat $CPLD_CONFIG_PATH | grep -A 4 'MB' | grep addr | cut -d ':' -f2 | cut -d ',' -f1 | cut -d '"' -f2`

        CPLD_SCM_BUS=`cat $CPLD_CONFIG_PATH | grep -A 4 'SCM' | grep bus | cut -d ':' -f2 | cut -d ',' -f1`
        CPLD_SCM_ADDR=`cat $CPLD_CONFIG_PATH | grep -A 4 'SCM' | grep addr | cut -d ':' -f2 | cut -d ',' -f1 | cut -d '"' -f2`

        echo "SW Bus Addr"
        echo "CPLD_MB_BUS: $CPLD_MB_BUS CPLD_MB_ADDR: $CPLD_MB_ADDR"
        echo "CPLD_SCM_BUS: $CPLD_SCM_BUS CPLD_SCM_ADDR: $CPLD_SCM_ADDR"
        CLPD_SUCCESS=true
    else
        echo "File $CPLD_CONFIG_PATH does not exists."
        CLPD_SUCCESS=false
    fi
}

#detect cpld device id
detect_cpld_main

