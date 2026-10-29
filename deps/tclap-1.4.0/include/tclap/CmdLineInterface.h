// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  CmdLineInterface.h
 *
 *  Copyright (c) 2003, Michael E. Smoot .
 *  Copyright (c) 2004, Michael E. Smoot, Daniel Aarno.
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

#ifndef TCLAP_CMD_LINE_INTERFACE_H
#define TCLAP_CMD_LINE_INTERFACE_H

#include <tclap/ArgContainer.h>

#include <algorithm>
#include <iostream>
#include <list>
#include <string>
#include <vector>

namespace TCLAP {

class Arg;
class ArgGroup;
class CmdLineOutput;

/**
 * The base class that manages the command line definition and passes
 * along the parsing to the appropriate Arg classes.
 */
class CmdLineInterface : public ArgContainer {
    friend class ArgGroup;
protected:
    // Return true when the implementation handles the whole attachment.
    // The default lets ArgGroup keep the legacy custom-parser path.
    virtual bool attachGroup(ArgGroup &) { return false; }

public:
    /**
     * Destructor
     */
    virtual ~CmdLineInterface() {}

    /**
     * Adds an argument. Ownership is not transfered.
     * @param a - Argument to be added.
     * @retval A reference to this so that add calls can be chained
     */
    virtual ArgContainer &add(Arg &a) = 0;

    /**
     * Adds an argument. Ownership is not transfered.
     * @param a - Argument to be added.
     * @retval A reference to this so that add calls can be chained
     */
    virtual ArgContainer &add(Arg *a) = 0;

    // TODO: Rename this to something smarter or refactor this logic so
    // it's not needed.
    // Internal - do not use
    virtual void addToArgList(Arg *a) = 0;

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
    virtual ArgContainer &add(ArgGroup &args) = 0;

    /**
     * \deprecated Use OneOf instead.
     */
    virtual void xorAdd(Arg &a, Arg &b) = 0;

    /**
     * \deprecated Use OneOf instead.
     */
    virtual void xorAdd(const std::vector<Arg *> &xors) = 0;

    /**
     * Parses the command line.
     * \param argc - Number of arguments.
     * \param argv - Array of arguments.
     */
    virtual void parse(int argc, const char *const *argv) = 0;

    /**
     * Parses the command line.
     * \param args - A vector of strings representing the args.
     * args[0] is still the program name.
     */
    void parse(std::vector<std::string> &args);

    /**
     * \param co - CmdLineOutput object that we want to use instead.
     */
    virtual void setOutput(CmdLineOutput *co) = 0;

    /**
     * Returns the version string.
     */
    virtual std::string getVersion() const = 0;

    /**
     * Returns the program name string.
     */
    virtual std::string getProgramName() const = 0;

    /**
     * Returns the list of ArgGroups.
     */
    virtual std::list<ArgGroup *> getArgGroups() = 0;
    virtual std::list<Arg *> getArgList() const = 0;  // TODO: get rid of this

    /**
     * Returns the delimiter string.
     */
    virtual char getDelimiter() const = 0;

    /**
     * Returns the message string.
     */
    virtual std::string getMessage() const = 0;

    /**
     * Translates a built-in message using the current translator.
     */
    virtual std::string translateMessage(const std::string &messageId,
                                         const std::string &fallback) const = 0;

    /**
     * Indicates whether or not the help and version switches were created
     * automatically.
     */
    virtual bool hasHelpAndVersion() const = 0;

    /**
     * Resets the instance as if it had just been constructed so that the
     * instance can be reused.
     */
    virtual void reset() = 0;

    /**
     * Begin ignoring arguments since the "--" argument was specified.
     * \internal
     */
    virtual void beginIgnoring() = 0;

    /**
     * Whether to ignore the rest.
     * \internal
     */
    virtual bool ignoreRest() = 0;
};

}  // namespace TCLAP

#endif  // TCLAP_CMD_LINE_INTERFACE_H
