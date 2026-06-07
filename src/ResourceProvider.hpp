/*
 * Open Chinese Convert
 *
 * Copyright 2010-2026 Carbo Kuo and contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "Export.hpp"

namespace opencc {

class OPENCC_EXPORT ResourceProvider {
public:
  virtual ~ResourceProvider() = default;

  virtual std::string Resolve(std::string_view resourceName) const = 0;
};

class OPENCC_EXPORT FilesystemResourceProvider : public ResourceProvider {
public:
  explicit FilesystemResourceProvider(std::vector<std::string> searchPaths);

  std::string Resolve(std::string_view resourceName) const override;

private:
  std::vector<std::string> searchPaths;
};

} // namespace opencc
