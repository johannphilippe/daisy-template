cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.25)

#add_definitions(-DFILEIO_ENABLE_CSTDIO_READER)
add_definitions(-DFILEIO_ENABLE_FATFS_READER)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)


#file(TO_CMAKE_PATH $ENV{HOME} USER_HOME)
#file(TO_CMAKE_PATH $ENV{AUDIO_LIBS_DIR} AUDIO_LIBS_DIR)

set(LIBDAISY_PATH ${CMAKE_SOURCE_DIR}/lib/libDaisy)
set(DAISYSP_PATH ${CMAKE_SOURCE_DIR}/lib/DaisySP)
set(Q_PATH ${CMAKE_SOURCE_DIR}/lib/q)

message(STATUS "LIBDAISY_PATH: ${LIBDAISY_PATH}")
message(STATUS "DAISYSP_PATH: ${DAISYSP_PATH}")


# IMPORTANT - CHANGE THIS PATH TO YOUR GCC INSTALLATION
#set(TOOLCHAIN_PREFIX "$ENV{HOME}/Documents/Electroacoustique/daisy/gcc-arm-none-eabi-10-2020-q4-major/bin/arm-none-eabi-gcc")
set(TOOLCHAIN_PREFIX "$ENV{HOME}/Documents/Electroacoustique/daisy/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc")
set(CMAKE_TOOLCHAIN_FILE ${LIBDAISY_PATH}/cmake/toolchains/stm32h750xx.cmake)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(daisy_program VERSION 0.1 LANGUAGES C CXX ASM)

# This is the target name of the firmware binary
set(FIRMWARE_NAME daisy_firmware)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# OPENOCD
set(CHIPSET stm32h7x)
set(OCD openocd)

# IMPORTANT - CHANGE THIS PATH TO YOUR OPENOCD SCRIPTS FOLDER
set(OCD_DIR "${USER_HOME}/scoop/apps/openocd/current/share/openocd/scripts")
set(PGM_DEVICE interface/stlink.cfg)
set(OCDFLAGS -f ${PGM_DEVICE} -f target/${CHIPSET}.cfg)
set(OCD_PROGRAM ${OCD} -s ${OCD_DIR} ${OCDFLAGS} -c "program ${FIRMWARE_NAME}.elf verify reset exit")

add_subdirectory(${LIBDAISY_PATH})

# We're compiling libDaisy with -Os to save space
target_compile_options(daisy
    PUBLIC
    -Os
)

add_subdirectory(${DAISYSP_PATH}) 

set(Q_BUILD_IO OFF)
set(Q_BUILD_EXAMPLES OFF) 
set(Q_BUILD_TEST OFF)
add_subdirectory(${Q_PATH})
target_compile_options(libq INTERFACE -fconcepts)

add_executable(${FIRMWARE_NAME} src/main.cpp)

target_link_libraries(${FIRMWARE_NAME}
    PUBLIC
    daisy
    DaisySP
    libq
)



set(LINKER_SCRIPT ${LIBDAISY_PATH}/core/STM32H750IB_sram.lds)

set_target_properties(${FIRMWARE_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    SUFFIX ".elf"
    LINK_DEPENDS ${LINKER_SCRIPT}
)

target_link_options(${FIRMWARE_NAME} PUBLIC
    -T ${LINKER_SCRIPT}
    -Wl,-Map=${FIRMWARE_NAME}.map,--cref
    -Wl,--check-sections
    -Wl,--unresolved-symbols=report-all
    -Wl,--warn-common
    -Wl,--warn-section-align
    -Wl,--print-memory-usage
    --specs=nosys.specs
)

add_custom_command(TARGET ${FIRMWARE_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
    ARGS -O ihex
    -S ${FIRMWARE_NAME}.elf
    ${FIRMWARE_NAME}.hex
    BYPRODUCTS
    ${FIRMWARE_NAME}.hex
    COMMENT "Generating HEX image"
    VERBATIM)

add_custom_command(TARGET ${FIRMWARE_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
    ARGS -O binary
    -S ${FIRMWARE_NAME}.elf
    ${FIRMWARE_NAME}.bin
    BYPRODUCTS
    ${FIRMWARE_NAME}.bin
    COMMENT "Generating binary image"
    VERBATIM)

add_custom_target(flash-st-link
    COMMAND ${OCD_PROGRAM}
    DEPENDS ${FIRMWARE_NAME}
    COMMENT "Flashing ${FIRMWARE_NAME}.elf with ST-Link"
    VERBATIM
)

set(FLASH_ADDRESS 0x08000000)
set(TARGET_BIN ${FIRMWARE_NAME}.bin)
set(USBPID df11)

add_custom_target(flash-dfu
    COMMAND dfu-util -a 0 -s ${FLASH_ADDRESS}:leave -D ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_BIN} -d ,0483:${USBPID}
    DEPENDS ${FIRMWARE_NAME}
    COMMENT "Flashing ${FIRMWARE_NAME}.elf with DFU"
    VERBATIM
)

