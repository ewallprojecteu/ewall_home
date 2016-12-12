SET(LIB_SDWM_SRCS
libSensorDeviceWaspMote.cpp)

find_package (Threads)

ADD_LIBRARY(SensorDeviceWaspMote STATIC ${LIB_SDWM_SRCS})
TARGET_LINK_LIBRARIES(SensorDeviceWaspMote SensorDevices DeviceGateway pthread time_funcs)

