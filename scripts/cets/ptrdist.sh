#!/bin/bash

#
# This scritps run the benchmarks of the Ptrdist test suite.
#

# load common directory paths and variables
cd ..
. common.sh

#
# Ptrdist-specific paths
BUILD_DIR="$CETS_TS_BUILD"
DATA_DIR="$DATA_DIR/ptrdsit/cets"
BIN_DIR="$BUILD_DIR/MultiSource/Benchmarks/Ptrdist"

PROGRAMS=(
    "anagram"
    "bc"
    "ft"
    "ks"
    "yacr2"
)

cd $BUILD_DIR

#
# Run one single benchmark program.
#
run_one() {
    # Compile the benchmark if its binary does not exist.
    if [[ ! -f "$BIN_DIR/$1/$1" ]]; then
        echo "Compiling $1..."
        # Set the compile parallel level to be #ofLogicalCore - 2.
        local parallell
        if [[ $OS == "Linux" ]]; then
            parallell=`lscpu | grep "^CPU(s)" | cut -d ':' -f2 | echo "$(cat -)-2" | bc`
        elif [[ $OS == "Darwin" ]]; then
            parallell=`sysctl -n hw.ncpu | echo "$(cat -)-2" | bc`
        fi
        make -j$parallell $1
    fi

    $CETS_LIT -v --filter $1 -o $DATA_DIR/$1.json .
}

#
# Run all benchmarks in Ptrdist
#
run_all() {
    for prog in ${PROGRAMS[@]}; do
        run_one $prog
    done
}

#
# Clean all compiled binaries.
#
clean() {
    echo "Cleaning all refactored Ptrdist benchmark binaries..."
    for prog in ${PROGRAMS[@]}; do
        echo "Cleaning $prog"
        cd "$BIN_DIR/$prog"
        find . -name "*.o" -delete
        rm -f "$prog" "$prog.stripped"
    done
}

#
# Print out usgae and exit.
#
usage() {
    echo "Usage $0 <benchmark>"
    echo
    echo "If no argument is given, $0 runs all the benchmark programs in Ptrdist."
    echo "To run a single benchmark program, specify its name as the first"\
        "argument of $0."
    exit
}

#
# Entrance of this script
#

# mkdir the data directory if it does not exist.
[[ -d $DATA_DIR ]] || mkdir -p $DATA_DIR

if [[ $# == 1 ]]; then
    if [[ $1 == "-h" || $1 == "--help" ]]; then
        usage
    elif [[ $1 == "clean" ]]; then
        clean
    else
        run_one $1
    fi
else
    run_all
fi
