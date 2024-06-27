# frozen_string_literal: true

module OpenCC
  cmake_file   = File.read File.expand_path("../../../CMakeLists.txt", __dir__)
  authors_file = File.read File.expand_path("../../../AUTHORS", __dir__)

  VERSION = [
    cmake_file.match(/OPENCC_VERSION_MAJOR (\d+)/)[1],
    cmake_file.match(/OPENCC_VERSION_MINOR (\d+)/)[1],
    cmake_file.match(/OPENCC_VERSION_REVISION (\d+)/)[1]
  ].join(".")

  AUTHORS = authors_file.lines.filter_map do |line|
    m = line.match /(.+) <(.+)>/
    { name: m[1], email: m[2] } if m
  end

  CONFIGS = Dir.glob("*.json", base: File.expand_path("../../../data/config", __dir__))
                .map { |f| File.basename(f, ".json").to_sym }
end
