IF(BUILD_BLUEZ)
    SET(BLUEZ_DIR libSensorDeviceBlueZ)
ELSEIF(BUILD_BLUEZ)
    SET(BLUEZ_DIR )
ENDIF(BUILD_BLUEZ)


SUBDIRS(libSensorDeviceExample libSensorDevicePrerecorded libSensorDeviceWaspMote libSensorDeviceArduino ${BLUEZ_DIR})

SET(LIB_SD_SRCS
libSensorDevices.cpp)

ADD_LIBRARY(SensorDevices STATIC ${LIB_SD_SRCS})
TARGET_LINK_LIBRARIES(SensorDevices DeviceGateway)
