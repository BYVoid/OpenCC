#!/bin/sh
set -e
set -x

autopoint --force
libtoolize --automake --force --copy
aclocal -I m4
autoheader
automake --add-missing --copy
autoconf
./configure --enable-maintainer-mode $*
