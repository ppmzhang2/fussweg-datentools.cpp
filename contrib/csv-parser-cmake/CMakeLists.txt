cmake_minimum_required(VERSION 3.27)
project(csv)

set(CSV_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../csv-parser")

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads QUIET REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage -Og")

set(CSV_INCLUDE_DIR ${CSV_ROOT_DIR}/include)
set(CSV_SRC_DIR ${CSV_INCLUDE_DIR}/internal)
set(CSV_DEVELOPER FALSE)

set(CSV_SRCS
    "${CSV_SRC_DIR}/basic_csv_parser.hpp"
    "${CSV_SRC_DIR}/basic_csv_parser.cpp"
    "${CSV_SRC_DIR}/col_names.cpp"
    "${CSV_SRC_DIR}/col_names.hpp"
    "${CSV_SRC_DIR}/common.hpp"
    "${CSV_SRC_DIR}/csv_format.hpp"
    "${CSV_SRC_DIR}/csv_format.cpp"
    "${CSV_SRC_DIR}/csv_reader.hpp"
    "${CSV_SRC_DIR}/csv_reader.cpp"
    "${CSV_SRC_DIR}/csv_reader_iterator.cpp"
    "${CSV_SRC_DIR}/csv_row.hpp"
    "${CSV_SRC_DIR}/csv_row.cpp"
    "${CSV_SRC_DIR}/csv_row_json.cpp"
    "${CSV_SRC_DIR}/csv_stat.cpp"
    "${CSV_SRC_DIR}/csv_stat.hpp"
    "${CSV_SRC_DIR}/csv_utility.cpp"
    "${CSV_SRC_DIR}/csv_utility.hpp"
    "${CSV_SRC_DIR}/csv_writer.hpp"
    "${CSV_SRC_DIR}/data_type.hpp"
)

add_library(csv STATIC "${CSV_SRCS}")

set_target_properties(csv PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(csv PRIVATE Threads::Threads)

target_include_directories(csv INTERFACE "${CSV_INCLUDE_DIR}")
target_include_directories(csv SYSTEM PUBLIC "${CSV_INCLUDE_DIR}")

# install
install(TARGETS csv DESTINATION lib)
install(DIRECTORY "${CSV_INCLUDE_DIR}/" DESTINATION include)
