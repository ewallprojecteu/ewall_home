SET(LIB_AVKINECT_SRCS
libAVKinect.cpp)

IF(BUILD_KINECT2)
    SET(FREENECT2_INCLUDE_DIRS /usr/local/include/libfreenect2 /usr/local/include/libfreenect2/tinythread)
    SET(FREENECT2_LIBS freenect2)
    ADD_DEFINITIONS(-DKINECT_V2)
ELSEIF(BUILD_KINECT2)
    SET(FREENECT2_INCLUDE_DIRS )
    SET(FREENECT2_LIBS )
    ADD_DEFINITIONS(-DKINECT_V2)
ENDIF(BUILD_KINECT2)
#message("DEBUG INFO:")
#message("FREENECT2_INCLUDE_DIRS: " '${FREENECT2_INCLUDE_DIRS}')
#message("FREENECT2_LIBS: " '${FREENECT2_LIBS}')

#find_package (freenect)
include_directories (/usr/local/include/libusb-1.0/ /usr/local/include/libfreenect ${FREENECT2_INCLUDE_DIRS} /usr/include/opencv)
link_directories(/usr/local/lib /usr/lib)
ADD_LIBRARY(AVKinect STATIC ${LIB_AVKINECT_SRCS})
TARGET_LINK_LIBRARIES(AVKinect AVDevice DeviceGateway freenect ${FREENECT2_LIBS} opencv_core opencv_highgui opencv_imgproc)

SET(LIB_AVDEVEX_SRCS
libAVDeviceExample.cpp)

ADD_LIBRARY(AVDeviceExample STATIC ${LIB_AVDEVEX_SRCS})
TARGET_LINK_LIBRARIES(AVDeviceExample AVDevice DeviceGateway)

SET(LIB_AVDEV_SRCS
libAVDevice.cpp)

ADD_LIBRARY(AVDevice STATIC ${LIB_AVDEV_SRCS})
TARGET_LINK_LIBRARIES(AVDevice DeviceGateway)


