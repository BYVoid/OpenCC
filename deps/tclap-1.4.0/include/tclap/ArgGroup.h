// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ArgGroup.h
 *
 *  Copyright (c) 2017 Google LLC.
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

#ifndef TCLAP_ARG_GROUP_H
#define TCLAP_ARG_GROUP_H

#include <tclap/Arg.h>
#include <tclap/ArgContainer.h>
#include <tclap/CmdLineInterface.h>

#include <list>
#include <string>

namespace TCLAP {

/**
 * ArgGroup is the base class for implementing groups of arguments
 * that are mutually exclusive (it replaces the deprecated xor
 * handler). It is not expected to be used directly, rather one of the
 * EitherOf or OneOf derived classes are used.
 */
class ArgGroup : public ArgContainer {
    friend class CmdLine;
    friend class CmdLineInterface;
public:
    typedef std::list<Arg *> Container;
    typedef Container::iterator iterator;
    typedef Container::const_iterator const_iterator;

    virtual ~ArgGroup() {}

    /// Add an argument to this arg group
    virtual ArgContainer &add(Arg &arg) { return add(&arg); }

    /// Add an argument to this arg group
    virtual ArgContainer &add(Arg *arg);

    /**
     * Validates that the constraints of the ArgGroup are satisfied.
     *
     * @internal
     * Throws an CmdLineParseException if there is an issue (except
     * missing required argument, in which case true is returned).
     *
     * @retval true iff a required argument was missing.
     */
    virtual bool validate() = 0;

    /**
     * Returns true if this argument group is required
     *
     * @internal
     */
    virtual bool isRequired() const = 0;

    /**
     * Returns true if this argument group is exclusive.
     *
     * @internal
     * Being exclusive means there is a constraint so that some
     * arguments cannot be selected at the same time.
     */
    virtual bool isExclusive() const = 0;

    /**
     * Used by the parser to connect itself to this arg group.
     *
     * @internal
     * This is needed so that existing and subsequently added args (in
     * this arg group) are also added to the parser (and checked for
     * consistency with other args).
     */
    void setParser(CmdLineInterface &parser) {
        if (_parser)
            throw SpecificationException("Arg group can have only one parser");
        if (parser.attachGroup(*this)) return;
        // Preserve the legacy path for custom CmdLineInterface implementations.
        _parser = &parser;
        for (iterator it = begin(); it != end(); ++it)
            parser.addToArgList(*it);
    }

    /**
     * If arguments in this group should show up as grouped in help.
     */
    virtual bool showAsGroup() const { return true; }

    /// Returns the argument group's name.
    const std::string getName() const;

    iterator begin() { return _args.begin(); }
    iterator end() { return _args.end(); }
    const_iterator begin() const { return _args.begin(); }
    const_iterator end() const { return _args.end(); }

protected:
    // No direct instantiation
    ArgGroup() : _parser(0), _args() {}

private:
    explicit ArgGroup(const ArgGroup &);
    ArgGroup &operator=(const ArgGroup &);  // no copy

protected:
    CmdLineInterface *_parser;
    Container _args;
};

/**
 * Implements common functionality for exclusive argument groups.
 *
 * @internal
 */
class ExclusiveArgGroup : public ArgGroup {
public:
    inline bool validate();
    bool isExclusive() const { return true; }
    ArgContainer &add(Arg &arg) { return add(&arg); }
    ArgContainer &add(Arg *arg) {
        if (arg->isRequired()) {
            throw SpecificationException(
                "Required arguments are not allowed"
                " in an exclusive grouping.",
                arg->longID());
        }

        return ArgGroup::add(arg);
    }

protected:
    ExclusiveArgGroup() {}
    explicit ExclusiveArgGroup(CmdLineInterface &parser) { parser.add(*this); }
};

/**
 * Implements a group of arguments where at most one can be selected.
 */
class EitherOf : public ExclusiveArgGroup {
public:
    EitherOf() {}
    explicit EitherOf(CmdLineInterface &parser) : ExclusiveArgGroup(parser) {}

    bool isRequired() const { return false; }
};

/**
 * Implements a group of arguments where exactly one must be
 * selected. This corresponds to the deprecated "xoradd".
 */
class OneOf : public ExclusiveArgGroup {
public:
    OneOf() {}
    explicit OneOf(CmdLineInterface &parser) : ExclusiveArgGroup(parser) {}

    bool isRequired() const { return true; }
};

/**
 * Groups arguments for presentation without imposing a selection rule.
 * Any combination of members may be selected, subject to each argument's
 * own requirements. This group does not enforce conditional dependencies;
 * applications must check any additional dependencies themselves.
 */
class AnyOf : public ArgGroup {
public:
    AnyOf() {}
    explicit AnyOf(CmdLineInterface &parser) { parser.add(*this); }

    bool validate() { return false; /* All good */ }
    bool isExclusive() const { return false; }
    bool isRequired() const { return false; }
};

inline ArgContainer &ArgGroup::add(Arg *arg) {
    for (iterator it = begin(); it != end(); it++) {
        if (arg->conflictsWith(**it)) {
            throw SpecificationException(
                "Argument with same flag/name already exists!", arg->longID());
        }
    }

    Container members(_args);
    members.push_back(arg);
    if (_parser) {
        _parser->addToArgList(arg);
    }
    _args.swap(members);

    return *this;
}

inline bool ExclusiveArgGroup::validate() {
    Arg *arg = NULL;
    std::string flag;

    for (const_iterator it = begin(); it != end(); ++it) {
        if ((*it)->isSet()) {
            if (arg != NULL && arg != *it) {
                // We found a matching argument, but one was
                // already found previously.
                throw CmdLineParseException(
                    "Only one is allowed.",
                    flag + " AND " + (*it)->setBy() + " provided.");
            }

            arg = *it;
            flag = arg->setBy();
        }
    }

    return isRequired() && !arg;
}

inline const std::string ArgGroup::getName() const {
    std::string name;
    std::string sep = "{";  // TODO: this should change for
                            // non-exclusive arg groups
    for (const_iterator it = begin(); it != end(); ++it) {
        name += sep + (*it)->getName();
        sep = " | ";
    }

    return name + '}';
}

/// @internal
inline int CountVisibleArgs(const ArgGroup &g) {
    int visible = 0;
    for (ArgGroup::const_iterator it = g.begin(); it != g.end(); ++it) {
        if ((*it)->visibleInHelp()) {
            visible++;
        }
    }

    return visible;
}

}  // namespace TCLAP

#endif  // TCLAP_ARG_GROUP_H
