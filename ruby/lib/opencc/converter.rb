# frozen_string_literal: true

require "ffi"

require_relative "constants"
require_relative "errors"
require_relative "opencc_lib"

module OpenCC
  class Converter
    class CCPointer < FFI::AutoPointer
      def self.release(ptr)
        OpenCCLib.opencc_close ptr
      end
    end

    def initialize(config)
      raise Error, "invalid config" unless CONFIGS.include?(config)

      ptr = OpenCCLib.opencc_open("#{config.to_s}.json")
      raise Error, "call opencc_open error" if ptr == FFI::Pointer.new(-1)

      @cc_ptr = CCPointer.new ptr
    end

    def convert(str)
      ptr = OpenCCLib.opencc_convert_utf8 @cc_ptr, str, -1
      raise Error, "convert error" if ptr.null?
      converted_str = ptr.read_string.force_encoding Encoding::UTF_8
      OpenCCLib.opencc_convert_utf8_free ptr
      converted_str
    end
  end
end
