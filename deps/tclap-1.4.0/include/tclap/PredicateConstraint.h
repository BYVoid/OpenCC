// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  PredicateConstraint.h
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

#ifndef TCLAP_PREDICATE_CONSTRAINT_H
#define TCLAP_PREDICATE_CONSTRAINT_H

#include <tclap/Constraint.h>

#include <stdexcept>
#include <string>

namespace TCLAP {
namespace internal {

// Text policies receive the stored callable so a rule owns its state once.
class FixedConstraintText {
public:
    FixedConstraintText(const std::string &description,
                        const std::string &shortId)
        : _description(description), _shortId(shortId) {}

    template <class Function>
    std::string description(const Function &) const {
        return _description;
    }
    template <class Function>
    std::string shortID(const Function &) const {
        return _shortId;
    }

private:
    std::string _description;
    std::string _shortId;
};

struct RuleConstraintText {
    template <class Rule>
    std::string description(const Rule &rule) const {
        return rule.description();
    }
    template <class Rule>
    std::string shortID(const Rule &rule) const {
        return rule.shortID();
    }
};

// Only known empty representations are checked; a functor's operator bool()
// may have a meaning unrelated to whether it can be called.
template <class Function>
struct ValidateConstraintPredicate {
    template <class Actual>
    static void check(const Actual &) {}
};

template <class Function>
struct ValidateConstraintPredicate<const Function>
    : ValidateConstraintPredicate<Function> {};
template <class Function>
struct ValidateConstraintPredicate<volatile Function>
    : ValidateConstraintPredicate<Function> {};
template <class Function>
struct ValidateConstraintPredicate<const volatile Function>
    : ValidateConstraintPredicate<Function> {};

template <class Result, class Argument>
struct ValidateConstraintPredicate<Result (*)(Argument)> {
    static void check(Result (*function)(Argument)) {
        if (!function)
            throw std::invalid_argument(
                "PredicateConstraint requires a non-empty callable");
    }
};

// noexcept is part of a function pointer's type since C++17. Keep the
// C++98 branch safe when a consumer uses a newer language standard too.
#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
template <class Result, class Argument>
struct ValidateConstraintPredicate<Result (*)(Argument) noexcept> {
    static void check(Result (*function)(Argument) noexcept) {
        if (!function)
            throw std::invalid_argument(
                "PredicateConstraint requires a non-empty callable");
    }
};
#endif

}  // namespace internal

/**
 * Adapts an owned function or const function object to Constraint<T>.
 *
 * Function must accept const T& and return a value convertible to bool. The
 * adapter copies the supplied callable; explicitly borrowed state inside it
 * must remain alive. Args borrow the constraint itself: declare it before
 * the arguments using it and do not move it after binding its address.
 * Null function pointers are rejected with std::invalid_argument. Exceptions
 * from the callable or text policy propagate unchanged from these methods.
 *
 * MakeConstraint() deduces Function. MakeRuleConstraint() also obtains
 * description()/shortID() from the same stored callable. Ordinary callers
 * supply separate description and short-ID strings and need no text policy.
 */
template <class T, class Function = bool (*)(const T &),
          class Text = internal::FixedConstraintText>
class PredicateConstraint : public Constraint<T> {
public:
    typedef Function Predicate;

    /** Copies the callable and the two independently supplied strings. */
    PredicateConstraint(const std::string &description,
                        const std::string &shortId, const Function &function)
        : _function(function), _text(description, shortId) {
        internal::ValidateConstraintPredicate<Function>::check(_function);
    }

    /** Advanced form used by the rule factory to share validation/text state.
     */
    PredicateConstraint(const Function &function, const Text &text)
        : _function(function), _text(text) {
        internal::ValidateConstraintPredicate<Function>::check(_function);
    }

    virtual ~PredicateConstraint() {}

    bool check(const T &value) const { return _function(value); }
    std::string description() const { return _text.description(_function); }
    std::string shortID() const { return _text.shortID(_function); }

private:
    Function _function;
    Text _text;
};

/**
 * Creates an owning constraint with fixed presentation text. T is explicit;
 * the callable type is deduced. C++98 callers normally construct the adapter
 * directly because storing a factory result requires spelling its type.
 */
template <class T, class Function>
PredicateConstraint<T, Function> MakeConstraint(const std::string &description,
                                                const std::string &shortId,
                                                Function function) {
    return PredicateConstraint<T, Function>(description, shortId, function);
}

/**
 * Creates an owning rule constraint. Rule supplies a const operator()(const
 * T&), description() and shortID(); both text methods return std::string.
 * Presentation observes the same rule object that validates values.
 */
template <class T, class Rule>
PredicateConstraint<T, Rule, internal::RuleConstraintText> MakeRuleConstraint(
    Rule rule) {
    return PredicateConstraint<T, Rule, internal::RuleConstraintText>(
        rule, internal::RuleConstraintText());
}

}  // namespace TCLAP

#endif  // TCLAP_PREDICATE_CONSTRAINT_H
