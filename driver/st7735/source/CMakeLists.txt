add_library(spi_st7735 st7735.c)

target_include_directories(spi_st7735 PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../include)

target_link_libraries(spi_st7735 pico_stdlib hardware_spi)