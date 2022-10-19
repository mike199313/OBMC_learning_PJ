RDEPENDS:${PN}-dbus:remove += " \
        python-dbus \
        python-xml \
        python-json \
        python-pickle \
        "
RDEPENDS:${PN}-dbus += " \
        ${PYTHON_PN}-dbus \
        ${PYTHON_PN}-xml \
        ${PYTHON_PN}-json \
        ${PYTHON_PN}-pickle \
        "

RDEPENDS:${PN} += " \
        ${PN}-ns \
        ${PN}-dbus \
        python-subprocess \
        python-dbus \
        "
RDEPENDS:${PN}:remove += " \
        python-subprocess \
        python-dbus \
        "
RDEPENDS:${PN} += " \
        ${PYTHON_PN}-dbus \
        "

