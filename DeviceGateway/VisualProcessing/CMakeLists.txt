SET(DGW_SIMPLE_VISUAL_SRCS
Communication.cpp
SimpleProcessing.cpp
simple_visual.cpp)

SET(DGW_EXP_REC_SRCS
../FaceProcessing/src/Communication.cpp
SimpleProcessing.cpp
expRecKinect.cpp)

SET(DGW_SCR_ACT_REC_SRCS
SimpleProcessing.cpp
scrActKinect.cpp)

find_package( OpenCV REQUIRED )

include_directories( ${OpenCV_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/libDGwClient ${CMAKE_SOURCE_DIR}/libTUSAlgos/libExprRec/inc ${CMAKE_SOURCE_DIR}/libTUSAlgos/libScrActRec/inc /usr/local/include/libfreenect /usr/include/libusb-1.0/ ${CMAKE_SOURCE_DIR}/libVisual/include)

link_directories(/usr/local/lib /usr/lib ${CMAKE_BINARY_DIR}/libTUSAlgos/libExprRec ${CMAKE_BINARY_DIR}/libTUSAlgos/libScrActRec)

ADD_EXECUTABLE(simple_visual ${DGW_SIMPLE_VISUAL_SRCS})
TARGET_LINK_LIBRARIES(simple_visual libDGwClientAV curl_helper time_funcs ${OpenCV_LIBS})

ADD_EXECUTABLE(expRecKinect ${DGW_EXP_REC_SRCS})
TARGET_LINK_LIBRARIES(expRecKinect libDGwClientAV TUSExprRec freenect curl_helper time_funcs ${OpenCV_LIBS})
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/libTUSAlgos/libExprRec/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(scrActKinect ${DGW_SCR_ACT_REC_SRCS})
TARGET_LINK_LIBRARIES(scrActKinect libDGwClientAV TUSScrActRec freenect freenect_sync curl_helper time_funcs X11 Xtst ${OpenCV_LIBS})
