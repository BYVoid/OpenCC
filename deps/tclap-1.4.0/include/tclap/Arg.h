// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  Arg.h
 *
 *  Copyright (c) 2003, Michael E. Smoot .
 *  Copyright (c) 2004, Michael E. Smoot, Daniel Aarno .
 *  Copyright (c) 2017 Google Inc.
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

#ifndef TCLAP_ARG_H
#define TCLAP_ARG_H

#ifdef HAVE_TCLAP_CONFIG_H
#include <tclap/TCLAPConfig.h>
#endif

// Define to 1 before including TCLAP to retain the historical truncation of
// ValueLike input at its first NUL byte. Use one setting across the program.
#ifndef TCLAP_ALLOW_NUL_TRUNCATION
#define TCLAP_ALLOW_NUL_TRUNCATION 0
#endif

#include <tclap/ArgException.h>
#include <tclap/ArgTraits.h>
#include <tclap/CmdLineInterface.h>
#include <tclap/StandardTraits.h>
#include <tclap/Visitor.h>
#include <tclap/sstream.h>

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <list>
#include <string>
#include <vector>

namespace TCLAP {

/** An additional short flag and/or long name for the same argument. */
struct ArgAlias {
    std::string flag;
    std::string name;
    ArgAlias(const std::string &f, const std::string &n) : flag(f), name(n) {}
};

/**
 * A virtual base class that defines the essential data for all arguments.
 * This class, or one of its existing children, must be subclassed to do
 * anything.
 */
class Arg {
    friend class CmdLine;

private:
    // NULL uses the legacy static delimiter for unregistered arguments.
    const char *_parserDelimiter;
    std::vector<ArgAlias> _aliases;
    // Registration validates against the destination parser's conventions.
    void _validateAliases(char delimiter, const std::string &flagPrefix,
                          const std::string &namePrefix) const;
    bool _aliasConflictsWithDialect(const ArgAlias &alias, char delimiter,
                                    const std::string &flagPrefix,
                                    const std::string &namePrefix) const;
    void _validateAliasToken(const std::string &label, const std::string &prefix,
                             std::vector<std::string> &tokens) const;
    void _validateAliasLabels(const std::string &flag,
                               const std::string &name) const;
    bool _matchesDeclaredToken(const std::string &token,
                               const std::string &flagPrefix,
                               const std::string &namePrefix) const;
    bool _aliasConflictsWith(const Arg &other, const std::string &flagPrefix,
                             const std::string &namePrefix) const;

    /**
     * Prevent accidental copying.
     */
    Arg(const Arg &rhs);

    /**
     * Prevent accidental copying.
     */
    Arg &operator=(const Arg &rhs);

    /**
     * The delimiter that separates an argument flag/name from the
     * value.
     */
    static char &delimiterRef() {
        static char delim = ' ';
        return delim;
    }

protected:
    bool matchesShortFlag(const std::string &flag) const;
    bool matchesLongName(const std::string &name) const;
    /**
     * The single char flag used to identify the argument.
     * This value (preceded by a dash {-}), can be used to identify
     * an argument on the command line.  The _flag can be blank,
     * in fact this is how unlabeled args work.  Unlabeled args must
     * override appropriate functions to get correct handling. Note
     * that the _flag does NOT include the dash as part of the flag.
     */
    std::string _flag;

    /**
     * A single word namd identifying the argument.
     * This value (preceded by two dashed {--}) can also be used
     * to identify an argument on the command line.  Note that the
     * _name does NOT include the two dashes as part of the _name. The
     * _name cannot be blank.
     */
    std::string _name;

    /**
     * Description of the argument.
     */
    std::string _description;

    /**
     * Indicating whether the argument is required.
     */
    const bool _required;

    /**
     * Label to be used in usage description.  Normally set to
     * "required", but can be changed when necessary.
     */
    std::string _requireLabel;

    /**
     * Indicates whether a value is required for the argument.
     * Note that the value may be required but the argument/value
     * combination may not be, as specified by _required.
     */
    bool _valueRequired;

    /**
     * Indicates whether the argument has been set.
     * Indicates that a value on the command line has matched the
     * name/flag of this argument and the values have been set accordingly.
     */
    bool _alreadySet;

    /** Indicates the value specified to set this flag (like -a or --all).
     */
    std::string _setBy;

    /**
     * A pointer to a visitor object.
     * The visitor allows special handling to occur as soon as the
     * argument is matched.  This defaults to NULL and should not
     * be used unless absolutely necessary.
     */
    Visitor *_visitor;

    /**
     * Whether this argument can be ignored, if desired.
     */
    bool _ignoreable;

    bool _acceptsMultipleValues;

    /**
     * Indicates if the argument is visible in the help output (e.g.,
     * when specifying --help).
     */
    bool _visibleInHelp;

    /**
     * Performs the special handling described by the Visitor.
     */
    void _checkWithVisitor() const;

    /**
     * Primary constructor. YOU (yes you) should NEVER construct an Arg
     * directly, this is a base class that is extended by various children
     * that are meant to be used.  Use SwitchArg, ValueArg, MultiArg,
     * UnlabeledValueArg, or UnlabeledMultiArg instead.
     *
     * \param flag - The flag identifying the argument.
     * \param name - The name identifying the argument.
     * \param desc - The description of the argument, used in the usage.
     * \param req - Whether the argument is required.
     * \param valreq - Whether the a value is required for the argument.
     * \param v - The visitor checked by the argument. Defaults to NULL.
     */
    Arg(const std::string &flag, const std::string &name,
        const std::string &desc, bool req, bool valreq, Visitor *v = NULL);

public:
    /** Add equivalent spellings before registration. Either may be blank,
     * but not both. Positional arguments cannot have aliases. Registration
     * freezes aliases; reset() retains them. Values, callbacks and group
     * membership are shared with the canonical argument.
     * For self-registering constructors, use a non-registering constructor
     * instead, add aliases, then call parser.add(arg).
     */
    Arg &addAlias(const std::string &flag, const std::string &name = "");

    /** Alias spellings in insertion order; no prefixes are stored. */
    const std::vector<ArgAlias> &aliases() const { return _aliases; }

    /**
     * Destructor.
     */
    virtual ~Arg();

    /**
     * Adds this to the specified list of Args.
     * \param argList - The list to add this to.
     */
    virtual void addToList(std::list<Arg *> &argList) const;

    /**
     * The delimiter that separates an argument flag/name from the
     * value.
     */
    static char delimiter() { return delimiterRef(); }

    /** Returns this argument's parser delimiter, or the legacy default.
     * Custom arguments should use this instead of the static delimiter().
     */
    char getDelimiter() const {
        return _parserDelimiter ? *_parserDelimiter : delimiter();
    }

    /**
     * The char used as a place holder when SwitchArgs are combined.
     * Currently set to the bell char (ASCII 7).
     */
    static char blankChar() { return '\a'; }

/**
 * The char that indicates the beginning of a flag.  Defaults to '-', but
 * clients can define TCLAP_FLAGSTARTCHAR to override.
 */
#ifndef TCLAP_FLAGSTARTCHAR
#define TCLAP_FLAGSTARTCHAR '-'
#endif
    static char flagStartChar() { return TCLAP_FLAGSTARTCHAR; }

/**
 * The sting that indicates the beginning of a flag.  Defaults to "-", but
 * clients can define TCLAP_FLAGSTARTSTRING to override. Should be the same
 * as TCLAP_FLAGSTARTCHAR.
 */
#ifndef TCLAP_FLAGSTARTSTRING
#define TCLAP_FLAGSTARTSTRING "-"
#endif
    static const std::string flagStartString() { return TCLAP_FLAGSTARTSTRING; }

/**
 * The sting that indicates the beginning of a name.  Defaults to "--", but
 *  clients can define TCLAP_NAMESTARTSTRING to override.
 */
#ifndef TCLAP_NAMESTARTSTRING
#define TCLAP_NAMESTARTSTRING "--"
#endif
    static const std::string nameStartString() { return TCLAP_NAMESTARTSTRING; }

    /**
     * The name used to identify the ignore rest argument.
     */
    static const std::string ignoreNameString() { return "ignore_rest"; }

    /**
     * Sets the legacy delimiter for unregistered arguments and static callers.
     * Registered built-in arguments use their owning parser delimiter.
     * \param c - The character that delimits flags/names from values.
     */
    static void setDelimiter(char c) { delimiterRef() = c; }

    /**
     * Pure virtual method meant to handle the parsing and value assignment
     * of the string on the command line.
     * \param i - Pointer the the current argument in the list.
     * \param args - Mutable list of strings. What is
     * passed in from main.
     */
    virtual bool processArg(int *i, std::vector<std::string> &args) = 0;

    /**
     * Operator ==.
     * Legacy equality operator, retained for source compatibility.
     * Registration uses conflictsWith(), not this virtual comparison.
     * \param a - The Arg to be compared to this.
     */
    virtual bool operator==(const Arg &a) const;

    /**
     * Checks declared registration conflicts: the same object, the same
     * name, or the same nonempty short flag, including aliases. Descriptions do not conflict.
     * This nonvirtual rule also applies to custom arguments; additional
     * tokens recognized by processArg() are not inferred.
     * \param a - The Arg to compare with this argument.
     */
    bool conflictsWith(const Arg &a) const;

    /**
     * Returns the argument flag.
     */
    const std::string &getFlag() const;

    /**
     * Returns the argument name.
     */
    const std::string &getName() const;

    /**
     * Returns the argument description.
     */
    std::string getDescription() const { return getDescription(_required); }

    /**
     * Updates the argument description.
     */
    virtual void setDescription(const std::string &desc) { _description = desc; }

    /**
     * Returns the argument description.
     *
     * @param required if the argument should be treated as
     * required when described.
     */
    std::string getDescription(bool required) const {
        return (required ? "(" + _requireLabel + ") " : "") + _description;
    }

    /**
     * Indicates whether the argument is required.
     */
    virtual bool isRequired() const;

    /**
     * Indicates whether a value must be specified for argument.
     */
    bool isValueRequired() const;

    /**
     * Indicates whether the argument has already been set.  Only true
     * if the arg has been matched on the command line.
     */
    bool isSet() const;

    /**
     * Returns the value specified to set this flag (like -a or --all).
     */
    const std::string &setBy() const { return _setBy; }

    /**
     * Indicates whether the argument can be ignored, if desired.
     */
    bool isIgnoreable() const;

    /**
     * A method that tests whether a string matches this argument.
     * This is generally called by the processArg() method.  This
     * method could be re-implemented by a child to change how
     * arguments are specified on the command line.
     * \param s - The string to be compared to the flag/name to determine
     * whether the arg matches.
     */
    virtual bool argMatches(const std::string &s) const;

    /**
     * Returns a simple string representation of the argument.
     * Primarily for debugging.
     */
    virtual std::string toString() const;

    /**
     * Returns a short ID for the usage.
     * \param valueId - The value used in the id.
     */
    virtual std::string shortID(const std::string &valueId = "val") const;

    /**
     * Returns a long ID for the usage.
     * \param valueId - The value used in the id.
     */
    virtual std::string longID(const std::string &valueId = "val") const;

    /**
     * Trims a value off of the flag.
     * \param flag - The string from which the flag and value will be
     * trimmed. Contains the flag once the value has been trimmed.
     * \param value - Where the value trimmed from the string will
     * be stored.
     */
    virtual void trimFlag(std::string &flag, std::string &value) const;

    /**
     * Checks whether a given string has blank chars, indicating that
     * it is a combined SwitchArg.  If so, return true, otherwise return
     * false.
     * \param s - string to be checked.
     */
    bool _hasBlanks(const std::string &s) const;

    /**
     * Used for MultiArgs to determine whether args can still be
     * set.
     */
    virtual bool allowMore();

    /**
     * Use by output classes to determine whether an Arg accepts
     * multiple values.
     */
    virtual bool acceptsMultipleValues();

    /**
     * Clears the Arg object and allows it to be reused by new
     * command lines.
     */
    virtual void reset();

    /**
     * Hide this argument from the help output (e.g., when
     * specifying the --help flag or on error.
     */
    virtual void hideFromHelp(bool hide = true) { _visibleInHelp = !hide; }

    /**
     * Returns true if this Arg is visible in the help output.
     */
    virtual bool visibleInHelp() const { return _visibleInHelp; }

    virtual bool hasLabel() const { return true; }
};

/**
 * Typedef of an Arg list iterator.
 */
typedef std::list<Arg *>::const_iterator ArgListIterator;

/**
 * Typedef of an Arg vector iterator.
 */
typedef std::vector<Arg *>::const_iterator ArgVectorIterator;

/**
 * Typedef of a Visitor list iterator.
 */
typedef std::list<Visitor *>::const_iterator VisitorListIterator;

/*
 * Extract a value of type T from it's string representation contained
 * in strVal. The ValueLike parameter used to select the correct
 * specialization of ExtractValue depending on the value traits of T.
 * ValueLike traits use operator>> to assign the value from strVal.
 */
template <typename T>
void ExtractValue(T &destVal, const std::string &strVal, ValueLike vl) {
    static_cast<void>(vl);  // Avoid warning about unused vl
#if !TCLAP_ALLOW_NUL_TRUNCATION
    if (strVal.find('\0') != std::string::npos)
        throw ArgParseException("Embedded NUL in argument value");
#endif
    istringstream is(strVal.c_str());

    int valuesRead = 0;
    // Let the extractor decide whether an explicitly empty input is valid.
    // Built-in numeric extraction fails; a custom extractor may accept it.
    if (strVal.empty()) {
        is >> destVal;
        ++valuesRead;
    }
    while (is.good()) {
        if (is.peek() != EOF)
#ifdef TCLAP_SETBASE_ZERO
            is >> std::setbase(0) >> destVal;
#else
            is >> destVal;
#endif
        else
            break;

        valuesRead++;
    }

    if (is.fail())
        throw(
            ArgParseException("Couldn't read argument value "
                              "from string '" +
                              strVal + "'"));

    if (valuesRead > 1)
        throw(
            ArgParseException("More than one valid value parsed from "
                              "string '" +
                              strVal + "'"));
}

/*
 * Extract a value of type T from it's string representation contained
 * in strVal. The ValueLike parameter used to select the correct
 * specialization of ExtractValue depending on the value traits of T.
 * StringLike uses assignment (operator=) to assign from strVal.
 */
template <typename T>
void ExtractValue(T &destVal, const std::string &strVal, StringLike sl) {
    static_cast<void>(sl);  // Avoid warning about unused sl
    SetString(destVal, strVal);
}

//////////////////////////////////////////////////////////////////////
// BEGIN Arg.cpp
//////////////////////////////////////////////////////////////////////

inline Arg::Arg(const std::string &flag, const std::string &name,
                const std::string &desc, bool req, bool valreq, Visitor *v)
    : _parserDelimiter(NULL),
      _aliases(),
      _flag(flag),
      _name(name),
      _description(desc),
      _required(req),
      _requireLabel("required"),
      _valueRequired(valreq),
      _alreadySet(false),
      _setBy(),
      _visitor(v),
      _ignoreable(true),
      _acceptsMultipleValues(false),
      _visibleInHelp(true) {
    if (_flag.length() > 1)
        throw(SpecificationException(
            "Argument flag can only be one character long", Arg::toString()));

    if (_name != ignoreNameString() &&
        (_flag == Arg::flagStartString() || _flag == Arg::nameStartString() ||
         _flag == " "))
        throw(SpecificationException(
            "Argument flag cannot be either '" + Arg::flagStartString() +
                "' or '" + Arg::nameStartString() + "' or a space.",
            Arg::toString()));

    if ((_name.substr(0, Arg::flagStartString().length()) ==
         Arg::flagStartString()) ||
        (_name.substr(0, Arg::nameStartString().length()) ==
         Arg::nameStartString()) ||
        (_name.find(" ", 0) != std::string::npos))
        throw(SpecificationException("Argument name begin with either '" +
                                         Arg::flagStartString() + "' or '" +
                                         Arg::nameStartString() + "' or space.",
                                     Arg::toString()));
}

inline Arg::~Arg() {}

inline std::string Arg::shortID(const std::string &valueId) const {
    std::string id = "";

    if (_flag != "")
        id = Arg::flagStartString() + _flag;
    else
        id = Arg::nameStartString() + _name;

    if (_valueRequired) id += std::string(1, getDelimiter()) + valueId;

    return id;
}

inline std::string Arg::longID(const std::string &valueId) const {
    std::string id = "";

    if (_flag != "") {
        id += Arg::flagStartString() + _flag;

        if (_valueRequired) id += std::string(1, getDelimiter()) + valueId;

        id += ",  ";
    }

    id += Arg::nameStartString() + _name;

    if (_valueRequired) id += std::string(1, getDelimiter()) + valueId;

    for (std::vector<ArgAlias>::const_iterator it = _aliases.begin();
         it != _aliases.end(); ++it) {
        if (!it->flag.empty()) {
            id += ",  " + flagStartString() + it->flag;
            if (_valueRequired) id += std::string(1, getDelimiter()) + valueId;
        }
        if (!it->name.empty()) {
            id += ",  " + nameStartString() + it->name;
            if (_valueRequired) id += std::string(1, getDelimiter()) + valueId;
        }
    }
    return id;
}

inline bool Arg::_matchesDeclaredToken(const std::string &token,
                                       const std::string &flagPrefix,
                                       const std::string &namePrefix) const {
    if ((!_flag.empty() && token == flagPrefix + _flag) ||
        token == namePrefix + _name)
        return true;
    for (std::vector<ArgAlias>::const_iterator it = _aliases.begin();
         it != _aliases.end(); ++it)
        if ((!it->flag.empty() && token == flagPrefix + it->flag) ||
            (!it->name.empty() && token == namePrefix + it->name))
            return true;
    return false;
}

inline bool Arg::_aliasConflictsWith(const Arg &other,
                                     const std::string &flagPrefix,
                                     const std::string &namePrefix) const {
    for (std::vector<ArgAlias>::const_iterator it = _aliases.begin();
         it != _aliases.end(); ++it)
        if ((!it->flag.empty() &&
             other._matchesDeclaredToken(flagPrefix + it->flag, flagPrefix,
                                         namePrefix)) ||
            (!it->name.empty() &&
             other._matchesDeclaredToken(namePrefix + it->name, flagPrefix,
                                         namePrefix)))
            return true;
    return false;
}

inline void Arg::_validateAliases(char delimiter, const std::string &flagPrefix,
                                  const std::string &namePrefix) const {
    if (_aliases.empty()) return;
    std::vector<std::string> tokens;
    if (!_flag.empty()) tokens.push_back(flagPrefix + _flag);
    tokens.push_back(namePrefix + _name);
    for (std::vector<ArgAlias>::const_iterator it = _aliases.begin();
         it != _aliases.end(); ++it) {
        if (_aliasConflictsWithDialect(*it, delimiter, flagPrefix, namePrefix))
            throw SpecificationException("Alias conflicts with parser dialect",
                                         toString());
        _validateAliasToken(it->flag, flagPrefix, tokens);
        _validateAliasToken(it->name, namePrefix, tokens);
    }
}

inline bool Arg::_aliasConflictsWithDialect(
    const ArgAlias &alias, char delimiter, const std::string &flagPrefix,
    const std::string &namePrefix) const {
    return (!alias.flag.empty() &&
            (alias.flag == flagPrefix || alias.flag == namePrefix)) ||
           (!alias.name.empty() &&
            ((!flagPrefix.empty() && alias.name.find(flagPrefix) == 0) ||
             (!namePrefix.empty() && alias.name.find(namePrefix) == 0))) ||
           (_valueRequired &&
            (alias.flag.find(delimiter) != std::string::npos ||
             alias.name.find(delimiter) != std::string::npos));
}

inline void Arg::_validateAliasToken(const std::string &label,
                                     const std::string &prefix,
                                     std::vector<std::string> &tokens) const {
    if (label.empty()) return;
    const std::string token = prefix + label;
    for (std::vector<std::string>::const_iterator it = tokens.begin();
         it != tokens.end(); ++it)
        if (*it == token)
            throw SpecificationException("Duplicate argument alias token",
                                         toString());
    tokens.push_back(token);
}

inline bool Arg::matchesShortFlag(const std::string &flag) const {
    if (flag.empty()) return false;
    if (flag == _flag) return true;
    for (std::vector<ArgAlias>::const_iterator it = _aliases.begin();
         it != _aliases.end(); ++it)
        if (flag == it->flag) return true;
    return false;
}

inline bool Arg::matchesLongName(const std::string &name) const {
    if (name.empty()) return false;
    if (name == _name) return true;
    for (std::vector<ArgAlias>::const_iterator it = _aliases.begin();
         it != _aliases.end(); ++it)
        if (name == it->name) return true;
    return false;
}

inline Arg &Arg::addAlias(const std::string &flag, const std::string &name) {
    if (_parserDelimiter)
        throw SpecificationException("Aliases must be added before registration", toString());
    if (!hasLabel())
        throw SpecificationException("Positional arguments cannot have aliases", toString());
    _validateAliasLabels(flag, name);
    if (matchesShortFlag(flag) || matchesLongName(name))
        throw SpecificationException("Duplicate argument alias", toString());
    _aliases.push_back(ArgAlias(flag, name));
    return *this;
}

inline void Arg::_validateAliasLabels(const std::string &flag,
                                      const std::string &name) const {
    if ((flag.empty() && name.empty()) || flag.size() > 1 ||
        flag == flagStartString() || flag == nameStartString() ||
        flag.find_first_of(" \t\r\n\v\f\a") != std::string::npos ||
        flag.find('\0') != std::string::npos ||
        name.find_first_of(" \t\r\n\v\f") != std::string::npos ||
        name.find('\0') != std::string::npos ||
        (!name.empty() && (name.find(flagStartString()) == 0 ||
                          name.find(nameStartString()) == 0)))
        throw SpecificationException("Invalid argument alias", toString());
    const std::string labels = flag + name;
    for (std::string::const_iterator it = labels.begin(); it != labels.end();
         ++it)
        if (static_cast<unsigned char>(*it) < 0x20 || *it == '\x7f')
            throw SpecificationException("Control byte in argument alias",
                                         toString());
}

inline bool Arg::conflictsWith(const Arg &a) const {
    if (this == &a || _name == a._name || matchesLongName(a._name) || matchesShortFlag(a._flag))
        return true;
    for (std::vector<ArgAlias>::const_iterator it = a._aliases.begin();
         it != a._aliases.end(); ++it)
        if (matchesShortFlag(it->flag) || matchesLongName(it->name)) return true;
    return false;
}

inline bool Arg::operator==(const Arg &a) const {
    return (_flag != "" && _flag == a._flag) || _name == a._name;
}

inline const std::string &Arg::getFlag() const { return _flag; }

inline const std::string &Arg::getName() const { return _name; }

inline bool Arg::isRequired() const { return _required; }

inline bool Arg::isValueRequired() const { return _valueRequired; }

inline bool Arg::isSet() const { return _alreadySet; }

inline bool Arg::isIgnoreable() const { return _ignoreable; }

inline bool Arg::argMatches(const std::string &argFlag) const {
    const std::string shortPrefix = flagStartString();
    const std::string longPrefix = nameStartString();
    return argFlag == longPrefix + _name ||
           (argFlag.find(shortPrefix) == 0 &&
            matchesShortFlag(argFlag.substr(shortPrefix.size()))) ||
           (argFlag.find(longPrefix) == 0 &&
            matchesLongName(argFlag.substr(longPrefix.size())));
}

inline std::string Arg::toString() const {
    std::string s = "";

    if (_flag != "") s += Arg::flagStartString() + _flag + " ";

    s += "(" + Arg::nameStartString() + _name + ")";

    return s;
}

inline void Arg::_checkWithVisitor() const {
    if (_visitor != NULL) _visitor->visit();
}

/**
 * Implementation of trimFlag.
 */
inline void Arg::trimFlag(std::string &flag, std::string &value) const {
    int stop = 0;
    for (int i = 0; static_cast<unsigned int>(i) < flag.length(); i++)
        if (flag[i] == getDelimiter()) {
            stop = i;
            break;
        }

    if (stop > 1) {
        value = flag.substr(stop + 1);
        flag = flag.substr(0, stop);
    }
}

/**
 * Implementation of _hasBlanks.
 */
inline bool Arg::_hasBlanks(const std::string &s) const {
    for (int i = 1; static_cast<unsigned int>(i) < s.length(); i++)
        if (s[i] == Arg::blankChar()) return true;

    return false;
}

/**
 * Overridden by Args that need to added to the end of the list.
 */
inline void Arg::addToList(std::list<Arg *> &argList) const {
    argList.push_front(const_cast<Arg *>(this));
}

inline bool Arg::allowMore() { return false; }

inline bool Arg::acceptsMultipleValues() { return _acceptsMultipleValues; }

inline void Arg::reset() { _alreadySet = false; }

//////////////////////////////////////////////////////////////////////
// END Arg.cpp
//////////////////////////////////////////////////////////////////////

}  // namespace TCLAP

#endif  // TCLAP_ARG_H
