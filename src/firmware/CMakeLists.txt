cmake_minimum_required(VERSION 3.13)

if(EMULATOR OR DEFINED ENV{EMULATOR})
  set(BUILD_FOR_PICO FALSE)
else()
  if(DEFINED ENV{PICO_SDK_PATH} OR PICO_SDK_PATH)
    set(BUILD_FOR_PICO TRUE)
  else()
    set(BUILD_FOR_PICO FALSE)
  endif()
endif()

project(mp3_player C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CORE_SOURCES
    src/app/main.c
    src/app/gfx.c
    src/app/library.c
    src/app/player.c
    src/app/theme.c

    # fonts
    src/ui/fonts/font_lexend.c
    src/ui/fonts/font_cozette.c
    src/ui/fonts/font_notosans.c
    src/ui/fonts/font_ibmplexmono.c
    src/ui/fonts/font_lexend64.c

    # other things ig
    src/ui/screen.c
    src/ui/widget.c
    src/ui/anim.c
    src/ui/screens/nowplaying.c
    src/ui/screens/library.c
    src/ui/layout/container.c
    src/ui/layout/vbox.c
    src/ui/layout/hbox.c
    src/ui/components/label.c
    src/ui/components/list.c
    src/ui/components/image.c
    src/ui/components/now_playing_bar.c
)

include_directories(src/app)

if(BUILD_FOR_PICO)
  include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
  message(STATUS "building for actual hardware")
  pico_sdk_init()

  set(PICO_HAL_SOURCES
        src/hal_pico/hal_pico.c
        src/hal_pico/hal_bt_pico.c
    )

  add_executable(firmware ${CORE_SOURCES} ${PICO_HAL_SOURCES})

  target_link_libraries(firmware pico_stdlib pico_audio)
  pico_add_extra_outputs(firmware)

else()
  message(STATUS "building for emulator")

  find_package(SDL2 REQUIRED)
  include_directories(${SDL2_INCLUDE_DIRS})

  set(EMULATOR_HAL_SOURCES
        src/hal_emulator/hal_desktop.c
        src/hal_emulator/hal_bt_mock.c
    )

  add_executable(emulator ${CORE_SOURCES} ${EMULATOR_HAL_SOURCES})

  target_link_libraries(emulator ${SDL2_LIBRARIES})

endif()
