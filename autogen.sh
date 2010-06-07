#!/bin/sh
set -e
set -x

autopoint
libtoolize --automake --copy
aclocal -I m4
autoheader
automake --add-missing --copy
autoconf
export CFLAGS="-g -O0"
export CXXFLAGS="$CFLAGS"
./configure --enable-maintainer-mode $*
