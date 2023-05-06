set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Dtimegm=mktime")

add_library(minmea minmea/minmea.c)

target_include_directories(minmea PRIVATE ${CMAKE_CURRENT_LIST_DIR}/minmea)

target_link_libraries(minmea)