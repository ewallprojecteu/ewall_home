find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/libDGwClient ${CMAKE_SOURCE_DIR}/libVisual/include ${CMAKE_SOURCE_DIR}/libTUSAlgos/libExprRec/inc)
link_directories(/usr/local/lib /usr/lib ${CMAKE_BINARY_DIR}/libTUSAlgos/libExprRec)

SET(TEST_SENDING_SRCS
./src/Communication.cpp
./src/TestSending.cpp)

ADD_EXECUTABLE(test_sending ${TEST_SENDING_SRCS})
TARGET_LINK_LIBRARIES(test_sending curl_helper time_funcs ${OpenCV_LIBS})

SET(FACE_PROCESSING_SRCS
./src/Communication.cpp
./src/FaceProcessing.cpp
./src/ExpRecogniser.cpp
./src/main.cpp)

SET(FACE_PROCESSING_LIBS
curl_helper time_funcs Visual TUSExprRec)

ADD_EXECUTABLE(face_processing ${FACE_PROCESSING_SRCS})
TARGET_LINK_LIBRARIES(face_processing libDGwClientAV ${FACE_PROCESSING_LIBS} ${OpenCV_LIBS})

ADD_EXECUTABLE(face_processing_noDGW ${FACE_PROCESSING_SRCS})
TARGET_LINK_LIBRARIES(face_processing_noDGW ${FACE_PROCESSING_LIBS} ${OpenCV_LIBS})
SET_TARGET_PROPERTIES(face_processing_noDGW PROPERTIES COMPILE_FLAGS -DNO_DEVICEGATEWAY)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/bin/AgeGenderData.bin
${CMAKE_CURRENT_SOURCE_DIR}/bin/haarcascade_frontalface_alt.xml
${CMAKE_CURRENT_SOURCE_DIR}/bin/haarcascade_mcs_lefteye.xml
${CMAKE_CURRENT_SOURCE_DIR}/bin/haarcascade_mcs_righteye.xml
${CMAKE_CURRENT_SOURCE_DIR}/bin/settings.txt
DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

INSTALL(FILES ${CMAKE_SOURCE_DIR}/libTUSAlgos/libExprRec/models/LGADASVM/kernels_features.yml
${CMAKE_SOURCE_DIR}/libTUSAlgos/libExprRec/models/LGADASVM/KSVM_scale_range.yml
${CMAKE_SOURCE_DIR}/libTUSAlgos/libExprRec/models/LGADASVM/SVMModel.model
DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/models)

#INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/libTUSAlgos/libExprRec/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

