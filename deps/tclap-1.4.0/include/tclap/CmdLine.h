// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  CmdLine.h
 *
 *  Copyright (c) 2003, Michael E. Smoot .
 *  Copyright (c) 2004, Michael E. Smoot, Daniel Aarno.
 *  Copyright (c) 2018, Google LLC
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

#ifndef TCLAP_CMD_LINE_H
#define TCLAP_CMD_LINE_H

#include <tclap/MultiSwitchArg.h>
#include <tclap/SwitchArg.h>
#include <tclap/UnlabeledMultiArg.h>
#include <tclap/UnlabeledValueArg.h>

#include <tclap/HelpVisitor.h>
#include <tclap/IgnoreRestVisitor.h>
#include <tclap/VersionVisitor.h>

#include <tclap/CmdLineOutput.h>
#include <tclap/StdOutput.h>

#include <tclap/Constraint.h>
#include <tclap/PredicateConstraint.h>
#include <tclap/ComparisonConstraint.h>
#include <tclap/RangeConstraint.h>
#include <tclap/ValuesConstraint.h>

#include <tclap/ArgGroup.h>
#include <tclap/DeferDelete.h>

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <utility>

namespace TCLAP {

class StandaloneArgs : public AnyOf {
public:
    StandaloneArgs() {}

    ArgContainer &add(Arg *arg) {
        for (iterator it = begin(); it != end(); it++) {
            if (arg->conflictsWith(**it)) {
                throw SpecificationException(
                    "Argument with same flag/name already exists!",
                    arg->longID());
            }
        }

        _args.push_back(arg);

        return *this;
    }

    bool showAsGroup() const { return false; }
};

/**
 * The base class that manages the command line definition and passes
 * along the parsing to the appropriate Arg classes.
 */
class CmdLine : public CmdLineInterface {
protected:
    /**
     * The list of arguments that will be tested against the
     * command line.
     */
    std::list<Arg *> _argList;

    StandaloneArgs _standaloneArgs;
    StandaloneArgs _autoArgs;  // --help, --version, etc

    /**
     * Some args have set constraints on them (i.e., exactly or at
     * most one must be specified.
     */
    std::list<ArgGroup *> _argGroups;

    /**
     * The name of the program.  Set to argv[0].
     */
    std::string _progName;

    /**
     * A message used to describe the program.  Used in the usage output.
     */
    std::string _message;

    /**
     * The version to be displayed with the --version switch.
     */
    std::string _version;

    /**
     * The number of arguments that are required to be present on
     * the command line. This is set dynamically, based on the
     * Args added to the CmdLine object.
     */
    int _numRequired;

    /**
     * The character that is used to separate the argument flag/name
     * from the value.  Defaults to ' ' (space).
     */
    char _delimiter;

    // Registration order, scoped to this parser rather than construction.
    bool _hasOptionalUnlabeledArg;
    bool _registrationActive;
    class Registration;
    bool attachGroup(ArgGroup &group) {
        add(group);
        return true;
    }

    /**
     * Add pointers that should be deleted as part of cleanup when
     * this object is destroyed.
     * @internal.
     */
    DeferDelete _deleteOnExit;

    /**
     * Default output handler if nothing is specified.
     */
    StdOutput _defaultOutput;

    /**
     * Object that handles all output for the CmdLine.
     */
    CmdLineOutput *_output;

    /**
     * Should CmdLine handle parsing exceptions internally?
     */
    bool _handleExceptions;

    /**
     * Optional callback used to localize user-facing strings.
     */
    struct MessageTranslator {
        virtual ~MessageTranslator() {}
        virtual std::string translate(const std::string &messageId,
                                      const std::string &fallback) const = 0;
    };

    template <typename T>
    struct MessageTranslatorImpl : MessageTranslator {
        T _translator;
        MessageTranslatorImpl(const T &translator) : _translator(translator) {}
        virtual std::string translate(const std::string &messageId,
                                      const std::string &fallback) const {
            return _translator(messageId, fallback);
        }
    };

    MessageTranslator *_messageTranslator;

    /**
     * Built-in arguments whose descriptions may need refreshing when the
     * translator changes.
     */
    SwitchArg *_ignoreArg;
    SwitchArg *_helpArg;
    SwitchArg *_versionArg;

    /**
     * Throws an exception listing the missing args.
     */
    void missingArgsException(const std::list<ArgGroup *> &missing);

    /**
     * Checks whether a name/flag string matches entirely matches
     * the Arg::blankChar.  Used when multiple switches are combined
     * into a single argument.
     * \param s - The message to be used in the usage.
     */
    bool _emptyCombined(const std::string &s);

private:
    /**
     * Prevent accidental copying.
     */
    CmdLine(const CmdLine &rhs);
    CmdLine &operator=(const CmdLine &rhs);

    /**
     * Encapsulates the code common to the constructors
     * (which is all of it).
     */
    void _constructor();

    void _createIgnoreRestArgument();
    void _createHelpAndVersionArguments();
    void _refreshBuiltinDescriptions();

    void _setProgramName(std::vector<std::string> &args);
    int _parseArguments(std::vector<std::string> &args);
    bool _processArgument(int &index, std::vector<std::string> &args,
                          int &requiredCount);
    void _validateRequiredArguments(int requiredCount);

    /**
     * Whether or not to automatically create help and version switches.
     */
    bool _helpAndVersion;

    /**
     * Whether or not to ignore unmatched args.
     */
    bool _ignoreUnmatched;

    /**
     * Ignoring arguments (e.g., after we have seen "--")
     */
    bool _ignoring;

public:
    /**
     * Command line constructor. Defines how the arguments will be
     * parsed.
     * \param message - The message to be used in the usage
     * output.
     * \param delimiter - The character that is used to separate
     * the argument flag/name from the value.  Defaults to ' ' (space).
     * \param version - The version number to be used in the
     * --version switch.
     * \param helpAndVersion - Whether or not to create the Help and
     * Version switches. Defaults to true.
     */
    CmdLine(const std::string &message, const char delimiter = ' ',
            const std::string &version = "none", bool helpAndVersion = true);

    /**
     * Deletes any resources allocated by a CmdLine object.
     */
    virtual ~CmdLine() { delete _messageTranslator; }

    /**
     * Adds an argument to the list of arguments to be parsed.
     *
     * @param a - Argument to be added.
     * @retval A reference to this so that add calls can be chained
     */
    ArgContainer &add(Arg &a);

    /**
     * An alternative add.  Functionally identical.
     *
     * @param a - Argument to be added.
     * @retval A reference to this so that add calls can be chained
     */
    ArgContainer &add(Arg *a);

    /**
     * Adds an argument group to the list of arguments to be parsed.
     *
     * All arguments in the group are added and the ArgGroup
     * object will validate that the input matches its
     * constraints.
     *
     * @param args - Argument group to be added.
     * @retval A reference to this so that add calls can be chained
     */
    ArgContainer &add(ArgGroup &args);

    // Internal, do not use
    void addToArgList(Arg *a);

    /**
     * \deprecated Use OneOf instead.
     */
    void xorAdd(Arg &a, Arg &b);

    /**
     * \deprecated Use OneOf instead.
     */
    void xorAdd(const std::vector<Arg *> &xors);

    /**
     * Parses the command line.
     * \param argc - Number of arguments.
     * \param argv - Array of arguments.
     */
    void parse(int argc, const char *const *argv);

    /**
     * Parses the command line.
     * \param args - A vector of strings representing the args.
     * args[0] is still the program name.
     */
    void parse(std::vector<std::string> &args);

    void setOutput(CmdLineOutput *co);

    std::string getVersion() const { return _version; }

    std::string getProgramName() const { return _progName; }

    // TOOD: Get rid of getArgList
    std::list<Arg *> getArgList() const { return _argList; }
    std::list<ArgGroup *> getArgGroups() {
        std::list<ArgGroup *> groups = _argGroups;
        groups.push_back(&_autoArgs);
        return groups;
    }

    char getDelimiter() const { return _delimiter; }
    std::string getMessage() const { return _message; }
    bool hasHelpAndVersion() const { return _helpAndVersion; }

    /**
     * Disables or enables CmdLine's internal parsing exception handling.
     *
     * @param state Should CmdLine handle parsing exceptions internally?
     */
    void setExceptionHandling(const bool state);

    /**
     * Returns the current state of the internal exception handling.
     *
     * @retval true Parsing exceptions are handled internally.
     * @retval false Parsing exceptions are propagated to the caller.
     */
    bool hasExceptionHandling() const { return _handleExceptions; }

    /**
     * Allows the CmdLine object to be reused.
     */
    void reset();

    /**
     * Allows unmatched args to be ignored. By default false.
     *
     * @param ignore If true the cmdline will ignore any unmatched args
     * and if false it will behave as normal.
     */
    void ignoreUnmatched(const bool ignore);

    /**
     * Sets an optional message translator callback.
     */
    template <typename T>
    void setMessageTranslator(const T &translator) {
        delete _messageTranslator;
        _messageTranslator = new MessageTranslatorImpl<T>(translator);
        _refreshBuiltinDescriptions();
    }

    /**
     * Translates a built-in message.
     */
    std::string translateMessage(const std::string &messageId,
                                 const std::string &fallback) const {
        if (_messageTranslator != NULL) {
            return _messageTranslator->translate(messageId, fallback);
        }
        return fallback;
    }

    void beginIgnoring() { _ignoring = true; }
    bool ignoreRest() { return _ignoring; }
};

///////////////////////////////////////////////////////////////////////////////
// Begin CmdLine.cpp
///////////////////////////////////////////////////////////////////////////////

inline CmdLine::CmdLine(const std::string &m, char delim, const std::string &v,
                        bool help)
    : _argList(),
      _standaloneArgs(),
      _autoArgs(),
      _argGroups(),
      _progName("not_set_yet"),
      _message(m),
      _version(v),
      _numRequired(0),
      _delimiter(delim),
      _hasOptionalUnlabeledArg(false),
      _registrationActive(false),
      _deleteOnExit(),
      _defaultOutput(),
      _output(&_defaultOutput),
      _handleExceptions(true),
      _messageTranslator(NULL),
      _ignoreArg(NULL),
      _helpArg(NULL),
      _versionArg(NULL),
      _helpAndVersion(help),
      _ignoreUnmatched(false),
      _ignoring(false) {
    _constructor();
}

inline void CmdLine::_constructor() {
    CmdLine::add(_standaloneArgs);
    _autoArgs._parser = this;
    _createIgnoreRestArgument();
    if (_helpAndVersion) _createHelpAndVersionArguments();
}

inline void CmdLine::_createIgnoreRestArgument() {
    Visitor *v = new IgnoreRestVisitor(*this);
    SwitchArg *ignore = new SwitchArg(
        Arg::flagStartString(), Arg::ignoreNameString(),
        translateMessage("ignore_rest_description",
                         "Ignores the rest of the labeled arguments following this flag."),
        false, v);
    _ignoreArg = ignore;
    _deleteOnExit(ignore);
    _deleteOnExit(v);
    _autoArgs.add(ignore);
    CmdLine::addToArgList(ignore);
}

inline void CmdLine::_createHelpAndVersionArguments() {
    Visitor *v = new HelpVisitor(this, &_output);
    SwitchArg *help = new SwitchArg(
        "h", "help",
        translateMessage("help_description",
                         "Displays usage information and exits."),
        false, v);
    _helpArg = help;
    _deleteOnExit(help);
    _deleteOnExit(v);

    v = new VersionVisitor(this, &_output);
    SwitchArg *vers = new SwitchArg(
        "", "version",
        translateMessage("version_description",
                         "Displays version information and exits."),
        false, v);
    _versionArg = vers;
    _deleteOnExit(vers);
    _deleteOnExit(v);

    // Retain the established registration order used by help output.
    _autoArgs.add(vers);
    CmdLine::addToArgList(vers);
    _autoArgs.add(help);
    CmdLine::addToArgList(help);
}

inline void CmdLine::_refreshBuiltinDescriptions() {
    if (_ignoreArg != NULL) {
        _ignoreArg->setDescription(translateMessage(
            "ignore_rest_description",
            "Ignores the rest of the labeled arguments following this flag."));
    }
    if (_helpArg != NULL) {
        _helpArg->setDescription(translateMessage(
            "help_description",
            "Displays usage information and exits."));
    }
    if (_versionArg != NULL) {
        _versionArg->setDescription(translateMessage(
            "version_description",
            "Displays version information and exits."));
    }
}

inline void CmdLine::xorAdd(const std::vector<Arg *> &args) {
    OneOf *group = new OneOf(*this);
    _deleteOnExit(group);

    for (std::vector<Arg *>::const_iterator it = args.begin(); it != args.end();
         ++it) {
        group->add(**it);
    }
}

inline void CmdLine::xorAdd(Arg &a, Arg &b) {
    std::vector<Arg *> ors;
    ors.push_back(&a);
    ors.push_back(&b);
    xorAdd(ors);
}

// Insertion hooks run against a private list. Bindings are
// temporarily visible to hooks and restored unless the whole operation commits.
class CmdLine::Registration {
public:
    explicit Registration(CmdLine &cmd)
        : _cmd(cmd), _args(cmd._argList), _numRequired(cmd._numRequired),
          _hasOptionalUnlabeledArg(cmd._hasOptionalUnlabeledArg), _bindings() {
        if (_cmd._registrationActive)
            throw SpecificationException("Argument registration cannot be re-entered");
        _cmd._registrationActive = true;
    }

    ~Registration() throw() {
        for (Bindings::reverse_iterator it = _bindings.rbegin();
             it != _bindings.rend(); ++it)
            it->first->_parserDelimiter = it->second;
        _cmd._registrationActive = false;
    }

    void add(Arg &arg) {
        arg._validateAliases(_cmd._delimiter, Arg::flagStartString(),
                             Arg::nameStartString());
        validateConflicts(arg);
        validatePositionalOrder(arg);
        _bindings.push_back(std::make_pair(&arg, arg._parserDelimiter));
        arg._parserDelimiter = &_cmd._delimiter;
        arg.addToList(_args);
        if (arg.isRequired()) ++_numRequired;
    }

    void commit() {
        _cmd._argList.swap(_args);
        _cmd._numRequired = _numRequired;
        _cmd._hasOptionalUnlabeledArg = _hasOptionalUnlabeledArg;
        _bindings.clear();
    }

private:
    void validateConflicts(const Arg &arg) const {
        for (ArgListIterator it = _args.begin(); it != _args.end(); ++it)
            if (arg.conflictsWith(**it) ||
                arg._aliasConflictsWith(**it, Arg::flagStartString(),
                                        Arg::nameStartString()) ||
                (*it)->_aliasConflictsWith(arg, Arg::flagStartString(),
                                           Arg::nameStartString()))
                throw SpecificationException(
                    "Argument with same flag/name already exists!", arg.longID());
    }

    void validatePositionalOrder(Arg &arg) {
        if (!arg.hasLabel()) {
            if (_hasOptionalUnlabeledArg)
                throw SpecificationException(
                    "You can't specify ANY Unlabeled Arg following an optional "
                    "Unlabeled Arg", arg.longID());
            if (!arg.isRequired() && !arg.acceptsMultipleValues())
                _hasOptionalUnlabeledArg = true;
        }
    }

    Registration(const Registration &);
    Registration &operator=(const Registration &);
    typedef std::vector<std::pair<Arg *, const char *> > Bindings;
    CmdLine &_cmd;
    std::list<Arg *> _args;
    int _numRequired;
    bool _hasOptionalUnlabeledArg;
    Bindings _bindings;
};

inline ArgContainer &CmdLine::add(ArgGroup &group) {
    if (group._parser)
        throw SpecificationException("Arg group can have only one parser");
    std::list<ArgGroup *> groups(_argGroups);
    groups.push_back(&group);
    Registration registration(*this);
    for (ArgGroup::iterator it = group.begin(); it != group.end(); ++it)
        registration.add(**it);
    group._parser = this;
    _argGroups.swap(groups);
    registration.commit();
    return *this;
}

inline ArgContainer &CmdLine::add(Arg &a) { return add(&a); }

inline void CmdLine::addToArgList(Arg *a) {
    Registration registration(*this);
    registration.add(*a);
    registration.commit();
}

inline ArgContainer &CmdLine::add(Arg *a) {
    ArgGroup::Container members(_standaloneArgs._args);
    members.push_back(a);
    Registration registration(*this);
    registration.add(*a);
    _standaloneArgs._args.swap(members);
    registration.commit();
    return *this;
}

inline void CmdLine::parse(int argc, const char *const *argv) {
    // this step is necessary so that we have easy access to
    // mutable strings.
    std::vector<std::string> args;
    for (int i = 0; i < argc; i++) args.push_back(argv[i]);

    parse(args);
}

inline void CmdLine::parse(std::vector<std::string> &args) {
    bool shouldExit = false;
    int estat = 0;

    try {
        _setProgramName(args);
        const int requiredCount = _parseArguments(args);
        _validateRequiredArguments(requiredCount);
    } catch (ArgException &e) {
        // If we're not handling the exceptions, rethrow.
        if (!_handleExceptions) {
            throw;
        }

        try {
            _output->failure(*this, e);
        } catch (ExitException &ee) {
            estat = ee.getExitStatus();
            shouldExit = true;
        }
    } catch (ExitException &ee) {
        // If we're not handling the exceptions, rethrow.
        if (!_handleExceptions) {
            throw;
        }

        estat = ee.getExitStatus();
        shouldExit = true;
    }

    if (shouldExit) exit(estat);
}

inline void CmdLine::_setProgramName(std::vector<std::string> &args) {
    if (args.empty()) {
        // https://sourceforge.net/p/tclap/bugs/30/
        throw CmdLineParseException(
            "The args vector must not be empty, "
            "the first entry should contain the "
            "program's name.");
    }

    // TODO(macbishop): Maybe store the full name somewhere?
    _progName = basename(args.front());
    args.erase(args.begin());
}

inline int CmdLine::_parseArguments(std::vector<std::string> &args) {
    int requiredCount = 0;
    for (int i = 0; static_cast<unsigned int>(i) < args.size(); i++) {
        bool matched = _processArgument(i, args, requiredCount);

        // checks to see if the argument is an empty combined
        // switch and if so, then we've actually matched it
        if (!matched && _emptyCombined(args[i])) matched = true;

        if (!matched && !ignoreRest() && !_ignoreUnmatched)
            throw(
                CmdLineParseException("Couldn't find match "
                                      "for argument",
                                      args[i]));
    }

    return requiredCount;
}

inline bool CmdLine::_processArgument(int &index,
                                      std::vector<std::string> &args,
                                      int &requiredCount) {
    for (ArgListIterator it = _argList.begin(); it != _argList.end(); it++) {
        Arg &arg = **it;
        // Count a required argument only on its first match.
        bool alreadySet = arg.isSet();
        bool ignore = arg.isIgnoreable() && ignoreRest();
        if (!ignore && arg.processArg(&index, args)) {
            requiredCount += (!alreadySet && arg.isRequired()) ? 1 : 0;
            return true;
        }
    }

    return false;
}

inline void CmdLine::_validateRequiredArguments(int requiredCount) {
    std::list<ArgGroup *> missingArgGroups;
    // Once all arguments have been parsed, check that we don't
    // violate any constraints.
    for (std::list<ArgGroup *>::iterator it = _argGroups.begin();
         it != _argGroups.end(); ++it) {
        bool missingRequired = (*it)->validate();
        if (missingRequired) {
            missingArgGroups.push_back(*it);
        }
    }

    if (requiredCount < _numRequired || !missingArgGroups.empty()) {
        missingArgsException(missingArgGroups);
    }

    if (requiredCount > _numRequired) {
        throw(CmdLineParseException(
            translateMessage("too_many_arguments", "Too many arguments!")));
    }
}

inline bool CmdLine::_emptyCombined(const std::string &s) {
    if (s.length() > 0 && s[0] != Arg::flagStartChar()) return false;

    for (int i = 1; static_cast<unsigned int>(i) < s.length(); i++)
        if (s[i] != Arg::blankChar()) return false;

    return true;
}

inline void CmdLine::missingArgsException(
    const std::list<ArgGroup *> &missing) {
    int count = 0;

    std::string missingArgList;
    for (ArgListIterator it = _argList.begin(); it != _argList.end(); it++) {
        if ((*it)->isRequired() && !(*it)->isSet()) {
            missingArgList += (*it)->getName();
            missingArgList += ", ";
            count++;
        }
    }

    for (std::list<ArgGroup *>::const_iterator it = missing.begin();
         it != missing.end(); it++) {
        missingArgList += (*it)->getName();
        missingArgList += ", ";
        count++;
    }

    missingArgList = missingArgList.substr(0, missingArgList.length() - 2);

    std::string msg;
    if (count > 1)
        msg = translateMessage("required_arguments_missing",
                               "Required arguments missing: ");
    else
        msg = translateMessage("required_argument_missing",
                               "Required argument missing: ");

    msg += missingArgList;

    throw(CmdLineParseException(msg));
}

inline void CmdLine::setOutput(CmdLineOutput *co) { _output = co; }

inline void CmdLine::setExceptionHandling(const bool state) {
    _handleExceptions = state;
}

inline void CmdLine::reset() {
    _ignoring = false;
    for (ArgListIterator it = _argList.begin(); it != _argList.end(); it++)
        (*it)->reset();

    _progName.clear();
}

inline void CmdLine::ignoreUnmatched(const bool ignore) {
    _ignoreUnmatched = ignore;
}

///////////////////////////////////////////////////////////////////////////////
// End CmdLine.cpp
///////////////////////////////////////////////////////////////////////////////

}  // namespace TCLAP

#endif  // TCLAP_CMD_LINE_H
