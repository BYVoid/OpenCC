require "fileutils"

ROOT_PATH = File.expand_path("../..", __dir__)
CLIB_PATH = File.expand_path("../clib", __dir__)

FileUtils.mkdir_p CLIB_PATH
system "cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=#{CLIB_PATH} #{ROOT_PATH}"
