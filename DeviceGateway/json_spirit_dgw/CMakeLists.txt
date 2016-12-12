SET(JSON_SPIRIT_DGW_SRCS
json_spirit_dgw.cpp)

FIND_PACKAGE(Boost 1.34 REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/json_spirit)

ADD_LIBRARY(json_spirit_dgw STATIC ${JSON_SPIRIT_DGW_SRCS})
TARGET_LINK_LIBRARIES(json_spirit_dgw json_spirit)

SET(JSON_SPIRIT_TEST_SRCS
json_spirit_test.cpp)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/json_spirit_dgw)
ADD_EXECUTABLE(json_spirit_test ${JSON_SPIRIT_TEST_SRCS})
TARGET_LINK_LIBRARIES(json_spirit_test json_spirit_dgw)

INSTALL(FILES mote1.cfg avDevIn.cfg avDevsIn.cfg streamConfIn.cfg sensIn.cfg moteIn.cfg moteDetailsIn.cfg measIn.cfg DESTINATION ${CMAKE_BINARY_DIR}/json_spirit_dgw/)
