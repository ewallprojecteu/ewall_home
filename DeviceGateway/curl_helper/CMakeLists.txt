SET(CURL_OPS_SRCS
curl_helper.c)

ADD_LIBRARY(curl_helper STATIC ${CURL_OPS_SRCS})
TARGET_LINK_LIBRARIES(curl_helper curl)
