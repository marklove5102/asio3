#
# Copyright (c) 2017-2023 zhllxt
# 
# author   : zhllxt
# email    : 37792738@qq.com
# 
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
#
# export CC=/usr/local/bin/gcc
# export CXX=/usr/local/bin/g++
#

cmake_minimum_required (VERSION 2.8...3.20)



# remove last end of "/"
string(REGEX REPLACE "/$" "" CMAKELISTS_DIR_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# get current relative dir name and set target name
string(REGEX REPLACE ".*/(.*)" "\\1" CMAKELISTS_DIR_NAME ${CMAKELISTS_DIR_PATH})

# set root directory
set(ASIO3_ROOT_DIR ${CMAKELISTS_DIR_PATH})

message("ASIO3_ROOT_DIR = ${ASIO3_ROOT_DIR}")



#-------------------------------------------------------------------------------

function (DoGroupSources curdir rootdir foldername folder)
    file (GLOB children RELATIVE ${ASIO3_ROOT_DIR}/${curdir} ${ASIO3_ROOT_DIR}/${curdir}/*)
    foreach (child ${children})
        if (IS_DIRECTORY ${ASIO3_ROOT_DIR}/${curdir}/${child})
            DoGroupSources (${curdir}/${child} ${rootdir} ${foldername} ${folder})
        elseif (${child} STREQUAL "CMakeLists.txt")
            source_group("" FILES ${ASIO3_ROOT_DIR}/${curdir}/${child})
        else()
            string (REGEX REPLACE ^${rootdir} ${folder} groupname ${curdir})
            string (REPLACE "/" "\\" groupname ${groupname})
            source_group (${foldername}${groupname} FILES ${ASIO3_ROOT_DIR}/${curdir}/${child})
        endif()
    endforeach()
endfunction()

function (GroupSources curdir folder)
    string (FIND ${curdir} "/" Separator)
    if(${Separator} EQUAL -1)
        set(Separator 0)
    else ()
        math(EXPR Separator "${Separator} + 1")
    endif()
    string (SUBSTRING ${curdir} ${Separator} -1 foldername)
    DoGroupSources (${curdir} ${curdir} ${foldername} ${folder})
endfunction()

#-------------------------------------------------------------------------------
#
# project
#
#-------------------------------------------------------------------------------

project (asio3)

set_property (GLOBAL PROPERTY USE_FOLDERS ON)

option(BUILD_EXAMPLES "Build examples" OFF)
option(BUILD_TESTS "Build tests" OFF)

# how to detect current is 32 bit or 64 bit ?
#if(NOT "${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
#endif()
#
#if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
#endif()
#
#if(NOT CMAKE_CL_64)
#endif()
#
#if(NOT ("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win64"))
#endif()
#
#if ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
#endif ()

#if(CMAKE_CL_64)
#    set(ASIO3_LIBS_DIR ${ASIO3_ROOT_DIR}/lib/x64)
#else(CMAKE_CL_64)
#    set(ASIO3_LIBS_DIR ${ASIO3_ROOT_DIR}/lib/x86)
#endif(CMAKE_CL_64)


# ---------------------------------------------------------------------------------------
# Set default build to debug
# ---------------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
    message("CMAKE_BUILD_TYPE is not set, Set default build to Release.")
endif()
message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
ELSE ()
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_definitions(-D_DEBUG)
    endif()
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Windows")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ASIO3_EXES_DIR ${ASIO3_ROOT_DIR}/bin/x64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ASIO3_EXES_DIR ${ASIO3_ROOT_DIR}/bin/x86)
endif()
 
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(OPENSSL_LIBS libssl.a libcrypto.a)
	set(GENERAL_LIBS -lpthread -lrt -ldl stdc++fs)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ASIO3_LIBS_DIR ${ASIO3_ROOT_DIR}/3rd/openssl/prebuilt/linux/x64)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(ASIO3_LIBS_DIR ${ASIO3_ROOT_DIR}/3rd/openssl/prebuilt/linux/x86)
    endif()
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(OPENSSL_LIBS "libssl.lib;libcrypto.lib;Crypt32.lib;")
	set(GENERAL_LIBS "ws2_32;mswsock;")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ASIO3_LIBS_DIR ${ASIO3_ROOT_DIR}/3rd/openssl/prebuilt/windows/x64)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(ASIO3_LIBS_DIR ${ASIO3_ROOT_DIR}/3rd/openssl/prebuilt/windows/x86)
    endif()
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	set(OPENSSL_LIBS libssl.a libcrypto.a)
	set(GENERAL_LIBS -lpthread -lrt -ldl stdc++fs)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
	set(OPENSSL_LIBS libssl.a libcrypto.a)
	set(GENERAL_LIBS -lpthread -ldl)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ASIO3_LIBS_DIR ${ASIO3_ROOT_DIR}/3rd/openssl/prebuilt/mac)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(ASIO3_LIBS_DIR ${ASIO3_ROOT_DIR}/3rd/openssl/prebuilt/mac)
    endif()
ELSE ()
	set(OPENSSL_LIBS libssl.a libcrypto.a)
	set(GENERAL_LIBS -lpthread -lrt -ldl stdc++fs)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

message("ASIO3_LIBS_DIR = ${ASIO3_LIBS_DIR}")
message("ASIO3_EXES_DIR = ${ASIO3_EXES_DIR}")


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${ASIO3_EXES_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${ASIO3_EXES_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${ASIO3_EXES_DIR})

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)


# ---------------------------------------------------------------------------------------
# Compiler config
# ---------------------------------------------------------------------------------------
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 23)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_SYSTEM_NAME MATCHES "CYGWIN" OR CMAKE_SYSTEM_NAME MATCHES "MSYS")
    set(CMAKE_CXX_EXTENSIONS ON)
endif()

add_library(asio3 INTERFACE)

target_include_directories(asio3
  INTERFACE
    ${ASIO3_ROOT_DIR}/3rd
    ${ASIO3_ROOT_DIR}/3rd/openssl/include
    ${ASIO3_ROOT_DIR}/include
)

target_link_directories(asio3
  INTERFACE
    ${ASIO3_LIBS_DIR}
)

target_link_libraries(asio3
  INTERFACE
    ${CMAKE_THREAD_LIBS_INIT}
    ${GENERAL_LIBS}
    ${OPENSSL_LIBS}
)

target_compile_features(asio3 INTERFACE cxx_std_23)

if (MSVC)
    set (CMAKE_VERBOSE_MAKEFILE FALSE)

    add_definitions (
        -D_SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING
        -D_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
    )

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      add_compile_options(
          /bigobj       # large object file format
          #/permissive-  # strict C++
          #/wd4503      # decorated name length exceeded, name was truncated
          /W4           # enable all warnings
          /JMC
          )
  
      set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
      set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi /Ob2 /Oi /Ot /MT")
  
      set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")

      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wrange-loop-analysis -Wthread-safety")
    else()
      add_compile_options(
          /bigobj       # large object file format
          #/permissive-  # strict C++
          #/wd4503      # decorated name length exceeded, name was truncated
          /W4           # enable all warnings
          /MP           # Multi-processor compilation
          /JMC
          /Zc:__cplusplus
          )
  
      set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /Zi")
      set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi /Ob2 /Oi /Ot /GL /MT")
  
      set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

      set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
      set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG:incremental")
    endif ()
else()
    set (THREADS_PREFER_PTHREAD_FLAG ON)
    find_package (Threads)

    set( CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -std=c++23 -Wall -Wextra -Wpedantic -Wno-unused-parameter -fcoroutines")

    #set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -ggdb")

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wrange-loop-analysis -Wthread-safety")
    endif ()
endif()


# https://stackoverflow.com/questions/44705845/file-too-big-compiling-on-cygwin-g
if(MINGW OR CYGWIN)
  add_definitions(-O3)
  add_compile_options(-Wa,-mbig-obj)
endif()




file (GLOB_RECURSE  ASIO3_FILES  ${ASIO3_ROOT_DIR}/include/asio3/*.*)
file (GLOB_RECURSE   ASIO_FILES  ${ASIO3_ROOT_DIR}/3rd/asio/*.*)

# exclude : asio/impl/src.cpp
list(REMOVE_ITEM ASIO_FILES ${ASIO3_ROOT_DIR}/3rd/asio/impl/src.cpp)

enable_testing()

# after test, i find that it will linked failed on MINGW when use the static ssl librarys, 
# but it will successed when use the shared ssl librarys like this:
# https://github.com/axmolengine/buildware/releases/tag/v41

if (BUILD_EXAMPLES)
  #include_directories (/usr/local/include) # for boost header file include
  #include_directories (D:/boost)           # for boost header file include
  
  include_directories (${ASIO3_ROOT_DIR}/3rd)
  include_directories (${ASIO3_ROOT_DIR}/3rd/openssl/include)
  include_directories (${ASIO3_ROOT_DIR}/include)
  
  #link_directories    (/usr/local/lib)     # for boost library file include
  #link_directories    (D:/boost/stage/lib) # for boost library file include
  
  add_subdirectory (example)
endif(BUILD_EXAMPLES)
