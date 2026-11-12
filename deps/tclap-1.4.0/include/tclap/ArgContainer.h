// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ArgContainer.h
 *
 *  Copyright (c) 2018 Google LLC
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

#ifndef TCLAP_ARG_CONTAINER_H
#define TCLAP_ARG_CONTAINER_H

namespace TCLAP {

class Arg;

/**
 * Interface that allows adding an Arg to a "container".
 *
 * A container does not have to be a container in the C++ standard
 * library sense, just something that wants to hold on to references
 * to Arg's. The container does not own the added Arg's and it is the
 * user's responsibility to ensure the life time (scope) of the Arg's
 * outlives any operations on the container.
 */
class ArgContainer {
public:
    virtual ~ArgContainer() {}

    /**
     * Adds an argument. Ownership is not transfered.
     * \param a - Argument to be added.
     */
    virtual ArgContainer &add(Arg &a) = 0;

    /**
     * Adds an argument. Ownership is not transfered.
     * \param a - Argument to be added.
     */
    virtual ArgContainer &add(Arg *a) = 0;
};

}  // namespace TCLAP

#endif  // TCLAP_ARG_CONTAINER_H
