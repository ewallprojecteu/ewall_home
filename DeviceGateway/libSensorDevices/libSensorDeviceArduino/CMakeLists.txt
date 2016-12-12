SET(LIB_SDAR_SRCS
libSensorDeviceArduino.cpp)

find_package (Threads)

ADD_LIBRARY(SensorDeviceArduino STATIC ${LIB_SDAR_SRCS})
TARGET_LINK_LIBRARIES(SensorDeviceArduino SensorDevices DeviceGateway pthread time_funcs)
