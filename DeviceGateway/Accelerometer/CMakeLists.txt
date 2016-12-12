SET(LIB_DGWACCELEROMETER_SRCS
libDGwAccelerometer.cpp)

find_package( OpenCV REQUIRED )
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/libDGwSockets ${CMAKE_SOURCE_DIR}/libDGwClient ${CMAKE_SOURCE_DIR}/libTUSAlgos/libAccActRec/inc)

ADD_LIBRARY(DGwAccelerometer STATIC ${LIB_DGWACCELEROMETER_SRCS})

link_directories(${CMAKE_BINARY_DIR}/libTUSAlgos/libAccActRec)

TARGET_LINK_LIBRARIES(DGwAccelerometer DGwClient curl TUSAccActRec ${OpenCV_LIBS})

SET(DGW_ACCELEROMETER_SRCS
main.cpp)

ADD_EXECUTABLE(accelerometer ${DGW_ACCELEROMETER_SRCS})

TARGET_LINK_LIBRARIES(accelerometer DGwAccelerometer curl_helper time_funcs)

INSTALL(FILES filterCoeffs.txt DESTINATION ${CMAKE_BINARY_DIR}/Accelerometer/)

INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/libTUSAlgos/libAccActRec/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
