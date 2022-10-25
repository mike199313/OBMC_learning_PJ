#!/bin/sh

num=1000
progress=30
OBJECT="/xyz/openbmc_project/software"
INTERFACE="xyz.openbmc_project.Software.ActivationProgress"

while [  $num -ge 1 ]; do
        #get pid num of application "bios-update"
        PID=$(pidof bios-update)
        if [ $PID ]
        then
	    cur_progress=$(busctl get-property xyz.openbmc_project.Software.BMC.Updater $OBJECT/$1 $INTERFACE Progress)
            #echo "cur_progress: $cur_progress"
         if [ "$cur_progress" = "y 100" ]; then
             echo "bios upgrade process is 100%!"
             break
         fi

            if [ $progress -lt 90 ]
            then
                #echo "progress: $progress"
                busctl set-property xyz.openbmc_project.Software.BMC.Updater $OBJECT/$1 $INTERFACE Progress y $progress
	    fi
            let progress=progress+2
        else
            break
        fi
        let num=num-1
        sleep 1
done

echo "monitor process is done"
exit 0;


