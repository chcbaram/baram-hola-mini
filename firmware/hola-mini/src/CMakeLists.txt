
include(ap/modules/qmk/CMakeLists.txt)


file(GLOB SRC_FILES 
  ap/*.cpp
  ap/*.c
  bsp/*.c
  hw/*.c
)

file(GLOB_RECURSE SRC_FILES_RECURSE
  common/*.c 
  hw/*.c
)

add_executable(
  ${PRJ_NAME}
  
  main.c

  ${SRC_FILES}
  ${SRC_FILES_RECURSE}
  ${QMK_SRC_FILES}
)

target_include_directories(${PRJ_NAME} PUBLIC 
  .
  ap
  ap/modules
  bsp
  common 
  common/core
  common/hw/include 
  hw
  hw/driver/usb
  lib

  ${QMK_INC_DIR}
)

target_link_libraries(${PRJ_NAME} 
  pico_stdlib
  hardware_spi
  hardware_dma
  hardware_irq
  hardware_i2c
  pico_multicore
  hardware_pio
  pico_unique_id
  tinyusb_device 
  tinyusb_board
  )

pico_add_extra_outputs(${PRJ_NAME})
pico_enable_stdio_usb(${PRJ_NAME}  0)
pico_enable_stdio_uart(${PRJ_NAME} 1)

pico_generate_pio_header(${PRJ_NAME} ${CMAKE_CURRENT_LIST_DIR}/hw/driver/pio/ws2812.pio)

set_target_properties(${PRJ_NAME} PROPERTIES SUFFIX ".elf")


