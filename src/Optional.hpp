/*
 * Open Chinese Convert
 *
 * Copyright 2010-2014 Carbo Kuo <byvoid@byvoid.com>
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

namespace opencc {
/**
 * A class that wraps type T into a nullable type.
 * @ingroup opencc_cpp_api
 */
template <typename T> class Optional {
public:
  /**
   * The constructor of Optional.
   */
  Optional(T actual) : isNull(false), data(actual) {}

  /**
   * Returns true if the instance is null.
   */
  bool IsNull() const { return isNull; }

  /**
   * Returns the containing data of the instance.
   */
  const T& Get() const { return data; }

  /**
   * Constructs a null instance.
   */
  static Optional<T> Null() { return Optional(); }

private:
  Optional() : isNull(true) {}

  bool isNull;
  T data;
};

/**
 * Specialization of Optional for pointers.
 *
 * Reduce a bool.
 */
template <typename T> class Optional<T*> {
private:
  Optional() : data(nullptr) {}

  typedef T* TPtr;
  TPtr data;

public:
  Optional(TPtr actual) : data(actual) {}

  bool IsNull() const { return data == nullptr; }

  const TPtr& Get() const { return data; }

  static Optional<TPtr> Null() { return Optional(); }
};
} // namespace opencc