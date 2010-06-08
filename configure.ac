#
# Open Chinese Convert
#
# Copyright 2010 BYVoid <byvoid1@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

m4_define([package_name], [opencc])
m4_define([opencc_released], [0])
m4_define([opencc_major_version], [0])
m4_define([opencc_minor_version], [0])
m4_define([opencc_micro_version], [1])
m4_define(opencc_maybe_datestamp,
    m4_esyscmd([if test x]opencc_released[ != x1; then date +.%Y%m%d | tr -d '\n\r'; fi]))

m4_define([opencc_version],
    opencc_major_version.opencc_minor_version.opencc_micro_version[]opencc_maybe_datestamp)

AC_INIT([package_name], [opencc_version], [http://code.google.com/p/open-chinese-convert/issues/entry], [package_name])
AM_INIT_AUTOMAKE([1.10])
AC_GNU_SOURCE

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# define PACKAGE_VERSION_* variables
AS_VERSION
AS_NANO
AM_SANITY_CHECK
AM_MAINTAINER_MODE
AM_DISABLE_STATIC
AC_PROG_CC
AM_PROG_CC_C_O
# AC_PROG_CXX
AC_ISC_POSIX
AC_HEADER_STDC
AM_PROG_LIBTOOL
# IT_PROG_INTLTOOL([0.35.0])

#check python
AM_PATH_PYTHON([2.5])

# define GETTEXT_* variables
GETTEXT_PACKAGE="$PACKAGE_NAME"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Define to the read-only architecture-independent data directory.])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.16.1)

# OUTPUT files
AC_CONFIG_FILES([po/Makefile.in
Makefile
opencc.pc
opencc.spec
src/Makefile
data/Makefile
m4/Makefile
])

AC_OUTPUT
