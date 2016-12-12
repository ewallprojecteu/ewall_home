SET(LIB_SDPR_SRCS
libSensorDevicePrerecorded.cpp)

find_package (Threads)

ADD_LIBRARY(SensorDevicePrerecorded STATIC ${LIB_SDPR_SRCS})
TARGET_LINK_LIBRARIES(SensorDevicePrerecorded SensorDevices DeviceGateway pthread time_funcs)

