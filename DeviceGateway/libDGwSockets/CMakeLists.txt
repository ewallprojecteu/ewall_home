SET(LIB_DGWSOCKET_SRCS
libDGwSockets.cpp)

ADD_LIBRARY(DGwSockets STATIC ${LIB_DGWSOCKET_SRCS})
TARGET_LINK_LIBRARIES(DGwSockets DGwClient DeviceGateway json_spirit_dgw)
