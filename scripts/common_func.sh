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
