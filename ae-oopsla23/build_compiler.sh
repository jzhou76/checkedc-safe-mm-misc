#!/bin/bash

# This script downloads and builds the Checked C compiler and the CETS compiler.

REQUIRED_DEPS=(
    "cmake"      # For building llvm
    "git"        # For downloading repos.
    "git-lfs"    # For pulling down large input data files for evaluation.
    "python3"    # For processing experimental data
)

ROOT_DIR=`realpath .`
SCRIPTS_DIR="$ROOT_DIR/misc/scripts"
CETS_DIR="$ROOT_DIR/cets"

#
# Github repos
#
MISC_REPO="https://github.com/jzhou76/checkedc-safe-mm-misc.git"
CHECKEDC_LLVM_REPO="https://github.com/jzhou76/checkedc-llvm.git"
CHECKEDC_CLANG_REPO="https://github.com/jzhou76/checkedc-clang.git"
CHECKEDC_REPO="https://github.com/jzhou76/checkedc.git"
WSS_REPO="https://github.com/brendangregg/wss.git"
CETS_REPO="https://github.com/jzhou76/CETS-llvm8.git"
OOPSLA23_BRANCH="AE-OOPSLA23"

PARALLEL=`lscpu | grep "^CPU(s)" | cut -d ':' -f2 | echo "$(cat -)" | bc`

#
# Check dependency
#
check_dep() {
    for dep in ${REQUIRED_DEPS[@]}; do
        echo "Checking $dep..."
        if [[ ! `which $dep` ]]; then
            echo "$dep not found. Please install $dep"
            exit
        fi
    done
}

#
# Prepare for building compilers.
#
prepare() {
    # Check if dependent software exists.
    check_dep

    # Check if the misc directory exists.
    if [[ ! -d "misc" ]]; then
        echo "Pulling the checkedc-mm-safe-misc repo..."
        git clone "$MISC_REPO" misc
        # The misc repo has a wss submodule in it for memory overhead eval,
        # the the submodule was clone by @jzhou76 using ssh.
        # Just in case the user of this script does not set up ssh for git,
        # here we pull the wss repo separately.
        git clone "$WSS_REPO" "misc/eval/wss"
        cd "misc/eval/wss"; git checkout 8951296
        cd -
    fi

    # Check if the Checked C compiler exists. git clone if not.
    if [[ ! -d "llvm" ]]; then
        echo "Pulling the Checked C compiler repo..."
        git clone "$CHECKEDC_LLVM_REPO" llvm
        cd llvm; git checkout $OOPSLA23_BRANCH; cd -
        git clone "$CHECKEDC_CLANG_REPO" llvm/tools/clang
        cd llvm/tools/clang; git checkout $OOPSLA23_BRANCH; cd -
        git clone "$CHECKEDC_REPO" llvm/projects/checkedc-wrapper/checkedc
        cd llvm/projects/checkedc-wrapper/checkedc; git checkout $OOPSLA23_BRANCH; cd -
    fi

    # Check if the CETS compiler exists.
    if [[ ! -d "cets" ]]; then
        echo "Pulling the CETS compiler repo..."
        mkdir cets
        git clone "$CETS_REPO" cets/llvm
        cd cets/llvm; git checkout $OOPSLA23_BRANCH
    fi
}

#
# Print out usage and exit
#
usage() {
    # TODO
    echo "Help"
}

#
# Build the Checked C compiler.
#
build_checkedc() {
    echo "Building the Checked C compiler"
    if [[ -f "build/bin/clang" ]]; then
        cd build
        make clang -j$PARALLEL
    else
        mkdir -p build; cd build
        cp "$SCRIPTS_DIR/cmake-llvm.sh" ./
        ./cmake-llvm.sh
        make clang -j$PARALLEL
    fi
}

#
# Build the CETS compiler.
#
build_cets() {
    echo "Building the CETS compiler"
    if [[ -f "cets/build/bin/clang" ]]; then
        cd cets/build
        make clang -j$PARALLEL
    else
        mkdir -p cets/build; cd cets/build
        cp "$SCRIPTS_DIR/cets/cmake-llvm.sh" ./
        ./cmake-llvm.sh
        make clang -j$PARALLEL
        make lld -j$PARALLEL
        make llvm-ar -j$PARALLEL
    fi

    # Build the CETS runtime lib.
    cd "$CETS_DIR/llvm/libsoftboundcets"
    make lto
}

#
# Entrance of this script
#
prepare

if [[ $# > 1 ]]; then
    echo "Unknown arguments"
        usage
    exit
elif [[ $# == 0 ]]; then
    build_checkedc
    build_cets
else
    if [[ $1 == "checkedc" ]]; then
        build_checkedc
    elif [[ $1 == "cets" ]]; then
        build_cets
    elif [[ $1 == "-h" ]]; then
        usage
    else
        echo "Unknown arguments"
        usage
        exit
    fi
fi
