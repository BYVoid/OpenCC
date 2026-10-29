// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  StdOutput.h
 *
 *  Copyright (c) 2004, Michael E. Smoot
 *  Copyright (c) 2017, Google LLC
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

#ifndef TCLAP_STD_OUTPUT_H
#define TCLAP_STD_OUTPUT_H

#include <tclap/Arg.h>
#include <tclap/ArgGroup.h>
#include <tclap/CmdLineInterface.h>
#include <tclap/CmdLineOutput.h>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace TCLAP {

/**
 * A class that isolates any output from the CmdLine object so that it
 * may be easily modified.
 */
class StdOutput : public CmdLineOutput {
public:
    /**
     * Prints the usage to stdout.  Can be overridden to
     * produce alternative behavior.
     * \param c - The CmdLine object the output is generated for.
     */
    virtual void usage(CmdLineInterface &c);

    /**
     * Prints the version to stdout. Can be overridden
     * to produce alternative behavior.
     * \param c - The CmdLine object the output is generated for.
     */
    virtual void version(CmdLineInterface &c);

    /**
     * Prints (to stderr) an error message, short usage
     * Can be overridden to produce alternative behavior.
     * \param c - The CmdLine object the output is generated for.
     * \param e - The ArgException that caused the failure.
     */
    virtual void failure(CmdLineInterface &c, ArgException &e);

protected:
    /**
     * Writes a brief usage message with short args.
     * \param c - The CmdLine object the output is generated for.
     * \param os - The stream to write the message to.
     */
    void _shortUsage(CmdLineInterface &c, std::ostream &os) const;

    /**
     * Writes a longer usage message with long and short args,
     * provides descriptions and prints message.
     * \param c - The CmdLine object the output is generated for.
     * \param os - The stream to write the message to.
     */
    void _longUsage(CmdLineInterface &c, std::ostream &os) const;

    /**
     * This function inserts line breaks and indents long strings
     * according the  params input. It will only break lines at spaces,
     * commas and pipes.
     * \param os - The stream to be printed to.
     * \param s - The string to be printed.
     * \param maxWidth - The maxWidth allowed for the output line.
     * \param indentSpaces - The number of spaces to indent the first line.
     * \param secondLineOffset - The number of spaces to indent the second
     * and all subsequent lines in addition to indentSpaces.
     */
    void spacePrint(std::ostream &os, const std::string &s, int maxWidth,
                    int indentSpaces, int secondLineOffset) const;

private:
    enum { _usageWidth = 75 };

    void _partitionGroups(std::list<ArgGroup *> &groups,
                          std::list<ArgGroup *> &exclusiveGroups,
                          std::list<ArgGroup *> &nonExclusiveGroups) const;
    void _printShortSwitches(std::ostream &os, std::string &switches,
                             std::list<ArgGroup *> &groups) const;
    void _printLongSwitches(std::ostream &os,
                            std::list<ArgGroup *> &groups) const;
    void _printShortOptions(std::ostream &os,
                            std::list<ArgGroup *> &groups) const;
    void _printShortOperands(CmdLineInterface &cmd, std::ostream &os,
                             std::list<ArgGroup *> &groups) const;
    void _printLabeledGroups(CmdLineInterface &cmd, std::ostream &os,
                              std::list<ArgGroup *> &groups) const;
    void _printLongOperands(CmdLineInterface &cmd, std::ostream &os,
                             std::list<ArgGroup *> &groups) const;
    void _printExclusiveLongGroup(CmdLineInterface &cmd, std::ostream &os,
                                  ArgGroup &group) const;
    void _printGroupHeading(CmdLineInterface &cmd, std::ostream &os,
                            const ArgGroup &group) const;
    void _printLongEntry(std::ostream &os, const Arg &arg, bool required,
                         int indent) const;
};

inline void StdOutput::version(CmdLineInterface &_cmd) {
    std::string progName = _cmd.getProgramName();
    std::string xversion = _cmd.getVersion();

    std::cout << std::endl
              << progName << "  "
              << _cmd.translateMessage("version_label", "version:")
              << " " << xversion << std::endl
              << std::endl;
}

inline void StdOutput::usage(CmdLineInterface &_cmd) {
    std::cout << std::endl
              << _cmd.translateMessage("usage_header", "USAGE: ")
              << std::endl
              << std::endl;

    _shortUsage(_cmd, std::cout);

    std::cout << std::endl << std::endl
              << _cmd.translateMessage("where_header", "Where: ")
              << std::endl
              << std::endl;

    _longUsage(_cmd, std::cout);

    std::cout << std::endl;
}

inline void StdOutput::failure(CmdLineInterface &_cmd, ArgException &e) {
    std::string progName = _cmd.getProgramName();

    std::cerr << _cmd.translateMessage("parse_error_header", "PARSE ERROR:")
              << " " << e.argId() << std::endl
              << "             " << e.error() << std::endl
              << std::endl;

    if (_cmd.hasHelpAndVersion()) {
        std::cerr << _cmd.translateMessage("brief_usage_header", "Brief USAGE: ")
                  << std::endl;

        _shortUsage(_cmd, std::cerr);

        std::cerr << std::endl
                  << _cmd.translateMessage("complete_usage_hint",
                                       "For complete USAGE and HELP type: ")
                  << std::endl
                  << "   " << progName << " " << Arg::nameStartString()
                  << "help" << std::endl
                  << std::endl;
    } else {
        usage(_cmd);
    }

    throw ExitException(1);
}

// TODO: Remove this
inline void removeChar(std::string &s, char r) {
    size_t p;
    while ((p = s.find_first_of(r)) != std::string::npos) {
        s.erase(p, 1);
    }
}

inline bool cmpSwitch(const char &a, const char &b) {
    int lowa = std::tolower(a);
    int lowb = std::tolower(b);

    if (lowa == lowb) {
        return a < b;
    }

    return lowa < lowb;
}

namespace internal {
inline bool IsVisibleShortSwitch(const Arg &arg) {
    return !(arg.getName() == Arg::ignoreNameString() ||
             arg.isValueRequired() || arg.getFlag() == "") &&
           arg.visibleInHelp();
}

inline bool IsVisibleLongSwitch(const Arg &arg) {
    return (arg.getName() != Arg::ignoreNameString() &&
            !arg.isValueRequired() && arg.getFlag() == "" &&
            arg.visibleInHelp());
}

inline bool IsVisibleOption(const Arg &arg) {
    return (arg.getName() != Arg::ignoreNameString() && arg.isValueRequired() &&
            arg.hasLabel() && arg.visibleInHelp());
}

inline bool CompareShortID(const Arg *a, const Arg *b) {
    if (a->getFlag() == "" && b->getFlag() != "") {
        return false;
    }
    if (b->getFlag() == "" && a->getFlag() != "") {
        return true;
    }

    return a->shortID() < b->shortID();
}

// TODO: Fix me not to put --gopt before -f
inline bool CompareOptions(std::pair<const Arg *, bool> a,
                           std::pair<const Arg *, bool> b) {
    // First optional, then required
    if (!a.second && b.second) {
        return true;
    }
    if (a.second && !b.second) {
        return false;
    }

    return CompareShortID(a.first, b.first);
}
// Group order is presentation order; positional operands use parser order.
inline ArgGroup *FindArgGroup(const std::list<ArgGroup *> &groups,
                              const Arg *arg) {
    for (std::list<ArgGroup *>::const_iterator it = groups.begin();
         it != groups.end(); ++it)
        if (std::find((*it)->begin(), (*it)->end(), arg) != (*it)->end())
            return *it;
    return NULL;
}

inline bool HasVisiblePositional(const ArgGroup &group) {
    for (ArgGroup::const_iterator it = group.begin(); it != group.end(); ++it)
        if (!(*it)->hasLabel() && (*it)->visibleInHelp()) return true;
    return false;
}

inline void PrintExclusiveShortGroup(std::ostream &os, const ArgGroup &group) {
    os << (group.isRequired() ? " {" : " [");
    std::vector<Arg *> args;
    for (ArgGroup::const_iterator it = group.begin(); it != group.end(); ++it)
        if ((*it)->visibleInHelp()) args.push_back(*it);
    std::sort(args.begin(), args.end(), CompareShortID);
    const char *separator = "";
    for (std::vector<Arg *>::const_iterator it = args.begin();
         it != args.end(); ++it) {
        os << separator << (*it)->shortID();
        separator = "|";
    }
    os << (group.isRequired() ? '}' : ']');
}
}  // namespace internal

/**
 * Usage statements should look like the manual pages.  Options w/o
 * operands come first, in alphabetical order inside a single set of
 * braces, upper case before lower case (AaBbCc...).  Next are options
 * with operands, in the same order, each in braces.  Then required
 * arguments in the order they are specified, followed by optional
 * arguments in the order they are specified.  A bar (`|') separates
 * either/or options/arguments, and multiple options/arguments which
 * are specified together are placed in a single set of braces.
 *
 * Use getprogname() instead of hardcoding the program name.
 *
 * "usage: f [-aDde] [-b b_arg] [-m m_arg] req1 req2 [opt1 [opt2]]\n"
 * "usage: f [-a | -b] [-c [-de] [-n number]]\n"
 */
inline void StdOutput::_shortUsage(CmdLineInterface &_cmd,
                                   std::ostream &os) const {
    std::list<ArgGroup *> argSets = _cmd.getArgGroups();

    std::ostringstream outp;
    outp << _cmd.getProgramName() + " ";

    std::string switches = Arg::flagStartString();
    std::list<ArgGroup *> exclusiveGroups;
    std::list<ArgGroup *> nonExclusiveGroups;
    _partitionGroups(argSets, exclusiveGroups, nonExclusiveGroups);
    _printShortSwitches(outp, switches, nonExclusiveGroups);
    _printLongSwitches(outp, nonExclusiveGroups);

    // Exclusive groups containing operands are printed with those operands.
    for (std::list<ArgGroup *>::iterator it = exclusiveGroups.begin();
         it != exclusiveGroups.end(); ++it) {
        if (!internal::HasVisiblePositional(**it))
            internal::PrintExclusiveShortGroup(outp, **it);
    }

    _printShortOptions(outp, nonExclusiveGroups);
    _printShortOperands(_cmd, outp, argSets);

    // if the program name is too long, then adjust the second line offset
    int secondLineOffset = static_cast<int>(_cmd.getProgramName().length()) + 2;
    if (secondLineOffset > _usageWidth / 2) secondLineOffset = static_cast<int>(_usageWidth / 2);

    spacePrint(os, outp.str(), _usageWidth, 3, secondLineOffset);
}

inline void StdOutput::_partitionGroups(
    std::list<ArgGroup *> &groups, std::list<ArgGroup *> &exclusiveGroups,
    std::list<ArgGroup *> &nonExclusiveGroups) const {
    for (std::list<ArgGroup *>::iterator sit = groups.begin();
         sit != groups.end(); ++sit) {
        if (CountVisibleArgs(**sit) <= 0) {
            continue;
        }

        if ((*sit)->isExclusive()) {
            exclusiveGroups.push_back(*sit);
        } else {
            nonExclusiveGroups.push_back(*sit);
        }
    }

    // Move "exclusive groups" that have at most a single item to
    // non-exclusive groups as exclusivit doesn't make sense with a
    // single option. This can happen if args are hidden in help for
    // example.
    for (std::list<ArgGroup *>::iterator it = exclusiveGroups.begin();
         it != exclusiveGroups.end();) {
        if (CountVisibleArgs(**it) < 2) {
            nonExclusiveGroups.push_back(*it);
            it = exclusiveGroups.erase(it);
        } else {
            ++it;
        }
    }
}

inline void StdOutput::_printShortSwitches(
    std::ostream &os, std::string &switches,
    std::list<ArgGroup *> &groups) const {
    bool hasShortSwitches = false;

    // First short switches (needs to be special because they are all
    // stuck together).
    for (std::list<ArgGroup *>::iterator sit = groups.begin();
         sit != groups.end(); ++sit) {
        for (ArgGroup::iterator it = (*sit)->begin(); it != (*sit)->end();
             ++it) {
            if (internal::IsVisibleShortSwitch(**it)) {
                switches += (*it)->getFlag();
                hasShortSwitches = true;
            }
        }

        std::sort(switches.begin(), switches.end(), cmpSwitch);
    }

    if (hasShortSwitches) os << " [" << switches << ']';
}

inline void StdOutput::_printLongSwitches(std::ostream &os,
                                          std::list<ArgGroup *> &groups) const {
    // Now do long switches (e.g., --version, but no -v)
    std::vector<Arg *> longSwitches;
    for (std::list<ArgGroup *>::iterator sit = groups.begin();
         sit != groups.end(); ++sit) {
        for (ArgGroup::iterator it = (*sit)->begin(); it != (*sit)->end();
             ++it) {
            Arg &arg = **it;
            if (internal::IsVisibleLongSwitch(arg)) {
                longSwitches.push_back(&arg);
            }
        }
    }

    std::sort(longSwitches.begin(), longSwitches.end(),
              internal::CompareShortID);
    for (std::vector<Arg *>::const_iterator it = longSwitches.begin();
         it != longSwitches.end(); ++it) {
        os << " [" << (**it).shortID() << ']';
    }
}

inline void StdOutput::_printShortOptions(std::ostream &os,
                                          std::list<ArgGroup *> &groups) const {
    // Next do options, we sort them later by optional first.
    std::vector<std::pair<const Arg *, bool> > options;
    for (std::list<ArgGroup *>::iterator sit = groups.begin();
         sit != groups.end(); ++sit) {
        for (ArgGroup::iterator it = (*sit)->begin(); it != (*sit)->end();
             ++it) {
            Arg &arg = **it;
            int visible = CountVisibleArgs(**sit);
            bool required = arg.isRequired();
            if (internal::IsVisibleOption(arg)) {
                if (visible == 1 && (**sit).isRequired()) {
                    required = true;
                }

                options.push_back(std::make_pair(&arg, required));
            }
        }
    }

    std::sort(options.begin(), options.end(), internal::CompareOptions);
    for (std::vector<std::pair<const Arg *, bool> >::const_iterator it =
             options.begin();
         it != options.end(); ++it) {
        const Arg &arg = *it->first;
        bool required = it->second;
        os << (required ? " " : " [");
        os << arg.shortID();
        os << (required ? "" : "]");
    }
}

inline void StdOutput::_printShortOperands(
    CmdLineInterface &cmd, std::ostream &os,
    std::list<ArgGroup *> &groups) const {
    // Positional order must match the order in which the parser consumes argv.
    const std::list<Arg *> operands = cmd.getArgList();
    std::list<ArgGroup *> shown;
    for (ArgListIterator it = operands.begin(); it != operands.end(); ++it) {
        const Arg &arg = **it;
        if (arg.hasLabel() || !arg.isValueRequired() || !arg.visibleInHelp())
            continue;
        ArgGroup *group = internal::FindArgGroup(groups, &arg);
        if (group && group->isExclusive() && CountVisibleArgs(*group) > 1) {
            if (std::find(shown.begin(), shown.end(), group) == shown.end()) {
                internal::PrintExclusiveShortGroup(os, *group);
                shown.push_back(group);
            }
            continue;
        }
        bool required = arg.isRequired() ||
                        (group && group->isRequired() && CountVisibleArgs(*group) == 1);
        os << (required ? " " : " [") << arg.shortID() << (required ? "" : "]");
    }
}

inline void StdOutput::_longUsage(CmdLineInterface &_cmd,
                                  std::ostream &os) const {
    std::string message = _cmd.getMessage();
    std::list<ArgGroup *> argSets = _cmd.getArgGroups();

    _printLabeledGroups(_cmd, os, argSets);
    _printLongOperands(_cmd, os, argSets);

    if (!message.empty()) {
        spacePrint(os, message, _usageWidth, 3, 0);
    }

    os.flush();
}

inline void StdOutput::_printLabeledGroups(
    CmdLineInterface &cmd, std::ostream &os, std::list<ArgGroup *> &groups) const {
    for (std::list<ArgGroup *>::iterator sit = groups.begin();
         sit != groups.end(); ++sit) {
        ArgGroup &argGroup = **sit;

        int visible = CountVisibleArgs(argGroup);
        bool exclusive = visible > 1 && argGroup.isExclusive();
        if (exclusive && internal::HasVisiblePositional(argGroup)) continue;
        bool forceRequired = visible == 1 && argGroup.isRequired();
        if (exclusive) {
            _printGroupHeading(cmd, os, argGroup);
        }

        for (ArgGroup::iterator it = argGroup.begin(); it != argGroup.end();
             ++it) {
            Arg &arg = **it;
            if (!arg.visibleInHelp()) {
                continue;
            }

            if (!arg.hasLabel()) {
                continue;
            }

            bool required = arg.isRequired() || forceRequired;
            _printLongEntry(os, arg, required, exclusive ? 6 : 3);
        }
    }
}

inline void StdOutput::_printLongOperands(
    CmdLineInterface &cmd, std::ostream &os, std::list<ArgGroup *> &groups) const {
    const std::list<Arg *> operands = cmd.getArgList();
    std::list<ArgGroup *> shown;
    for (ArgListIterator it = operands.begin(); it != operands.end(); ++it) {
        const Arg &arg = **it;
        if (arg.hasLabel() || !arg.visibleInHelp()) continue;
        ArgGroup *group = internal::FindArgGroup(groups, &arg);
        if (group && group->isExclusive() && CountVisibleArgs(*group) > 1) {
            if (std::find(shown.begin(), shown.end(), group) != shown.end()) continue;
            shown.push_back(group);
            _printExclusiveLongGroup(cmd, os, *group);
            continue;
        }
        const bool required = arg.isRequired() ||
                              (group && group->isRequired() && CountVisibleArgs(*group) == 1);
        _printLongEntry(os, arg, required, 3);
    }
}

inline void StdOutput::_printExclusiveLongGroup(
    CmdLineInterface &cmd, std::ostream &os, ArgGroup &group) const {
    _printGroupHeading(cmd, os, group);
    for (ArgGroup::iterator member = group.begin(); member != group.end(); ++member) {
        if (!(*member)->visibleInHelp()) continue;
        // Preserve longID() before the virtual isRequired() call.
        spacePrint(os, (*member)->longID(), _usageWidth, 6, 3);
        spacePrint(os, (*member)->getDescription((*member)->isRequired()), _usageWidth, 8, 0);
        os << '\n';
    }
}

inline void StdOutput::_printGroupHeading(CmdLineInterface &cmd,
                                          std::ostream &os,
                                          const ArgGroup &group) const {
    spacePrint(os,
               group.isRequired()
                   ? cmd.translateMessage("one_of_group", "One of:")
                   : cmd.translateMessage("either_of_group", "Either of:"),
               _usageWidth, 3, 0);
}

inline void StdOutput::_printLongEntry(std::ostream &os, const Arg &arg,
                                       bool required, int indent) const {
    spacePrint(os, arg.longID(), _usageWidth, indent, 3);
    spacePrint(os, arg.getDescription(required), _usageWidth, indent + 2, 0);
    os << '\n';
}

namespace {
inline void fmtPrintLine(std::ostream &os, const std::string &text, int maxWidth,
                         int indentSpaces, int secondLineOffset) {
    const std::string breakCharacters(" ,|");
    // String indentation uses unsigned counts; clamp negative geometry first.
    indentSpaces = std::max(indentSpaces, 0);
    secondLineOffset = std::max(secondLineOffset, 0);
    size_t availableWidth = std::max(maxWidth - indentSpaces, 0);
    std::string indentString(indentSpaces, ' ');
    size_t lineStart = 0;
    size_t lineEnd = 0;
    size_t textLength = text.length();
    for (;;) {
        if (textLength - lineStart <= availableWidth) {
            // Rest of string fits on line, just print the remainder
            os << indentString << text.substr(lineStart) << std::endl;
            return;
        }

        // Keep the last delimiter that fits within the available width.
        size_t nextBreak = lineEnd;
        while (nextBreak - lineStart <= availableWidth &&
               nextBreak != std::string::npos) {
            lineEnd = nextBreak;
            nextBreak = text.find_first_of(breakCharacters, lineEnd + 1);
        }

        if (lineEnd == lineStart) {
            // Split a long word. A zero width must still consume one byte;
            // subtracting from zero would underflow the unsigned width.
            lineEnd = availableWidth > 0 ? lineStart + availableWidth - 1
                                         : lineStart;
        }

        if (text[lineEnd] != ' ') {
            // Include delimiter before line break, unless it's a space
            lineEnd++;
        }

        os << indentString << text.substr(lineStart, lineEnd - lineStart) << '\n';

        // Avoid printing extra white space at start of a line
        for (; text[lineEnd] == ' '; lineEnd++) {
        }
        lineStart = lineEnd;

        if (secondLineOffset != 0) {
            // Adjust offset for following lines
            indentString.insert(indentString.end(), secondLineOffset, ' ');
            // Clamp the remaining width to prevent unsigned underflow.
            availableWidth -=
                std::min(availableWidth, static_cast<size_t>(secondLineOffset));
            secondLineOffset = 0;
        }
    }
}
}  // namespace

inline void StdOutput::spacePrint(std::ostream &os, const std::string &s,
                                  int maxWidth, int indentSpaces,
                                  int secondLineOffset) const {
    std::stringstream ss(s);
    std::string line;
    std::getline(ss, line);
    fmtPrintLine(os, line, maxWidth, indentSpaces, secondLineOffset);
    indentSpaces += secondLineOffset;

    while (std::getline(ss, line)) {
        fmtPrintLine(os, line, maxWidth, indentSpaces, 0);
    }
}

}  // namespace TCLAP

#endif  // TCLAP_STD_OUTPUT_H
