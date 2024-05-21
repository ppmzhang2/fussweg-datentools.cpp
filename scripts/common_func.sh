#!/bin/bash

# Function to print error message and exit
function error_exit {
    echo "$1" 1>&2
    exit 1
}

# Function to determine the number of CPU cores
function get_num_cores {
    if command -v nproc &>/dev/null; then
        nproc
    elif command -v sysctl &>/dev/null; then
        sysctl -n hw.ncpu
    else
        echo "Unable to determine the number of CPU cores. Defaulting to 1."
        echo 1
    fi
}

# Function to create and clean directories
function make_clean_dir {
    local DIR=$1
    mkdir -p "${DIR}" || error_exit "Failed to create directory: ${DIR}"
    rm -rf "${DIR}"
    mkdir -p "${DIR}"
}

# Function to find the LLVM installation directory
#
# Ensure existance of the following files (in case of LLVM 19):
# - /usr/lib/llvm-19/lib/libLLVM.so.19.0
# - /usr/lib/llvm-19/lib/libLLVM.so
find_llvm_dir() {
    for dir in /usr/lib/llvm-*; do
        if [ -d "$dir" ]; then
            echo "$dir"
            return
        fi
    done
    echo ""
}
