#!/bin/bash

#
# This scripts generates Makefiles for the llvm test-suite for the modified
# Checked C version.
#

# Load the common paths and variables.
. common.sh

# Use lld to link the libsafemm.
CFLAGS="-fuse-ld=lld"
LDFLAGS="$CHECKEDC_LIB/libsafemm.a"

# Go to the build directory. Create one if it does not exist.
[[ -d $TESTSUITE_BUILD_DIR ]] || mkdir -p $TESTSUITE_BUILD_DIR
cd "$TESTSUITE_BUILD_DIR"
rm -rf CMakeCache.txt

if [[ $1 == "lto" ]]; then
    CFLAGS="-flto $CFLAGS"
    LDFLAGS="-flto $CHECKEDC_LIB/libsafemm_lto.a"
fi

cmake -DCMAKE_C_COMPILER="$CC"                                                 \
      -DCMAKE_C_FLAGS="$CFLAGS"                                                \
      -DCMAKE_CXX_FLAGS="-mllvm -checkedc-init=false"                          \
      -DCMAKE_PREFIX_PATH="$CHECKEDC_INC;$CHECKEDC_LIB"                        \
      -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS"                                      \
      -C"$TESTSUITE_DIR"/cmake/caches/O3.cmake                                 \
      -DEXTRA_LARGE_PROBLEM_SIZE=1                                             \
      "$TESTSUITE_DIR"
