# frozen_string_literal: true

require_relative "ruby/lib/opencc/constants"

Gem::Specification.new do |spec|
  spec.name = "opencc"
  spec.version = OpenCC::VERSION
  spec.authors = OpenCC::AUTHORS.map { |a| a[:name] }
  spec.email = OpenCC::AUTHORS.map { |a| a[:email] }

  spec.summary = "Conversion between Traditional and Simplified Chinese"
  spec.description = "Open Chinese Convert (OpenCC, 開放中文轉換) is an opensource project for conversions between Traditional Chinese, Simplified Chinese and Japanese Kanji (Shinjitai). It supports character-level and phrase-level conversion, character variant conversion and regional idioms among Mainland China, Taiwan and Hong Kong. This is not translation tool between Mandarin and Cantonese, etc."
  spec.homepage = "https://github.com/BYVoid/OpenCC"
  spec.required_ruby_version = ">= 2.6.0"

  spec.metadata["homepage_uri"] = spec.homepage
  spec.metadata["source_code_uri"] = spec.homepage

  spec.files = Dir.chdir(File.expand_path(__dir__)) do
    `git ls-files -z`.split("\x0").reject { |f| f == __FILE__ }
  end

  %w[node python . release-pypi].each do |prefix|
    spec.files.delete_if { |f| f.start_with? prefix }
  end

  %w[
    Makefile
    Gemfile Gemfile.lock Rakefile
    package.json package-lock.json
    binding.gyp setup.py
    build.cmd test.cmd
  ].each { |f| spec.files.delete f }

  spec.require_paths = ["ruby/lib"]
  spec.extensions = ["ruby/ext/extconf.rb"]

  spec.add_dependency "ffi", "~> 1.15", ">= 1.15.5"
end
