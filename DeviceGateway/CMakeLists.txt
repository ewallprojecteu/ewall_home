CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(device_gateway)

#set(CMAKE_C_COMPILER "gcc")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC"  )
#set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC"  )

OPTION(BUILD_KINECT2 "Use freenect2 for Kinect2 support (also sets KINECT_V2 to switch library to Kinect2) " ON)
OPTION(BUILD_BLUEZ "Use blueZ (requires antidote)" ON)
OPTION(BUILD_WASP "Use Waspmotes instead of Arduinos" OFF)

SUBDIRS(json_spirit libSensorDevices libAVDevices libDeviceGateway libDGwClient libDGwSockets libTUSAlgos DeviceGatewayConsole DGwClientConsole json_spirit_dgw time_funcs curl_helper poseFaceProcessing Accelerometer VisualProcessing libVisual FaceProcessing libDeviceGatewayJavaWrapper libDGwClientJavaWrapper)

INCLUDE_DIRECTORIES(includeDir json_spirit)

message("")
message("************** Configuration options **************")
message("**")
message("** BUILD_KINECT2: " ${BUILD_KINECT2})
message("**")
message("** BUILD_BLUEZ: " ${BUILD_BLUEZ})
message("**")
message("** BUILD_WASP: " ${BUILD_WASP})
message("**")
message("***************************************************")
message("")

