// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  RangeConstraint.h
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

#ifndef TCLAP_RANGE_CONSTRAINT_H
#define TCLAP_RANGE_CONSTRAINT_H

#include <tclap/Constraint.h>

#include <sstream>
#include <string>

namespace TCLAP {

/**
 * A Constraint that requires a value to fall within a closed
 * [min, max] range, for the common "between X and Y" case that
 * otherwise requires writing a full Constraint<T> subclass by hand.
 * T must support <= and ostream insertion. Bounds are copied and owned.
 * Short IDs are cached; descriptions are formatted on request. Reversed or
 * unordered bounds accept no values; omitted optional defaults are unchecked.
 */
template <class T>
class RangeConstraint : public Constraint<T> {
public:
    /**
     * Constructor.
     * \param min - The smallest value that satisfies the constraint.
     * \param max - The largest value that satisfies the constraint.
     */
    RangeConstraint(const T &min, const T &max)
        : _min(min), _max(max), _shortId() {
        std::ostringstream os;
        os << '[' << _min << '-' << _max << ']';
        _shortId = os.str();
    }

    std::string description() const {
        std::ostringstream os;
        os << "value must be between " << _min << " and " << _max
           << " (inclusive)";
        return os.str();
    }

    std::string shortID() const { return _shortId; }

    bool check(const T &value) const {
        // Positive comparisons also reject unordered floating-point values.
        return _min <= value && value <= _max;
    }

private:
    T _min;
    T _max;
    std::string _shortId;
};

}  // namespace TCLAP

#endif  // TCLAP_RANGE_CONSTRAINT_H
