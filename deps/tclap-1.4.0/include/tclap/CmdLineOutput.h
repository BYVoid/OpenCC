// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  CmdLineOutput.h
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

#ifndef TCLAP_CMD_LINE_OUTPUT_H
#define TCLAP_CMD_LINE_OUTPUT_H

#include <tclap/Arg.h>
#include <tclap/ArgGroup.h>

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <list>
#include <string>
#include <vector>

namespace TCLAP {
const char PATH_SEPARATOR =
#ifdef _WIN32
	'\\';
#else
	'/';
#endif

class CmdLineInterface;
class ArgException;

/**
 * The interface that any output object must implement.
 */
class CmdLineOutput {
public:
    /**
     * Virtual destructor.
     */
    virtual ~CmdLineOutput() {}

    /**
     * Generates some sort of output for the USAGE.
     * \param c - The CmdLine object the output is generated for.
     */
    virtual void usage(CmdLineInterface &c) = 0;

    /**
     * Generates some sort of output for the version.
     * \param c - The CmdLine object the output is generated for.
     */
    virtual void version(CmdLineInterface &c) = 0;

    /**
     * Generates some sort of output for a failure.
     * \param c - The CmdLine object the output is generated for.
     * \param e - The ArgException that caused the failure.
     */
    virtual void failure(CmdLineInterface &c, ArgException &e) = 0;
};

inline bool isInArgGroup(const Arg *arg, const std::list<ArgGroup *> &argSets) {
    for (std::list<ArgGroup *>::const_iterator it = argSets.begin();
         it != argSets.end(); ++it) {
        if (std::find((*it)->begin(), (*it)->end(), arg) != (*it)->end()) {
            return true;
        }
    }
    return false;
}

inline void removeArgsInArgGroups(std::list<Arg *> &argList,
                                  const std::list<ArgGroup *> &argSets) {
    for (std::list<Arg *>::iterator it = argList.begin();
         it != argList.end();) {
        if (isInArgGroup(*it, argSets)) {
            it = argList.erase(it);
        } else {
            ++it;
        }
    }
}

inline std::string removeSuffix(std::string s, const std::string &suffix) {
	std::ptrdiff_t start = s.length() - suffix.length();
	if (start >= 0 && s.substr(start) == suffix) {
		return s.substr(0, start);
	}

	return s;
}

inline std::string basename(std::string s) {
    size_t p = s.find_last_of(PATH_SEPARATOR);
    if (p != std::string::npos) {
        s.erase(0, p + 1);
    }

	return removeSuffix(s, ".exe");
}

}  // namespace TCLAP

#endif  // TCLAP_CMD_LINE_OUTPUT_H
