//%LICENSE////////////////////////////////////////////////////////////////
//
// Licensed to The Open Group (TOG) under one or more contributor license
// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
// this work for additional information regarding copyright ownership.
// Each contributor licenses this file to you under the OpenPegasus Open
// Source License; you may not use this file except in compliance with the
// License.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////
/* NOCHKSRC */

//

// Author: Humberto Rivero (hurivero@us.ibm.com)

//

// Modified By:

//

//%/////////////////////////////////////////////////////////////////////////////



#ifndef Pegasus_LanguageElementContainer_h

#define Pegasus_LanguageElementContainer_h





#include <cstdlib>

#include <cctype>

#ifdef PEGASUS_OS_HPUX
# ifdef HPUX_IA64_NATIVE_COMPILER
#  include <iostream>
# else
#  include <iostream.h>
# endif
#else
# include <iostream>
#endif

#include <Pegasus/Common/Linkage.h>

#include <Pegasus/Common/Config.h>

#include <Pegasus/Common/String.h>

#include <Pegasus/Common/Array.h>

#include <Pegasus/Common/LanguageElement.h>

#include <Pegasus/Common/LanguageParser.h>


#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES


PEGASUS_NAMESPACE_BEGIN



//////////////////////////////////////////////////////////////

//

// LanguageElementContainer::

//

//////////////////////////////////////////////////////////////



/** This class is a container class for AcceptLanguageElement

 */

class PEGASUS_COMMON_LINKAGE LanguageElementContainer{



public:



	/**	This member is used to represent an empty LanguageElementContainer. Using this 

        member avoids construction of an empty LanguageElementContainer 

        (e.g., LanguageElementContainer()).

	*/

	static const LanguageElementContainer EMPTY;

    

    /**

     * Default Constructor

     */

	LanguageElementContainer();    

	

	/**

	 * Constructor

	 * @param container Array<LanguageElement> - contructs the object with the elements in the array

	 */

	LanguageElementContainer(Array<LanguageElement> container);



	/** Copy Constructor

	 * @param rhs LanguageElementContainer

	 */

	LanguageElementContainer(const LanguageElementContainer &rhs);

	

	/**

	 * Destructor

	 */

	virtual ~LanguageElementContainer();

	

	/**

	 * Deep copy

	 * @param rhs LanguageElementContainer

	 */

	LanguageElementContainer operator=(LanguageElementContainer rhs);



	/** Gets an AcceptLanguageElement object at position index

	 * @param index int position of AcceptLanguageElement

	 * @return AcceptLanguageElement

	 */

	LanguageElement getLanguageElement(int index) const;



	/** Gets all AcceptLanguageElement objects in the container

	 * @return Array<AcceptLanguageElement>

	 */

	Array<LanguageElement> getAllLanguageElements() const;



	/** Returns the String representation of this container

	 * @return String

	 */

	virtual String toString() const;



	/** 

	 * Returns the length of the LanguageElementContainer object.

	 * @return Length of the container.

     */

    Uint32 size() const;



	/**

	 * True if the container has element (case INSENSITIVE match of element's language fields

	 * @param LanguageElement element 

	 * @return Boolean

	 */

	Boolean contains(LanguageElement element)const;



	/**

	 * Resets this object's iterator, should be called once before using itrNext()

	 */

	void itrStart();



	/**

	 * Returns the next element in the container

	 * Callers should call itrStart() ONCE before making calls to this function and

	 * anytime callers want the iterator reset to the beginning of the container.

	 * @return LanguageElement - the next element the container or LanguageElement::EMPTY_REF

	 * if the end of the container has been reached.

	 */

	LanguageElement& itrNext();



	/**

	 * Removes the element at index from the container.  

	 * @param index Uint32 - position of the element in the container to remove

	 */

	void remove(Uint32 index);

	

	/**

	 * Removes the element matching the parameter, returns -1 if not found

	 * @param element LanguageElement - element to remove

	 * @return int  -1 if element not found, otherwise returns the position of element before the remove.

	 */

	int remove(LanguageElement element);



	/**

	 * Clears the container of all elements

	 */

	void clear();



	/**

	 * Equality based on language fields only

	 */

	Boolean operator==(const LanguageElementContainer rhs)const;



	/**

	 * Inequality based on language fields only

	 */

	Boolean operator!=(const LanguageElementContainer rhs)const;

	

	/**

	 * Finds the element in the container and returns its position.

	 * @param element LanguageElement - element to find

	 * @return int index of element if found, otherwise -1

	 */

	int find(LanguageElement element)const;



protected:



  	Array<LanguageElement> container;        // holds AcceptLanguageElements



private:

	

	void buildLanguageElements(Array<String> values);



	void updateIterator();

	

    int itr_index;					// current position for iterator type access

	       

}; // end LanguageElementContainer



PEGASUS_NAMESPACE_END


#endif  // PEGASUS_USE_EXPERIMENTAL_INTERFACE


#endif

