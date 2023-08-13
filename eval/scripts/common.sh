#!/bin/bash

#
# project paths
#
ROOT_DIR="$(realpath $(dirname ${BASH_SOURCE[0]})/../../../)"
MISC_DIR="$ROOT_DIR/misc"
MISC_SCRIPTS="$MISC_DIR/scripts"
LLVM_SRC="$ROOT_DIR/llvm"
LLVM_RELEASE_BIN_DIR="$ROOT_DIR/build-release/bin"
TESTS_DIR="$ROOT_DIR/tests"
TESTSUITE_DIR="$TESTS_DIR/test-suite"
TESTSUITE_BUILD_DIR="$TESTS_DIR/ts-build"
BENCHMARKS_DIR="$MISC_DIR/benchmarks"
OLDEN_DIR="$LLVM_TESTSUITE_DIR/MultiSource/Benchmarks/Olden"
PTRDIST_DIR="$LLVM_TESTSUITE_DIR/MultiSource/Benchmarks/Ptrdist"
DATA_DIR="$MISC_DIR/eval/perf_data"

#
# Compiler related
#
LLVM_BIN_DIR="$ROOT_DIR/build/bin"
CC=$LLVM_BIN_DIR/clang
CXX=$LLVM_BIN_DIR/clang++
CHECKEDC_INC=$MISC_DIR/include
CHECKEDC_LIB=$MISC_DIR/lib
CC_RELEASE="$LLVM_RELEASE_BIN_DIR/clang"
LLVM_VANILLA_BIN="$ROOT_DIR/llvm-vanilla/build/bin"
LIT="$LLVM_BIN_DIR/llvm-lit"
# Original LLVM compiler
CC_VANILLA="$LLVM_VANILLA_BIN/clang"
CXX_VANILLA="$LLVM_VANILLA_BIN/clang++"

#
# Evaluation related
#
EVAL_DIR="$MISC_DIR/eval"
BENCHMARK_BUILD="$ROOT_DIR/benchmark-build"

#
# Others
#
OS=`uname`

# Set the compile parallel level to be #ofLogicalCore
PARA_LEVEL=`lscpu | grep "^CPU(s):" | cut -d ':' -f2`
