// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ComparisonConstraint.h
 *
 *  Copyright (c) 2026, Google LLC
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

#ifndef TCLAP_COMPARISON_CONSTRAINT_H
#define TCLAP_COMPARISON_CONSTRAINT_H

#include <tclap/PredicateConstraint.h>

#include <functional>
#include <sstream>
#include <string>

namespace TCLAP {
namespace internal {

// A single owned threshold supplies validation and both forms of help text.
template <class T, class Compare>
class ComparisonRule {
public:
    ComparisonRule(const T &bound, const std::string &relation,
                   const std::string &symbol)
        : _bound(bound), _relation(relation), _shortId() {
        std::ostringstream out;
        out << '[' << symbol << _bound << ']';
        _shortId = out.str();
    }

    bool operator()(const T &value) const { return Compare()(value, _bound); }
    std::string description() const {
        std::ostringstream out;
        out << "value must be " << _relation << ' ' << _bound;
        return out.str();
    }
    std::string shortID() const { return _shortId; }

private:
    T _bound;
    std::string _relation;
    std::string _shortId;
};

template <class T, class Compare>
class ComparisonConstraintBase
    : public PredicateConstraint<T, ComparisonRule<T, Compare>,
                                 RuleConstraintText> {
protected:
    ComparisonConstraintBase(const T &bound, const std::string &relation,
                             const std::string &symbol)
        : PredicateConstraint<T, ComparisonRule<T, Compare>,
                              RuleConstraintText>(
              ComparisonRule<T, Compare>(bound, relation, symbol),
              RuleConstraintText()) {}
};

}  // namespace internal

/**
 * Accepts values strictly less than an owned upper bound.
 * T must support comparison and ostream insertion for generated help text.
 * Short IDs are cached at construction; descriptions are formatted on request.
 * Floating-point unordered comparisons fail, including NaN bounds/values.
 * Args borrow these constraints just as they borrow any Constraint<T>.
 */
template <class T>
class LessThanConstraint
    : public internal::ComparisonConstraintBase<T, std::less<T> > {
public:
    explicit LessThanConstraint(const T &bound)
        : internal::ComparisonConstraintBase<T, std::less<T> >(
              bound, "less than", "<") {}
};

/** Accepts values less than or equal to an owned upper bound. */
template <class T>
class LessThanOrEqualConstraint
    : public internal::ComparisonConstraintBase<T, std::less_equal<T> > {
public:
    explicit LessThanOrEqualConstraint(const T &bound)
        : internal::ComparisonConstraintBase<T, std::less_equal<T> >(
              bound, "less than or equal to", "<=") {}
};

/** Accepts values strictly greater than an owned lower bound. */
template <class T>
class GreaterThanConstraint
    : public internal::ComparisonConstraintBase<T, std::greater<T> > {
public:
    explicit GreaterThanConstraint(const T &bound)
        : internal::ComparisonConstraintBase<T, std::greater<T> >(
              bound, "greater than", ">") {}
};

/** Accepts values greater than or equal to an owned lower bound. */
template <class T>
class GreaterThanOrEqualConstraint
    : public internal::ComparisonConstraintBase<T, std::greater_equal<T> > {
public:
    explicit GreaterThanOrEqualConstraint(const T &bound)
        : internal::ComparisonConstraintBase<T, std::greater_equal<T> >(
              bound, "greater than or equal to", ">=") {}
};

}  // namespace TCLAP

#endif  // TCLAP_COMPARISON_CONSTRAINT_H
