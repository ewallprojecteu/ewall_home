SET(LIB_SDBLUEZ_SRCS
libSensorDeviceBlueZ.cpp)

SET(ANTIDOTESRC_DIR ${CMAKE_SOURCE_DIR}/antidote-master/src)
SET(ANTIDOTEAPP_DIR ${CMAKE_SOURCE_DIR}/antidote-master/apps)
SET(ANTIDOTEMAIN_DIR ${CMAKE_SOURCE_DIR}/antidote-master)
SET(ANTIDOTECPLUG_DIR ${CMAKE_SOURCE_DIR}/antidote-master/src/communication/plugin)

SET(ANTIDOTE_SRCS ${ANTIDOTEAPP_DIR}/healthd-healthd_ipc_tcp.o ${ANTIDOTEAPP_DIR}/healthd-healthd_ipc_auto.o ${ANTIDOTEAPP_DIR}/healthd-healthd_ipc_dbus.o ${ANTIDOTECPLUG_DIR}/usb/libusbplugin_la-plugin_usb.o ${ANTIDOTECPLUG_DIR}/usb/libusbplugin_la-usb_phdc_drive.o ${ANTIDOTECPLUG_DIR}/plugin_pthread.o)

find_package (Threads)
find_path (GLIB_INCLUDE1_DIR NAMES glib.h PATHS /usr/local/include /usr/include /usr/lib /usr/lib/x86_64-linux-gnu PATH_SUFFIXES glib-2.0/include glib-2.0)
find_path (GLIB_INCLUDE2_DIR NAMES glibconfig.h PATHS /usr/local/include /usr/include /usr/lib /usr/lib/x86_64-linux-gnu /usr/lib/arm-linux-gnueabihf PATH_SUFFIXES glib-2.0/include glib-2.0)

INCLUDE_DIRECTORIES(${ANTIDOTEMAIN_DIR} ${ANTIDOTESRC_DIR} ${ANTIDOTEAPP_DIR} ${CMAKE_SOURCE_DIR}/libSensorDevices/libSensorDeviceBlueZ ${GLIB_INCLUDE1_DIR} ${GLIB_INCLUDE2_DIR})

ADD_LIBRARY(SensorDeviceBlueZ STATIC ${LIB_SDBLUEZ_SRCS} ${ANTIDOTE_SRCS})

TARGET_LINK_LIBRARIES(SensorDeviceBlueZ SensorDevices DeviceGateway antidote bluezplugin pthread time_funcs glib-2.0 gio-2.0 dbus-1 dbus-glib-1 dbus-1 gobject-2.0 usb-1.0)

