# Machine name for release
MACHINE_NAME ?= ""
PLATFORM_ID ?= ""
BMC_IMAGE ?= "BC"
BMC_VERSION = "v2.12.0"
BOOT_VERSION = "v2019.04"

OS_RELEASE_FIELDS = "ID ID_LIKE NAME VERSION VERSION_ID PRETTY_NAME BMC_IMAGE_NAME PLATFORM_NAME BOOT_VERSION"

# Replace VERSION_ID so that it can carry more meaningful information
VERSION_ID = "${BMC_VERSION}-${@run_git(d, 'describe --long')}"

# Replace PRETTY_NAME to add MACHINE_NAME and VERSION_ID
PRETTY_NAME = "${MACHINE_NAME} ${VERSION_ID}! (Base: ${DISTRO_NAME} ${VERSION})"
PLATFORM_NAME = "${PLATFORM_ID}"
BMC_IMAGE_NAME = "${PLATFORM_ID}.${BMC_IMAGE}.${VERSION_ID}"

