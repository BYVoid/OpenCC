/*
 * Open Chinese Convert
 *
 * Copyright 2010-2013 BYVoid <byvoid@byvoid.com>
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

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <stdexcept>

#include <cstdio>
#include <cassert>
#include <cstddef>

#include "Optional.hpp"

using std::string;
using std::vector;
using std::list;
using std::map;
using std::runtime_error;

#ifdef ENABLE_GETTEXT
# include <libintl.h>
# include <locale.h>
# define _(STRING) dgettext(PACKAGE_NAME, STRING)
#else // ENABLE_GETTEXT
# define _(STRING) STRING
#endif // ENABLE_GETTEXT

#ifndef PKGDATADIR
#define PKGDATADIR ""
#endif
