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

#include <string>
#include <vector>

#include "src/tools/CommandLineMain.hpp"
#include "src/tools/PlatformIO.hpp"

namespace {

std::vector<std::string> ArgsToUtf8(int argc, char* argv[]) {
  std::vector<std::string> args;
  args.reserve(static_cast<size_t>(argc));
  for (int i = 0; i < argc; i++) {
    args.push_back(argv[i]);
  }
  return args;
}

} // namespace

#if defined(_WIN32) && defined(_MSC_VER)
int wmain() {
  return opencc::tools::CommandLineMain(
      opencc::tools::GetUtf8CommandLineArgs());
}
#elif defined(_WIN32)
int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;
  return opencc::tools::CommandLineMain(
      opencc::tools::GetUtf8CommandLineArgs());
}
#else
int main(int argc, char* argv[]) {
  return opencc::tools::CommandLineMain(ArgsToUtf8(argc, argv));
}
#endif
