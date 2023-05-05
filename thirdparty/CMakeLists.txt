set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Dtimegm=mktime")
set(minmea_SRCS minmea/minmea.c minmea/minmea.h)
add_library(minmea ${minmea_SRCS})
target_link_libraries(minmea)