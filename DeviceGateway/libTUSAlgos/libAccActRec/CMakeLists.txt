set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2"  )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2"  )

SET(LIB_ACCACTREC_SRC
src/cTUSAccActRec.cpp
src/AR.cpp
src/HAR_Accel.cpp
src/HAR_Track.cpp
src/svm.cpp
)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libTUSAlgos/libAccActRec/inc)
ADD_LIBRARY(TUSAccActRec STATIC ${LIB_ACCACTREC_SRC})

