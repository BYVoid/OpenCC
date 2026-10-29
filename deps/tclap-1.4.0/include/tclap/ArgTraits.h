// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ArgTraits.h
 *
 *  Copyright (c) 2007, Daniel Aarno, Michael E. Smoot .
 *  Copyright (c) 2017 Google LLC
 *  All rights reserved.
 *
 *  See the file COPYING in the top directory of this distribution for
 *  more information.
 *
 *  THE SOFTWARE IS PROVIDED _AS IS_, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

// This is an internal tclap file, you should probably not have to
// include this directly

#ifndef TCLAP_ARG_TRAITS_H
#define TCLAP_ARG_TRAITS_H

namespace TCLAP {

// We use two empty structs to get compile type specialization
// function to work

/**
 * A value like argument value type is a value that can be set using
 * operator>>. This is the default value type.
 */
struct ValueLike {
    typedef ValueLike ValueCategory;
    virtual ~ValueLike() {}
};

/**
 * A string like argument value type is a value that can be set using
 * operator=(string). Useful if the value type contains spaces which
 * will be broken up into individual tokens by operator>>.
 */
struct StringLike {
    virtual ~StringLike() {}
};

/**
 * A class can inherit from this object to make it have string like
 * traits. The category selects parsing behavior at compile time.
 * This helper has a virtual destructor, so inheriting from it makes
 * the derived type polymorphic. Specialize ArgTraits instead if that
 * is undesirable.
 */
struct StringLikeTrait {
    typedef StringLike ValueCategory;
    virtual ~StringLikeTrait() {}
};

/**
 * A class can inherit from this object to make it have value like
 * traits. The category selects parsing behavior at compile time.
 * This helper has a virtual destructor, so inheriting from it makes
 * the derived type polymorphic. Specialize ArgTraits instead if that
 * is undesirable.
 */
struct ValueLikeTrait {
    typedef ValueLike ValueCategory;
    virtual ~ValueLikeTrait() {}
};

/**
 * Arg traits are used to get compile type specialization when parsing
 * argument values. Using an ArgTraits you can specify the way that
 * values gets assigned to any particular type during parsing. The two
 * supported types are StringLike and ValueLike. ValueLike is the
 * default and means that operator>> will be used to assign values to
 * the type.
 */
template <typename T>
class ArgTraits {
    // This is a bit silly, but what we want to do is:
    // 1) If there exists a specialization of ArgTraits for type X,
    // use it.
    //
    // 2) If no specialization exists but X has the typename
    // X::ValueCategory, use the specialization for X::ValueCategory.
    //
    // 3) If neither (1) nor (2) defines the trait, use the default
    // which is ValueLike.

    // This is the "how":
    //
    // test<T>(0) (where 0 is the NULL ptr) will match
    // test(typename C::ValueCategory*) iff type T has the
    // corresponding typedef. If it does not test(...) will be
    // matched. This allows us to determine if T::ValueCategory
    // exists by checking the sizeof for the test function (return
    // value must have different sizeof).
    template <typename C>
    static short test(typename C::ValueCategory *);  // NOLINT
    template <typename C>
    static long test(...);                                             // NOLINT
    static const bool hasTrait = sizeof(test<T>(0)) == sizeof(short);  // NOLINT

    template <typename C, bool>
    struct DefaultArgTrait {
        typedef ValueLike ValueCategory;
    };

    template <typename C>
    struct DefaultArgTrait<C, true> {
        typedef typename C::ValueCategory ValueCategory;
    };

public:
    typedef typename DefaultArgTrait<T, hasTrait>::ValueCategory ValueCategory;
};

}  // namespace TCLAP

#endif  // TCLAP_ARG_TRAITS_H
