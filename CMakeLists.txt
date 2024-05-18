cmake_minimum_required(VERSION 3.27)
project(FusswegDatentools VERSION 1.0.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Specify the compilers
set(CMAKE_C_COMPILER ${CC})
set(CMAKE_CXX_COMPILER ${CXX})

# Force CMake to use ld.lld, and set linker flags
set(CMAKE_LINKER "${LD}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fuse-ld=lld")

# Detect the operating system and architecture
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND
   CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    message(STATUS "Building for macOS arm64")
    include("CMakeLists_osx.cmake")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND
       CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    message(STATUS "Building for Linux x86-64")
    include("CMakeLists_x64.cmake")
else()
    message(FATAL_ERROR
        "Unsupported operating system or architecture: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Additional common configuration (if any) can be added here
