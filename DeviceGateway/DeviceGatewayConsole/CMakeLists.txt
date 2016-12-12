SET(DGW_CONSOLE_SRCS
DeviceGatewayConsole.cpp)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libAVDevices ${CMAKE_SOURCE_DIR}/libSensorDevices/libSensorDeviceArduino ${CMAKE_SOURCE_DIR}/libSensorDevices/libSensorDeviceWaspMote ${CMAKE_SOURCE_DIR}/libSensorDevices/libSensorDevicePrerecorded ${CMAKE_SOURCE_DIR}/libAVDevices ${CMAKE_SOURCE_DIR}/libDeviceGateway)

ADD_EXECUTABLE(DGWConsole ${DGW_CONSOLE_SRCS})
TARGET_LINK_LIBRARIES(DGWConsole SensorDeviceArduino SensorDeviceWaspMote SensorDevicePrerecorded AVKinect DeviceGateway)

INSTALL(FILES mote.cfg motes.cfg sample-data.txt motesArduino.cfg kinect.cfg DESTINATION ${CMAKE_BINARY_DIR}/DeviceGatewayConsole/)

SET(DGW_CONSOLEPRREC_SRCS
DGwConsolePrerecorded.cpp)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libAVDevices ${CMAKE_SOURCE_DIR}/libSensorDevices/libSensorDevicePrerecorded ${CMAKE_SOURCE_DIR}/libDeviceGateway)

ADD_EXECUTABLE(DGWConsolePrRec ${DGW_CONSOLEPRREC_SRCS})
TARGET_LINK_LIBRARIES(DGWConsolePrRec SensorDevicePrerecorded AVDeviceExample DeviceGateway)

SET(DGW_CONSOLEWM_SRCS
DeviceGatewayConsoleWM.cpp)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libAVDevices ${CMAKE_SOURCE_DIR}/libSensorDevices/libSensorDeviceWaspMote ${CMAKE_SOURCE_DIR}/libDeviceGateway)

ADD_EXECUTABLE(DGWConsoleWM ${DGW_CONSOLEWM_SRCS})
TARGET_LINK_LIBRARIES(DGWConsoleWM SensorDeviceWaspMote AVDeviceExample DeviceGateway)

SET(DGW_CONSOLEARDUINO_SRCS
DeviceGatewayConsoleArduino.cpp)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libAVDevices ${CMAKE_SOURCE_DIR}/libSensorDevices/libSensorDeviceArduino ${CMAKE_SOURCE_DIR}/libDeviceGateway)

ADD_EXECUTABLE(DGWConsoleArduino ${DGW_CONSOLEARDUINO_SRCS})
TARGET_LINK_LIBRARIES(DGWConsoleArduino SensorDeviceArduino AVDeviceExample DeviceGateway)

SET(DGW_KINECTTEST_SRCS
DGwKinectTest.cpp)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libAVDevices ${CMAKE_SOURCE_DIR}/libDeviceGateway)

ADD_EXECUTABLE(DGwKinectTest ${DGW_KINECTTEST_SRCS})
TARGET_LINK_LIBRARIES(DGwKinectTest AVKinect DeviceGateway)

IF(BUILD_BLUEZ)

SET(DGW_CONSOLECA_SRCS
DGwConsoleCA.cpp)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libSensorDevices/libSensorDeviceBlueZ ${CMAKE_SOURCE_DIR}/libDeviceGateway)

ADD_EXECUTABLE(DGwConsoleCA ${DGW_CONSOLECA_SRCS})
TARGET_LINK_LIBRARIES(DGwConsoleCA SensorDeviceBlueZ DeviceGateway)

SET(ANTIDOTESRC_DIR ${CMAKE_SOURCE_DIR}/antidote-master/src)
SET(ANTIDOTECPLUG_DIR ${CMAKE_SOURCE_DIR}/antidote-master/src/communication/plugin)

SET(NONIN_IEEEAGT_SRCS
nonin_IEEE_agent.c ${ANTIDOTECPLUG_DIR}/plugin_tcp_agent.o)

INCLUDE_DIRECTORIES(${ANTIDOTESRC_DIR})

ADD_EXECUTABLE(nonin_ieee_agent ${NONIN_IEEEAGT_SRCS})
TARGET_LINK_LIBRARIES(nonin_ieee_agent antidote pthread bluetooth)

ENDIF(BUILD_BLUEZ)

IF(BUILD_WASP)
ADD_DEFINITIONS(-DWASPMOTES)
ENDIF(BUILD_WASP)

SET(DGW_CONSOLEALL_SRCS
DeviceGatewayConsoleAll.cpp)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libAVDevices ${CMAKE_SOURCE_DIR}/libSensorDevices/libSensorDeviceArduino ${CMAKE_SOURCE_DIR}/libSensorDevices/libSensorDeviceWaspMote  ${CMAKE_SOURCE_DIR}/libSensorDevices/libSensorDeviceBlueZ ${CMAKE_SOURCE_DIR}/libDeviceGateway)

ADD_EXECUTABLE(dgw_console_all ${DGW_CONSOLEALL_SRCS})
TARGET_LINK_LIBRARIES(dgw_console_all SensorDeviceArduino SensorDeviceWaspMote SensorDeviceBlueZ AVKinect DeviceGateway)
