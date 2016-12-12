SET(LIB_DGW_SRCS
libDeviceGateway.cpp)

ADD_LIBRARY(DeviceGateway STATIC ${LIB_DGW_SRCS})
TARGET_LINK_LIBRARIES(DeviceGateway SensorDeviceExample AVDeviceExample DGwSockets json_spirit_dgw)
