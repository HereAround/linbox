/* linbox/element/archetype.h
 * Copyright (C) 1999-2001 William J Turner,
 *               2001 Bradford Hovinen
 *
 * Written by William J Turner <wjturner@math.ncsu.edu>,
 *            Bradford Hovinen <hovinen@cis.udel.edu>
 *
 * ========LICENCE========
 * This file is part of the library LinBox.
 *
  * LinBox is free software: you can redistribute it and/or modify
 * it under the terms of the  GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ========LICENCE========
 */

/*!@file element/archetype.h
 * @ingroup element
 * @brief NO DOC
 * @see \ref Archetypes
 */
#ifndef __LINBOX_element_archetype_H
#define __LINBOX_element_archetype_H

#include "linbox/element/abstract.h"

namespace LinBox
{

	// Forward declarations
	class FieldArchetype;
	class RandIterArchetype;

	/** @brief Field and Ring element interface specification and archetypical instance class.
	  \ingroup element

	 * Element classes must contain public default constructor, copy constructor,
	 * assignment operator, and destructor.  Note that primitive types
	 * such as int and double meet this specification.

	 * In particular, the copy constructor is used to allow elements to
	 * be passed by value to a function.

	 * Note that the documentation below of the specific methods, describes
	 * them as implemented in the archetypic element class.

*/
	class ElementArchetype {
	public:

		/** @name Common Object Interface for LinBox Field elements.
		 * These methods are required of all \ref LinBox
		 * @link Fields field@endlink elements.
		 */
		//@{

		/** Default constructor.
		 * This constructor is required to allow
		 * @link Fields field@endlink elements to be primitive \c C++ types.
		 * Because constructor does not know what @link Fields field@endlink
		 * the element belongs to, it cannot actually construct the element.
		 * In this implementation, the constructor it sets \c _elem_ptr
		 * to the null pointer.  Initialization of the element is done through
		 * the field function init where the field is known.
		 */
		ElementArchetype (void) { _elem_ptr = 0; }

		/** Copy constructor.
		 * This constructor is required to allow
		 * @link Fields field@endlink elements to be primitive C++ types,
		 * and to allow field elements to be passed by value into
		 * functions.
		 * Constructs @link Fields field@endlink element by copying the
		 * @link Fields field@endlink element.
		 * In this implementation, this means copying the element to
		 * which \c a._elem_ptr points.
		 * @param  a field element.
		 */
		ElementArchetype (const ElementArchetype &a)
		{
			if (a._elem_ptr != 0)
				_elem_ptr = a._elem_ptr->clone ();
			else
				_elem_ptr = 0;
		}

		/** Destructor.
		 * In this implementation, this destroys element by deleting field
		 * element to which \c _elem_ptr points.
		 */
		~ElementArchetype () { if (_elem_ptr != 0) delete _elem_ptr; }

		/** Assignment operator.
		 * Assigns element a to element.
		 * In this implementation, this is done
		 * by copying field element to which \c _elem_ptr points.
		 * @param  a field element.
		 */
		ElementArchetype &operator=(const ElementArchetype &a)
		{
			if (this != &a) { // guard against self-assignment
				if (_elem_ptr != 0) delete _elem_ptr;
				if (a._elem_ptr != 0) _elem_ptr = a._elem_ptr->clone ();
			}

			return *this;
		}

		//@} Common Object Interface

		/** @name Implementation-Specific Methods.
		 * These methods are not required of all LinBox field elements
		 * and are included only for this implementation of the archetype.
		 */
		//@{

		/** Constructor.
		 * Constructs field element from pointer to \ref ElementAbstract
		 * Not part of the interface.
		 * Creates new copy of element object in dynamic memory.
		 * @param  elem_ptr  pointer to \ref ElementAbstract
		 */
		ElementArchetype (ElementAbstract *elem_ptr) :
			_elem_ptr (elem_ptr->clone ())
		{}

		//@}

	private:

		friend class RingArchetype;
		friend class FieldArchetype;
		friend class RandIterArchetype;

		/** @name Implementation-Specific Data.
		 * This data is not required of all LinBox field elements
		 * and is included only for this implementation of the archetype.
		 */
		//@{

		/** Pointer to parameterized field element.
		 * Not part of the common object interface for \ref LinBox field elements.
		 * Included to avoid code bloat.
		 */
		mutable ElementAbstract *_elem_ptr;

		//@} Non-Interface

	}; // class element

} // namespace LinBox

#endif // __LINBOX_element_archetype_H

// Local Variables:
// mode: C++
// tab-width: 4
// indent-tabs-mode: nil
// c-basic-offset: 4
// End:
// vim:sts=4:sw=4:ts=4:et:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
