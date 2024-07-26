/*
 * Open Chinese Convert
 *
 * Copyright 2015 Carbo Kuo <byvoid@byvoid.com>
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

#ifdef BAZEL
#include "tools/cpp/runfiles/runfiles.h"
#endif

#include "TestUtils.hpp"

namespace opencc {

#ifdef CMAKE_SOURCE_DIR
class ConfigTestBase : public ::testing::Test {
protected:
  ConfigTestBase()
      : CONFIG_TEST_JSON_PATH(CMAKE_SOURCE_DIR
                              "/test/config_test/config_test.json"),
        CONFIG_TEST_DIR_PATH(CMAKE_SOURCE_DIR "/test/config_test") {}

  const std::string CONFIG_TEST_JSON_PATH;
  const std::string CONFIG_TEST_DIR_PATH;
};
#endif

#ifdef BAZEL
using bazel::tools::cpp::runfiles::Runfiles;
class ConfigTestBase : public ::testing::Test {
protected:
  ConfigTestBase()
      : runfiles_(Runfiles::CreateForTest()),
        CONFIG_TEST_JSON_PATH(
            runfiles_->Rlocation("_main/test/config_test/config_test.json")),
        CONFIG_TEST_DIR_PATH(runfiles_->Rlocation("_main/test/config_test")) {}

  const std::unique_ptr<Runfiles> runfiles_;
  const std::string CONFIG_TEST_JSON_PATH;
  const std::string CONFIG_TEST_DIR_PATH;
};
#endif

} // namespace opencc
