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

#ifndef Pegasus_AcceptLanguages_h
#define Pegasus_AcceptLanguages_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/LanguageElementContainer.h>
#include <Pegasus/Common/AcceptLanguageElement.h>

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

PEGASUS_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////
//
// AcceptLanguages::
//
//////////////////////////////////////////////////////////////

/** This class is a container class for AcceptLanguageElement
 */
class PEGASUS_COMMON_LINKAGE AcceptLanguages : public LanguageElementContainer{

public:

	/**	This member is used to represent an empty AcceptLanguages. Using this 
        member avoids construction of an empty AcceptLanguages 
        (e.g., AcceptLanguages()).
    */
    static const AcceptLanguages EMPTY;
	
	/**
	 * Constructor
	 */
	AcceptLanguages();

	/** Constructor
	 * @param hdr String complete value portion of AcceptLanguage header
	 */
	AcceptLanguages(String hdr);
	
	/**
	 * Constructor
	 * @param container Array<LanguageElement> - constructs the object with this array
	 */
	AcceptLanguages(Array<LanguageElement> container):LanguageElementContainer(container){}
	
	/**
	 * Constructor
	 * @param container Array<AcceptLanguageElement> - constructs the object with this array
	 */
	AcceptLanguages(Array<AcceptLanguageElement> container);
	
	/**
	 * Destrctor
	 */
	~AcceptLanguages(){}
	
	/**
	 * @return String - Returns a String representation of this object in AcceptLanguage header format
	 * according to the RFC
	 */
	String toString() const;
	
	/**
	 * @return ostream - Returns a representation of this object in AcceptLanguage header format
	 * according to the RFC
	 */
	PEGASUS_COMMON_LINKAGE friend PEGASUS_STD(ostream) & operator<<(PEGASUS_STD(ostream) &stream, AcceptLanguages al);
	
	/**
	 * Assignment 
	 * @param rhs AcceptLanguages
	 */
	AcceptLanguages operator=(AcceptLanguages rhs);

	/**
	 * Random access into this container given an index
	 * @param index int - index into the container
	 * @return AcceptLanguageElement - valid element 
	 * @throw IndexOutOfBounds Exception 
	 */
	AcceptLanguageElement getLanguageElement(int index) const;
	
	/**
	 * Fills in the array with all the language elements in the container
	 * @param elements Array<AcceptLanguageElement> & - array to fill in
	 */
	void getAllLanguageElements(Array<AcceptLanguageElement> & elements) const;
	
	/**
	 * Returns the next element in the container
	 * Callers should call itrStart() ONCE before making calls to this function and
	 * anytime callers want the iterator reset to the beginning of the container.
	 * @return AcceptLanguageElement - the next element the container or AcceptLanguageElement::EMPTY_REF
	 * if the end of the container has been reached.
	 */
	AcceptLanguageElement itrNext();
	
	/**
	 * Adds an element to this container.  The elements will be reordered according to the natural ordering
	 * of this container, that is, based on the quality value associated with the language_tag.
	 * @param element AcceptLanguageElement - element to add
	 */
	void add(AcceptLanguageElement element);

	/**
	 * Removes the element at index from the container.  The elements will be reordered according to the natural ordering
	 * of this container, that is, based on the quality value associated with the language_tag.
	 * @param index Uint32 - position of the element in the container to remove
	 */
	void remove(Uint32 index);

	/**
	 * Removes the element matching the parameter
	 * @param element AcceptLanguageElement - element to remove
	 * @return int  -1 if element not found, otherwise returns the position of element before the remove.
	 */
	int remove(AcceptLanguageElement element);
	
	/**
	 * Finds the element in the container and returns its position.
	 * @param element AcceptLanguageElement - element to find
	 * @return int index of element if found, otherwise -1
	 */
	int find(AcceptLanguageElement element);
	
	/**
	 * Finds the element in the container that matches the language_tag and quality.
	 * @param language_tag String - case insensitive match
	 * @param quality Real32 - the quality associated with the language_tag to match.
	 * @return int index of element if found, otherwise -1
	 */
	int find(String language_tag, Real32 quality);
	
	static AcceptLanguages getDefaultAcceptLanguages();
		
		

private:

	void buildLanguageElements(Array<String> values);

	void prioritize();
	       
}; // end AcceptLanguages

PEGASUS_NAMESPACE_END

#endif  // PEGASUS_USE_EXPERIMENTAL_INTERFACES

#endif
