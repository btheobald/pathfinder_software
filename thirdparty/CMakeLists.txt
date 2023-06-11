set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Dtimegm=mktime")

add_subdirectory(hagl)

add_library(mcufont mcufont/decoder/mf_bwfont.c
                    mcufont/decoder/mf_encoding.c
                    mcufont/decoder/mf_font.c
                    mcufont/decoder/mf_justify.c
                    mcufont/decoder/mf_kerning.c
                    mcufont/decoder/mf_rlefont.c
                    mcufont/decoder/mf_scaledfont.c
                    mcufont/decoder/mf_wordwrap.c
)

target_include_directories(mcufont PUBLIC   ${CMAKE_CURRENT_LIST_DIR}/mcufont/decoder
                                            ${CMAKE_CURRENT_LIST_DIR}/mcufont/fonts
)

add_subdirectory(no-OS-FatFS-SD-SPI-RPi-Pico/src)