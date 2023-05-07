add_executable(pathfinder
    boot.c
    pathfinder.c
    utility.c
    #hooks.c
)

target_compile_definitions(pathfinder PRIVATE
	FREE_RTOS_KERNEL_SMP=1
	portSUPPORT_SMP=1
)

target_include_directories(pathfinder PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../include
        ${CMAKE_CURRENT_LIST_DIR}/../driver/ms5637/include
        ${CMAKE_CURRENT_LIST_DIR}/../driver/st7735/include
        ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/minmea
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(pathfinder    pico_stdlib 
                                    hardware_spi 
                                    hardware_i2c 
                                    hardware_uart 
                                    hardware_pio 
                                    hardware_dma
                                    i2c_ms5637
                                    spi_st7735
                                   # FreeRTOS-Kernel
                                   # FreeRTOS-Kernel-Heap4
                                    minmea)

target_link_options(pathfinder PRIVATE -Xlinker --print-memory-usage)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(pathfinder)

pico_enable_stdio_usb(pathfinder 1)
pico_enable_stdio_uart(pathfinder 0)
