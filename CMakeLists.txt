cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(hazard C CXX)
include("${VITASDK}/share/vita.cmake" REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2_mixer REQUIRED IMPORTED_TARGET SDL2_mixer)
pkg_check_modules(SDL2_image REQUIRED IMPORTED_TARGET SDL2_image)
pkg_check_modules(SDL2_ttf REQUIRED IMPORTED_TARGET SDL2_ttf)
pkg_check_modules(SDL2_net REQUIRED IMPORTED_TARGET SDL2_net)
pkg_check_modules(sndfile REQUIRED IMPORTED_TARGET sndfile)
pkg_check_modules(vorbisfile REQUIRED IMPORTED_TARGET vorbisfile)
pkg_check_modules(openal REQUIRED IMPORTED_TARGET openal)
set(VITA_APP_NAME "Beat Hazard 2")
set(VITA_TITLEID  "BTHAZARD2")
set(VITA_MKSFOEX_FLAGS "-d ATTRIBUTE2=4")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wl,-q,--wrap,memcpy,--wrap,memmove,--wrap,memset,--allow-multiple-definition -D_GNU_SOURCE -Wall -O3 -fdiagnostics-color=always -fno-optimize-sibling-calls -mfloat-abi=softfp")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -Wno-write-strings -fpermissive -fno-rtti")

add_executable(hazard
  loader/main.c
  loader/dialog.c
  loader/so_util.c
  loader/sha1.c
  loader/ctype_patch.c
  loader/bass_reimpl.cpp
)


# Reserve space for vita-elf-create import metadata between load segments.
set_property(TARGET hazard APPEND_STRING PROPERTY LINK_FLAGS " -Wl,--defsym=__sce_headroom=0x10000")

target_link_libraries(hazard
  -pthread
  PkgConfig::SDL2_mixer
  PkgConfig::SDL2_image
  PkgConfig::SDL2_ttf
  PkgConfig::SDL2_net
  PkgConfig::sndfile
  PkgConfig::vorbisfile
  PkgConfig::openal
  SDL2_mixer
  SDL2_image
  SDL2_ttf
  SDL2_net
  freetype
  soloud
  SDL2
  png
  webp
  jpeg
  mikmod
  z
  m
  stdc++
  OpenSLES
  sndfile
  vorbisfile
  vorbis
  vorbisenc
  ogg
  opusfile
  opus
  modplug
  FLAC
  openal
  imgui
  vitaGL
  vitashark
  SceShaccCgExt
  mathneon
  mpg123
  taihen_stub
  kubridge_stub
  SceNet_stub
  SceNetCtl_stub
  SceAppMgr_stub
  SceAppUtil_stub
  SceAudio_stub
  SceAudioIn_stub
  SceCtrl_stub
  SceCommonDialog_stub
  SceDisplay_stub
  SceFios2_stub
  SceGxm_stub
  SceIme_stub
  SceShaccCg_stub
  SceSysmodule_stub
  ScePower_stub
  SceTouch_stub
  SceMotion_stub
  SceHid_stub
  SceVshBridge_stub
  SceKernelDmacMgr_stub
)

vita_create_self(eboot.bin hazard UNSAFE)
vita_create_vpk(hazard.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE ${CMAKE_SOURCE_DIR}/livearea/icon0.png sce_sys/icon0.png
       ${CMAKE_SOURCE_DIR}/livearea/pic0.png sce_sys/pic0.png
       ${CMAKE_SOURCE_DIR}/livearea/bg0.png sce_sys/livearea/contents/bg0.png
       ${CMAKE_SOURCE_DIR}/livearea/beat.png sce_sys/livearea/contents/beat.png
       ${CMAKE_SOURCE_DIR}/livearea/conf.png sce_sys/livearea/contents/conf.png
       ${CMAKE_SOURCE_DIR}/livearea/template.xml sce_sys/livearea/contents/template.xml
)

set_property(SOURCE loader/so_util.c APPEND PROPERTY COMPILE_OPTIONS -Wno-error=return-mismatch -Wno-error=incompatible-pointer-types)

set_property(SOURCE loader/main.c APPEND PROPERTY COMPILE_OPTIONS -std=gnu17)
