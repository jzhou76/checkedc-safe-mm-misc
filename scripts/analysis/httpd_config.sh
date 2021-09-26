#!/usr/bin/env bash

#
# This scripts configures the compiling files for httpd.
#
# Note:
# 1. Although setting the LDFLAGS to use lld, the build system may still
# tries to use the system's ld. For example, if linux brew is installed and
# there is a ld in /home/linuxbrew/.linuxbrew/bin and this bin directory
# is in a user's shell's PATH, the build system may try to use the ld there.
# One solution is to "ln -s" that ld to the lld that is supposed to be used.
#
# 2. After compiling and a httpd binary is successfully generated, the binary
# may not be able to find libpcre.so.1, even though --with-pcre is set to
# a used-specified pcre directory. If libpcre.so.3 is installed in system's
# default library directories, e.g., /lib/x86_64-linux-gnu/libpcre.so.3, then
# one solution to this problem is to simply create a libpcre.so.1 that is a
# softlink to libpcre.so.3.
#

. common.sh

SRC_DIR="$PROGRAMS/httpd-2.4.46"

export CC="$CC"
export CFLAGS="-flto"
export LD="$CC"
export LDFLAGS="-fuse-ld=lld -lstdc++ $DYN_STATS_PASS $ANALYSIS_LIB/libanalysis.o"
export AR="$AR"
export NM="$NM"

cd $SRC_DIR

make clean

./configure --prefix="$SRC_DIR/install"

if [[ $1 == "make" ]]; then
    make -j8
    restore
elif [[ $1 == "restore" ]]; then
    restore
fi
