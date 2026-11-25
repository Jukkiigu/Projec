
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(APP_NAME JTKJ_app) 

set(MAIN_TARGET hat_app)



# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work =======
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ================================================================================================

# == DO NOT EDIT:  SETUP pico SDK and create project ==============================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(${APP_NAME} C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

include(FreeRTOS_Kernel_import.cmake)


target_include_directories(FreeRTOS-Kernel INTERFACE
${CMAKE_CURRENT_LIST_DIR}/config
)

add_subdirectory(libs/TKJHAT)
add_subdirectory(examples/hat_imu_ex)
add_subdirectory(libs/usb-serial-debug)

add_subdirectory(examples/hello_blink_freertos)

add_executable(${MAIN_TARGET}
    src/main.c
)


target_link_libraries(${MAIN_TARGET}
        pico_stdlib
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
        TKJHAT_SDK
)

# Include libraries necessaries to control the WiFi. If you are using the internal pico LED in W model, it is also 
# necessary 
if (PICO_CYW43_SUPPORTED)
    target_link_libraries(${MAIN_TARGET} pico_cyw43_arch_none)
endif()

#Support for stdio (printf, fwrite, puts...) via usb or UART. 
pico_enable_stdio_usb(${MAIN_TARGET} 1)
pico_enable_stdio_uart(${MAIN_TARGET} 0)

pico_add_extra_outputs(${MAIN_TARGET})

# add url via pico_set_program_url
pico_set_program_url(${MAIN_TARGET} https://github.com/UniOulu-Ubicomp-Programming-Courses/JTKJ-PicoRTOS-ProjectP)
# ==================================================================================
