/*
 * Open Chinese Convert
 *
 * Copyright 2020-2026 Carbo Kuo and contributors
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

#include "Config.hpp"
#include "ResourceProvider.hpp"
#include "opencc.h"
#include <memory>
#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE(opencc_clib, m) {
  py::class_<opencc::SimpleConverter>(m, "_OpenCC")
      .def(py::init([](const std::string& configFileName,
                       bool includeTofuRiskDictionaries,
                       py::object resourceZip) {
             opencc::ConfigLoadOptions options;
             options.includeTofuRiskDictionaries = includeTofuRiskDictionaries;
             if (!resourceZip.is_none()) {
               std::shared_ptr<opencc::ResourceProvider> provider(
                   new opencc::ZipResourceProvider(
                       resourceZip.cast<std::string>()));
               return new opencc::SimpleConverter(configFileName, provider,
                                                  options);
             }
             return new opencc::SimpleConverter(configFileName, options);
           }),
           py::arg("config"),
           py::arg("include_tofu_risk_dictionaries") = true,
           py::arg("resource_zip") = py::none())
      .def("convert", py::overload_cast<const char*, size_t>(
                          &opencc::SimpleConverter::Convert, py::const_));

#ifdef OPENCC_VERSION
  m.attr("__version__") = OPENCC_VERSION;
#else
  m.attr("__version__") = "dev";
#endif
}
