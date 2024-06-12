# # TODO: Add compile options: libc++ and libc++abi. May need to add options
# #       for dependencies (e.g. exiv2) before adding these options.
# add_compile_options(-stdlib=libc++)
# # Add link options
# add_link_options(-stdlib=libc++)
# # Optionally add include directories for libc++ headers
# include_directories(/path/to/libc++/include)
# # Optionally add link directories for libc++ libraries
# link_directories(/path/to/libc++/lib)

# Link against libraries such as libdl, libc, etc.
# TODO: maybe libc++ and libc++abi?
# link_directories(/usr/lib/x86_64-linux-gnu)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -static-libgcc -static-libstdc++")

# -----------------------------------------------------------------------------
# OpenCV
# -----------------------------------------------------------------------------
set(OpenCV_DIR ${FusswegDatentools_SOURCE_DIR}/build_contrib/opencv-install)
set(OpenCV_INC_DIR ${OpenCV_DIR}/include/opencv4)
set(OpenCV_LIB_DIR ${OpenCV_DIR}/lib)
set(OpenCV_LIB3RD_DIR ${OpenCV_DIR}/lib/opencv4/3rdparty)
set(ZLIBNG_DIR ${FusswegDatentools_SOURCE_DIR}/build_contrib/zlib-ng-install)
set(ZLIBNG_INC_DIR ${ZLIBNG_DIR}/include)
set(ZLIBNG_LIB_DIR ${ZLIBNG_DIR}/lib)
set(PNG_DIR ${FusswegDatentools_SOURCE_DIR}/build_contrib/libpng-install)
set(PNG_INC_DIR ${PNG_DIR}/include)
set(PNG_LIB_DIR ${PNG_DIR}/lib)

# Find the libraries
find_library(ZLIBNG NAMES z HINTS ${ZLIBNG_LIB_DIR})
find_library(PNG NAMES png HINTS ${PNG_LIB_DIR})

find_library(ILMIMF NAMES IlmImf HINTS ${OpenCV_LIB3RD_DIR})
find_library(ITTNOTIFY NAMES ittnotify HINTS ${OpenCV_LIB3RD_DIR})
find_library(LIBJPEGTURBO NAMES libjpeg-turbo HINTS ${OpenCV_LIB3RD_DIR})
find_library(LIBOPENJP2 NAMES libopenjp2 HINTS ${OpenCV_LIB3RD_DIR})
find_library(LIBTIFF NAMES libtiff HINTS ${OpenCV_LIB3RD_DIR})
find_library(LIBWEBP NAMES libwebp HINTS ${OpenCV_LIB3RD_DIR})
find_library(TEGRA_HAL NAMES tegra_hal HINTS ${OpenCV_LIB3RD_DIR})

find_library(OPENCV_CORE NAMES opencv_core HINTS ${OpenCV_LIB_DIR})
find_library(OPENCV_IMGCODECS NAMES opencv_imgcodecs HINTS ${OpenCV_LIB_DIR})
find_library(OPENCV_IMGPROC NAMES opencv_imgproc HINTS ${OpenCV_LIB_DIR})
find_library(OPENCV_VIDEO NAMES opencv_video HINTS ${OpenCV_LIB_DIR})

# -----------------------------------------------------------------------------
# Exiv2
# -----------------------------------------------------------------------------
set(EXIV2_DIR ${FusswegDatentools_SOURCE_DIR}/build_contrib/exiv2-install)
set(EXIV2_INC_DIR ${EXIV2_DIR}/include)
set(EXIV2_LIB_DIR ${EXIV2_DIR}/lib)

# Find the libraries
find_library(EXIV2 NAMES exiv2 HINTS ${EXIV2_LIB_DIR})

# -----------------------------------------------------------------------------
# nlohmann_json
# -----------------------------------------------------------------------------
set(JSON_SRC_DIR "${FusswegDatentools_SOURCE_DIR}/contrib/nlohmann_json")
set(JSON_INC_DIR "${JSON_SRC_DIR}/include/nlohmann")
set(JSON_BuildTests OFF CACHE INTERNAL "")

add_subdirectory(${JSON_SRC_DIR})

# -----------------------------------------------------------------------------
# GTest
# -----------------------------------------------------------------------------
add_subdirectory(${FusswegDatentools_SOURCE_DIR}/contrib/googletest-cmake)

# -----------------------------------------------------------------------------
# System and executable
# -----------------------------------------------------------------------------
find_package(Threads REQUIRED)

# -----------------------------------------------------------------------------
# Library List
# -----------------------------------------------------------------------------
set(MAIN_ALL_LIBS
    c
    dl
    Threads::Threads
    ${ZLIBNG}
    ${PNG}
    ${EXIV2}
    nlohmann_json::nlohmann_json
)
set(TEST_ALL_LIBS
    c
    dl
    Threads::Threads
    ${ZLIBNG}
    ${PNG}
    ${EXIV2}
    nlohmann_json::nlohmann_json
    gtest
    gtest_main
)
set(OCV_ALL_LIBS)

foreach(lib ILMIMF ITTNOTIFY LIBJPEGTURBO LIBOPENJP2 LIBTIFF LIBWEBP TEGRA_HAL OPENCV_CORE OPENCV_IMGCODECS OPENCV_IMGPROC OPENCV_VIDEO)
    if(${lib})
        list(APPEND OCV_ALL_LIBS ${${lib}})
    endif()
endforeach()

list(APPEND MAIN_ALL_LIBS ${OCV_ALL_LIBS})
list(APPEND TEST_ALL_LIBS ${OCV_ALL_LIBS})

target_include_directories(${OUT_BIN_NAME} PRIVATE
    ${OpenCV_INC_DIR}
    ${ZLIBNG_INC_DIR}
    ${PNG_INC_DIR}
    ${EXIV2_INC_DIR}
    ${JSON_INC_DIR}
    "${FusswegDatentools_SOURCE_DIR}/include"
    "${FusswegDatentools_BINARY_DIR}/include" # Ensures config.h can be found
)
target_include_directories(${TEST_BIN_NAME} PRIVATE
    ${OpenCV_INC_DIR}
    ${ZLIBNG_INC_DIR}
    ${PNG_INC_DIR}
    ${EXIV2_INC_DIR}
    ${JSON_INC_DIR}
    "${FusswegDatentools_SOURCE_DIR}/include"
    "${FusswegDatentools_BINARY_DIR}/include" # Ensures config.h can be found
)

# Link libraries
target_link_libraries(${OUT_BIN_NAME} PRIVATE ${MAIN_ALL_LIBS})
target_link_libraries(${TEST_BIN_NAME} PRIVATE ${TEST_ALL_LIBS})
