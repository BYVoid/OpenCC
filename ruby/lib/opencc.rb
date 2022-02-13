# frozen_string_literal: true

require_relative "opencc/constants"
require_relative "opencc/errors"
require_relative "opencc/converter"
require_relative "opencc/opencc_lib"

module OpenCC
  def create(config)
    Converter.new config
  end

  module_function :create
end
