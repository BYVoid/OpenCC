// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  DocBookOutput.h
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

#ifndef TCLAP_DOC_BOOK_OUTPUT_H
#define TCLAP_DOC_BOOK_OUTPUT_H

#include <tclap/Arg.h>
#include <tclap/ArgGroup.h>
#include <tclap/CmdLineInterface.h>
#include <tclap/CmdLineOutput.h>

#include <algorithm>
#include <iostream>
#include <list>
#include <string>
#include <vector>

namespace TCLAP {

/**
 * A class that generates DocBook output for usage() method for the
 * given CmdLine and its Args.
 */
class DocBookOutput : public CmdLineOutput {
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

    DocBookOutput() : theDelimiter('=') {}

protected:
    // Escape raw data once, at the XML output boundary (text and attributes).
    static std::string escapeXml(const std::string &text) {
        std::string result;
        for (std::string::const_iterator it = text.begin(); it != text.end(); ++it) {
            switch (*it) {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '\"': result += "&quot;"; break;
            case '\'': result += "&apos;"; break;
            default: result += *it; break;
            }
        }
        return result;
    }

    static std::string valueLabel(const Arg &arg) {
        const std::string id = arg.shortID();
        const std::string label = arg.getFlag().empty()
                                      ? arg.nameStartString() + arg.getName()
                                      : arg.flagStartString() + arg.getFlag();
        const std::string::size_type labelStart = id.find(label);
        if (labelStart != std::string::npos) {
            const std::string::size_type open = labelStart + label.size() + 1;
            const std::string::size_type close = id.rfind('>');
            if (open < id.size() && id[open] == '<' &&
                close != std::string::npos && close > open)
                return id.substr(open + 1, close - open - 1);
        }
        return id;  // Custom IDs without the usual value wrapper.
    }

    /**
     * Substitutes the char r for string x in string s.
     * \param s - The string to operate on.
     * \param r - The char to replace.
     * \param x - What to replace r with.
     */
    void substituteSpecialChars(std::string &s, char r,
                                const std::string &x) const;
    void removeChar(std::string &s, char r) const;

    void printShortGroup(const ArgGroup &group, bool labelsOnly);
    static bool hasVisiblePositional(const ArgGroup &group) {
        for (ArgGroup::const_iterator it = group.begin(); it != group.end(); ++it)
            if (!(*it)->hasLabel() && (*it)->visibleInHelp()) return true;
        return false;
    }

    void printShortArg(Arg *it, bool required);
    void printLongArg(const ArgGroup &it) const;

    char theDelimiter;

private:
    void _printSynopsis(CmdLineInterface &cmd,
                        const std::list<ArgGroup *> &groups,
                        const std::string &programName);
    static ArgGroup *_findArgGroup(const std::list<ArgGroup *> &groups,
                                   const Arg *arg);
    void _printLongEntry(Arg &arg, bool forceRequired) const;
    void _printAliasTerms(const Arg &arg) const;
    void _printValuePlaceholder(const Arg &arg) const;
};

inline void DocBookOutput::version(CmdLineInterface &_cmd) {
    std::cout << _cmd.getVersion() << std::endl;
}

namespace internal {
inline const char *GroupChoice(const ArgGroup &group) {
    if (!group.showAsGroup()) {
        return "plain";
    }

    if (group.isRequired()) {
        return "req";
    }

    return "opt";
}
}  // namespace internal

inline void DocBookOutput::printShortGroup(const ArgGroup &group, bool labelsOnly) {
    int visible = 0;
    for (ArgGroup::const_iterator it = group.begin(); it != group.end(); ++it)
        if ((*it)->visibleInHelp() && (!labelsOnly || (*it)->hasLabel())) ++visible;
    if (visible > 1)
        std::cout << "<group choice='" << internal::GroupChoice(group) << "'>\n";
    for (ArgGroup::const_iterator it = group.begin(); it != group.end(); ++it) {
        Arg *arg = *it;
        if (!arg->visibleInHelp() || (labelsOnly && !arg->hasLabel())) continue;
        printShortArg(arg, arg->isRequired() || (visible == 1 && group.isRequired()));
    }
    if (visible > 1) std::cout << "</group>\n";
}

inline void DocBookOutput::usage(CmdLineInterface &_cmd) {
    std::list<ArgGroup *> argSets = _cmd.getArgGroups();
    std::string progName = _cmd.getProgramName();
    std::string xversion = _cmd.getVersion();
    theDelimiter = _cmd.getDelimiter();

    std::cout << "<?xml version='1.0'?>\n";
    std::cout
        << "<!DOCTYPE refentry PUBLIC \"-//OASIS//DTD DocBook XML V4.2//EN\"\n";
    std::cout
        << "\t\"http://www.oasis-open.org/docbook/xml/4.2/docbookx.dtd\">\n\n";

    std::cout << "<refentry>\n";

    std::cout << "<refmeta>\n";
    std::cout << "<refentrytitle>" << escapeXml(progName) << "</refentrytitle>\n";
    std::cout << "<manvolnum>1</manvolnum>\n";
    std::cout << "</refmeta>\n";

    std::cout << "<refnamediv>\n";
    std::cout << "<refname>" << escapeXml(progName) << "</refname>\n";
    std::cout << "<refpurpose>" << escapeXml(_cmd.getMessage()) << "</refpurpose>\n";
    std::cout << "</refnamediv>\n";

    _printSynopsis(_cmd, argSets, progName);

    std::cout << "<refsect1>\n";
    std::cout << "<title>Description</title>\n";
    std::cout << "<para>\n";
    std::cout << escapeXml(_cmd.getMessage()) << '\n';
    std::cout << "</para>\n";
    std::cout << "</refsect1>\n";

    std::cout << "<refsect1>\n";
    std::cout << "<title>Options</title>\n";

    std::cout << "<variablelist>\n";

    for (std::list<ArgGroup *>::iterator sit = argSets.begin();
         sit != argSets.end(); ++sit) {
        printLongArg(**sit);
    }

    std::cout << "</variablelist>\n";
    std::cout << "</refsect1>\n";

    std::cout << "<refsect1>\n";
    std::cout << "<title>Version</title>\n";
    std::cout << "<para>\n";
    std::cout << escapeXml(xversion) << '\n';
    std::cout << "</para>\n";
    std::cout << "</refsect1>\n";

    std::cout << "</refentry>" << std::endl;
}

inline void DocBookOutput::_printSynopsis(CmdLineInterface &cmd,
                                          const std::list<ArgGroup *> &groups,
                                          const std::string &programName) {
    std::cout << "<refsynopsisdiv>\n";
    std::cout << "<cmdsynopsis>\n";

    std::cout << "<command>" << escapeXml(programName) << "</command>\n";

    for (std::list<ArgGroup *>::const_iterator it = groups.begin();
         it != groups.end(); ++it) {
        const ArgGroup &group = **it;
        if (group.isExclusive() && hasVisiblePositional(group)) continue;
        printShortGroup(group, true);
    }
    const std::list<Arg *> operands = cmd.getArgList();
    std::list<ArgGroup *> shown;
    for (ArgListIterator it = operands.begin(); it != operands.end(); ++it) {
        Arg *arg = *it;
        if (arg->hasLabel() || !arg->visibleInHelp()) continue;
        ArgGroup *owner = _findArgGroup(groups, arg);
        if (owner && owner->isExclusive()) {
            if (std::find(shown.begin(), shown.end(), owner) == shown.end()) {
                printShortGroup(*owner, false);
                shown.push_back(owner);
            }
        } else {
            printShortArg(arg, arg->isRequired());
        }
    }

    std::cout << "</cmdsynopsis>\n";
    std::cout << "</refsynopsisdiv>\n";
}

inline ArgGroup *DocBookOutput::_findArgGroup(
    const std::list<ArgGroup *> &groups, const Arg *arg) {
    for (std::list<ArgGroup *>::const_iterator group = groups.begin();
         group != groups.end(); ++group)
        if (std::find((*group)->begin(), (*group)->end(), arg) !=
            (*group)->end())
            return *group;
    return NULL;
}

inline void DocBookOutput::failure(CmdLineInterface &_cmd, ArgException &e) {
    static_cast<void>(_cmd);  // unused
    std::cout << e.what() << std::endl;
    throw ExitException(1);
}

inline void DocBookOutput::substituteSpecialChars(std::string &s, char r,
                                                  const std::string &x) const {
    size_t p;
    while ((p = s.find_first_of(r)) != std::string::npos) {
        s.erase(p, 1);
        s.insert(p, x);
    }
}

inline void DocBookOutput::removeChar(std::string &s, char r) const {
    size_t p;
    while ((p = s.find_first_of(r)) != std::string::npos) {
        s.erase(p, 1);
    }
}

inline void DocBookOutput::printShortArg(Arg *a, bool required) {

    std::string choice = "opt";
    if (required) {
        choice = "plain";
    }

    std::cout << "<arg choice='" << choice << '\'';
    if (a->acceptsMultipleValues()) std::cout << " rep='repeat'";

    std::cout << '>';
    if (!a->hasLabel()) {
        std::cout << "<replaceable>" << escapeXml(a->getName())
                  << "</replaceable></arg>" << std::endl;
        return;
    }

    if (!a->getFlag().empty())
        std::cout << escapeXml(std::string(1, a->flagStartChar()) + a->getFlag());
    else
        std::cout << escapeXml(a->nameStartString() + a->getName());
    if (a->isValueRequired()) {
        _printValuePlaceholder(*a);
    }
    std::cout << "</arg>" << std::endl;
}

inline void DocBookOutput::printLongArg(const ArgGroup &group) const {

    bool forceRequired = group.isRequired() && CountVisibleArgs(group) == 1;
    for (ArgGroup::const_iterator it = group.begin(); it != group.end(); ++it) {
        Arg &a = **it;
        if (!a.visibleInHelp()) {
            continue;
        }
        _printLongEntry(a, forceRequired);
    }
}

inline void DocBookOutput::_printLongEntry(Arg &a, bool forceRequired) const {
    std::string desc = a.getDescription(forceRequired || a.isRequired());

    std::cout << "<varlistentry>\n";
    if (!a.hasLabel()) {
        std::cout << "<term><replaceable>" << escapeXml(a.getName())
                  << "</replaceable></term>\n<listitem><para>"
                  << escapeXml(desc) << "</para></listitem>\n</varlistentry>"
                  << std::endl;
        return;
    }

    if (!a.getFlag().empty()) {
        std::cout << "<term>\n";
        std::cout << "<option>";
        std::cout << escapeXml(std::string(1, a.flagStartChar()) + a.getFlag());
        std::cout << "</option>\n";
        std::cout << "</term>\n";
    }

    std::cout << "<term>\n";
    std::cout << "<option>";
    std::cout << escapeXml(a.nameStartString() + a.getName());
    if (a.isValueRequired()) {
        _printValuePlaceholder(a);
    }

    std::cout << "</option>\n";
    std::cout << "</term>\n";

    _printAliasTerms(a);

    std::cout << "<listitem>\n";
    std::cout << "<para>\n";
    std::cout << escapeXml(desc) << '\n';
    std::cout << "</para>\n";
    std::cout << "</listitem>\n";

    std::cout << "</varlistentry>" << std::endl;
}

inline void DocBookOutput::_printAliasTerms(const Arg &a) const {
    for (std::vector<ArgAlias>::const_iterator alias = a.aliases().begin();
         alias != a.aliases().end(); ++alias) {
        for (int spelling = 0; spelling < 2; ++spelling) {
            const std::string &label =
                spelling == 0 ? alias->flag : alias->name;
            if (label.empty()) continue;
            std::cout << "<term><option>"
                      << escapeXml((spelling == 0 ? a.flagStartString()
                                                  : a.nameStartString()) +
                                   label);
            if (a.isValueRequired())
                std::cout << escapeXml(std::string(1, theDelimiter))
                          << "<replaceable>" << escapeXml(valueLabel(a))
                          << "</replaceable>";
            std::cout << "</option></term>\n";
        }
    }
}

inline void DocBookOutput::_printValuePlaceholder(const Arg &arg) const {
    const std::string label = valueLabel(arg);
    std::cout << escapeXml(std::string(1, theDelimiter));
    std::cout << "<replaceable>" << escapeXml(label) << "</replaceable>";
}

}  // namespace TCLAP
#endif  // TCLAP_DOC_BOOK_OUTPUT_H
