SET(LIB_SDEX_SRCS
libSensorDeviceExample.cpp)

find_package (Threads)

ADD_LIBRARY(SensorDeviceExample STATIC ${LIB_SDEX_SRCS})
TARGET_LINK_LIBRARIES(SensorDeviceExample SensorDevices DeviceGateway pthread time_funcs)

