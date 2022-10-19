FILESEXTRAPATHS:append := ":${THISDIR}/${PN}"
SRC_URI:append = " file://power_cap.override.yml \
                   file://boot.override.yml \
                   file://globalenables.override.yml \
                   file://sol-parameters.override.yml \                   
                 "
