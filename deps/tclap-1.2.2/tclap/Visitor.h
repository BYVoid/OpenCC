
/****************************************************************************** 
 * 
 *  file:  Visitor.h
 * 
 *  Copyright (c) 2003, Michael E. Smoot .
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


#ifndef TCLAP_VISITOR_H
#define TCLAP_VISITOR_H

namespace TCLAP {

/**
 * A base class that defines the interface for visitors.
 */
class Visitor
{
	public:

		/**
		 * Constructor. Does nothing.
		 */
		Visitor() { }

		/**
		 * Destructor. Does nothing.
		 */
		virtual ~Visitor() { }

		/**
		 * Does nothing. Should be overridden by child.
		 */
		virtual void visit() { }
};

}

#endif
