cmake_minimum_required(VERSION 3.21)

project(sigmaker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(IDASDK_ROOT "$ENV{IDASDK}" CACHE PATH "Path to the Hex-Rays IDA SDK root")
set(SIGMAKER_OUTPUT_NAME "sigmaker" CACHE STRING "Plugin file name without extension")

if(NOT IDASDK_ROOT)
  message(FATAL_ERROR "Set the IDASDK environment variable or pass -DIDASDK_ROOT to the IDA SDK root directory.")
endif()

set(IDASDK_LAYOUT_CANDIDATES
  "${IDASDK_ROOT}"
  "${IDASDK_ROOT}/src"
)

unset(IDASDK_DIR)
foreach(candidate IN LISTS IDASDK_LAYOUT_CANDIDATES)
  if(EXISTS "${candidate}/include/ida.hpp")
    set(IDASDK_DIR "${candidate}")
    break()
  endif()
endforeach()

if(NOT IDASDK_DIR)
  message(FATAL_ERROR "IDASDK_ROOT does not look valid: ${IDASDK_ROOT}")
endif()

set(SIGMAKER_SOURCES
  Main.cpp
  Plugin.cpp
  SignatureUtils.cpp
  Utils.cpp
)

add_library(sigmaker MODULE ${SIGMAKER_SOURCES})

target_include_directories(sigmaker PRIVATE "${IDASDK_DIR}/include")
target_compile_features(sigmaker PRIVATE cxx_std_20)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_compile_definitions(sigmaker PRIVATE __MAC__ __EA64__)
  set_target_properties(sigmaker PROPERTIES
    PREFIX ""
    OUTPUT_NAME "${SIGMAKER_OUTPUT_NAME}"
    SUFFIX ".dylib"
  )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_compile_definitions(sigmaker PRIVATE __EA64__)
  set_target_properties(sigmaker PROPERTIES
    PREFIX ""
    OUTPUT_NAME "${SIGMAKER_OUTPUT_NAME}64"
    SUFFIX ".dll"
  )
else()
  message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

if(MSVC)
  target_compile_options(sigmaker PRIVATE /W3 /MP /EHsc)
else()
  target_compile_options(sigmaker PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  if(NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "${CMAKE_HOST_SYSTEM_PROCESSOR}" CACHE STRING "macOS target architectures" FORCE)
  endif()

  list(LENGTH CMAKE_OSX_ARCHITECTURES SIGMAKER_ARCH_COUNT)
  if(SIGMAKER_ARCH_COUNT GREATER 1)
    message(FATAL_ERROR "Universal macOS builds are not supported by this CMake file. Configure one architecture at a time.")
  endif()

  if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
    set(IDA_LIBRARY_CANDIDATES
      "${IDASDK_DIR}/lib/arm64_mac_clang_64/ida.a"
      "${IDASDK_DIR}/lib/arm64_mac_clang_64_pro/ida.a"
      "${IDASDK_DIR}/lib/arm64_mac_clang_64/libida.dylib"
      "${IDASDK_DIR}/lib/arm64_mac_clang_64_pro/libida.dylib"
    )
  elseif(CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64" OR CMAKE_OSX_ARCHITECTURES STREQUAL "amd64")
    set(IDA_LIBRARY_CANDIDATES
      "${IDASDK_DIR}/lib/x64_mac_clang_64/ida.a"
      "${IDASDK_DIR}/lib/x64_mac_clang_64_pro/ida.a"
      "${IDASDK_DIR}/lib/x64_mac_clang_64/libida.dylib"
      "${IDASDK_DIR}/lib/x64_mac_clang_64_pro/libida.dylib"
    )
  else()
    message(FATAL_ERROR "Unsupported macOS architecture list: ${CMAKE_OSX_ARCHITECTURES}")
  endif()

  unset(IDA_LIBRARY)
  foreach(candidate IN LISTS IDA_LIBRARY_CANDIDATES)
    if(EXISTS "${candidate}")
      set(IDA_LIBRARY "${candidate}")
      break()
    endif()
  endforeach()

  if(NOT IDA_LIBRARY)
    list(JOIN IDA_LIBRARY_CANDIDATES "\n  " IDA_LIBRARY_HINTS)
    message(FATAL_ERROR "Could not find an IDA macOS library. Looked in:\n  ${IDA_LIBRARY_HINTS}")
  endif()

  target_link_libraries(sigmaker PRIVATE "${IDA_LIBRARY}")
  target_link_options(sigmaker PRIVATE
    "-Wl,-exported_symbol,_PLUGIN"
    "-Wl,-exported_symbol,_sigmaker_generate"
  )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_compile_definitions(sigmaker PRIVATE __NT__ __EA64__)
  find_library(IDA_LIBRARY
    NAMES ida
    PATHS
      "${IDASDK_ROOT}/lib/x64_win_vc_64"
      "${IDASDK_DIR}/lib/x64_win_vc_64"
      "${IDASDK_DIR}/lib/x64_win_vc_64_pro"
    NO_DEFAULT_PATH
  )

  if(NOT IDA_LIBRARY)
    message(FATAL_ERROR "Could not find ida.lib under ${IDASDK_DIR}/lib")
  endif()

  target_link_libraries(sigmaker PRIVATE "${IDA_LIBRARY}")
  target_link_options(sigmaker PRIVATE /EXPORT:PLUGIN /EXPORT:sigmaker_generate)
endif()
