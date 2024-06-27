# frozen_string_literal: true

require "ffi"

module OpenCC
  module OpenCCLib
    extend FFI::Library
    ffi_lib File.expand_path("../../clib/lib/libopencc.so", __dir__)

    attach_function :opencc_open, [:string], :pointer
    attach_function :opencc_close, [:pointer], :int
    attach_function :opencc_convert_utf8, [:pointer, :string, :size_t], :pointer
    attach_function :opencc_convert_utf8_free, [:pointer], :void
  end
end
