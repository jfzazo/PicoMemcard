cmake_minimum_required(VERSION 3.13)
option(USE_SDCARD "Use SD Card design" OFF)
option(RP2040ZERO "Use RP2040-zero" ON)

# cmake -DDBG_LEVEL=2 ..
# Use debugging level (0=No debug; 1=info messages; 2=verbose messages)
set(DBG_LEVEL 2)


set(PICOPLATFORM rp2040)
if(NOT RP2040ZERO)
  set(PICO_BOARD pico_w)
else()
  set(PICO_BOARD waveshare_rp2040_zero)
endif()
include(pico_sdk_import.cmake)

project(picomemcard_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_executable(PicoMemcard)
target_compile_definitions(PicoMemcard PUBLIC DBG_LEVEL=${DBG_LEVEL})

pico_sdk_init()


if(PICO)
  message("Pico Configuration")
  add_definitions(-DPICO)
  set(PIN_DAT 5)
  set(PIN_CMD 6)
  set(PIN_SEL 7)
  set(PIN_CLK 8)
  set(PIN_ACK 9)
elseif(RP2040ZERO)
  message("RP2040-zero configuration")
  add_definitions(-DRP2040ZERO)
  set(PIN_DAT 9)
  set(PIN_CMD 10)
  set(PIN_SEL 11)
  set(PIN_CLK 12)
  set(PIN_ACK 13)
endif()

if(USE_SDCARD)
  add_definitions(-DUSE_SDCARD)
  message("Using the design with MicroSD")
else()
  message("Using the design without MicroSD")
endif()

pico_set_linker_script(PicoMemcard ${CMAKE_SOURCE_DIR}/memmap.ld)
configure_file(${CMAKE_CURRENT_LIST_DIR}/psxSPI.pio.in ${CMAKE_CURRENT_LIST_DIR}/psxSPI.pio @ONLY)
pico_generate_pio_header(PicoMemcard ${CMAKE_CURRENT_LIST_DIR}/psxSPI.pio)
pico_generate_pio_header(PicoMemcard ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

add_definitions(-DNO_PICO_LED)  # prevent SD SPI library from using LED

file(COPY
    ${CMAKE_SOURCE_DIR}/deps/overlay/
    DESTINATION ${CMAKE_SOURCE_DIR}/deps/
)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI PicoMemcard)
# add_subdirectory(poc_examples)

# Example source
target_sources(PicoMemcard PUBLIC
    ${CMAKE_SOURCE_DIR}/src/led.c
    ${CMAKE_SOURCE_DIR}/src/debug.c
    ${CMAKE_SOURCE_DIR}/src/main.c
    ${CMAKE_SOURCE_DIR}/src/memcard_manager.c
    ${CMAKE_SOURCE_DIR}/src/memcard_simulator.c
    ${CMAKE_SOURCE_DIR}/src/memory_card.c
    ${CMAKE_SOURCE_DIR}/src/msc_handler.c
    ${CMAKE_SOURCE_DIR}/src/usb_descriptors.c
    ${CMAKE_SOURCE_DIR}/src/fs/fs.c
    ${CMAKE_SOURCE_DIR}/src/fs/sd/sd_config.c
    ${CMAKE_SOURCE_DIR}/src/fs/sd/sd_wrapper.c
    ${CMAKE_SOURCE_DIR}/src/fs/flash/ram_disk.c
    ${CMAKE_SOURCE_DIR}/src/fs/flash/lfs_disk.c
    ${CMAKE_SOURCE_DIR}/src/fs/flash/lfs_flash_handler.c
    ${CMAKE_SOURCE_DIR}/src/fs/flash/flash_wrapper.c
    ${CMAKE_SOURCE_DIR}/deps/littlefs/lfs.c
    ${CMAKE_SOURCE_DIR}/deps/littlefs/lfs_util.c
)

# Example include
target_include_directories(PicoMemcard PUBLIC
    ${CMAKE_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/deps/littlefs
)

pico_enable_stdio_usb(PicoMemcard 1)	# enable usb stdio
pico_enable_stdio_uart(PicoMemcard 0)	# enable UART stdio

target_link_libraries(
  PicoMemcard
  pico_stdlib
  pico_multicore
  pico_time
  hardware_pio
  tinyusb_device
  tinyusb_board
  FatFs_SPI

  hardware_adc
  hardware_structs
)

if(PICO)
  target_link_libraries(
    pico_cyw43_arch_none # If you do not need the TCP/IP stack but wish to use the on-board LED on a Pico W
  )
endif()

pico_add_extra_outputs(PicoMemcard)
