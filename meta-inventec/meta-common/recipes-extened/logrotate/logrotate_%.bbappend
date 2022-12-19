LOGROTATE_CONF = "/etc/logrotate.d/logrotate.rsyslog"
LOGROTATE_SYSTEMD_TIMER_BASIS = "minutely"
LOGROTATE_SYSTEMD_TIMER_ACCURACY = "1m"
LOGROTATE_SYSTEMD_TIMER_PERSISTENT = "false"

do_install:append(){
    sed -ri \
        -e 's|( ).*$|\1${LOGROTATE_CONF}|g' \
        ${D}${systemd_system_unitdir}/logrotate.service
}