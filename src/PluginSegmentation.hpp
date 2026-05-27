#pragma once

#include <utility>
#include <vector>

#include "Common.hpp"

namespace opencc {

typedef std::vector<std::pair<std::string, std::string>> PluginConfigPairs;

SegmentationPtr CreatePluginSegmentation(const std::string& type,
                                         const PluginConfigPairs& configPairs);

} // namespace opencc
