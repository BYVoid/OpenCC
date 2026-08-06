Bump the Git-less fallback version to 1.4.1: the release tarball carries
no Git metadata, so without this the build identifies itself as 1.4.0 and
installs libopencc.so.1.4.0.

Obtained from:	https://github.com/BYVoid/OpenCC/commit/c23e743702f4

--- cmake/GitVersion.cmake.orig	2026-07-12 14:54:12 UTC
+++ cmake/GitVersion.cmake
@@ -14,7 +14,7 @@
 
 set(_OPENCC_FALLBACK_MAJOR 1)
 set(_OPENCC_FALLBACK_MINOR 4)
-set(_OPENCC_FALLBACK_REVISION 0)
+set(_OPENCC_FALLBACK_REVISION 1)
 
 find_package(Git QUIET)
