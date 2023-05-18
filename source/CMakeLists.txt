add_executable(pathfinder
    boot.c
    #hooks.c
    xoroshiro64star.c
)

target_compile_definitions(pathfinder PRIVATE
	FREE_RTOS_KERNEL_SMP=1
	portSUPPORT_SMP=1
)

target_include_directories(pathfinder PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../include
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(pathfinder    pathfinder_hal
                                    pico_stdlib 
                                    #FreeRTOS-Kernel
                                    #FreeRTOS-Kernel-Heap4
                                    hagl
                                    hagl_hal
)

target_link_options(pathfinder PRIVATE -Xlinker --print-memory-usage)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(pathfinder)

pico_enable_stdio_usb(pathfinder 1)
pico_enable_stdio_uart(pathfinder 0)
