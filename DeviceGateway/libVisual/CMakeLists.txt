SET(LIB_VISUAL_SRC
src/assignmentoptimal.cpp
src/detectObject.cpp
src/PF.cpp
src/recognition.cpp
src/videoCaptureAdv.cpp
src/colorhist.cpp
src/display.cpp
src/ImageUtils_0.7.cpp
src/preprocessFace.cpp
src/tools.cpp
src/tracker.cpp
src/VJ.cpp
src/genderReco.cpp
)

#suppress warning -Wformat-security in external file ImageUtils
#https://gcc.gnu.org/onlinedocs/gcc-3.4.4/gcc/Warning-Options.html
#http://stackoverflow.com/questions/13638408/cmake-override-compile-flags-for-single-files
set_source_files_properties(src/ImageUtils_0.7.cpp PROPERTIES COMPILE_FLAGS -Wno-format-security)

find_package( OpenCV REQUIRED )
INCLUDE_DIRECTORIES( ${OpenCV_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/libVisual/include)

ADD_LIBRARY(Visual STATIC ${LIB_VISUAL_SRC})
TARGET_LINK_LIBRARIES(Visual ${OpenCV_LIBS})

 
