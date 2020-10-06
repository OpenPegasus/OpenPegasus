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



#ifndef Pegasus_AcceptLanguageElement_h

#define Pegasus_AcceptLanguageElement_h



#include <Pegasus/Common/Config.h>

#include <Pegasus/Common/LanguageElement.h>


#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES


PEGASUS_NAMESPACE_BEGIN



//////////////////////////////////////////////////////////////

//

// AcceptLanguageElement::

//

//////////////////////////////////////////////////////////////

    

/** This class is used to hold language information for a given AcceptLanguage

 *  header value.

 */ 

class PEGASUS_COMMON_LINKAGE AcceptLanguageElement : public LanguageElement{

	     

public:



	/**	This member is used to represent an empty AcceptLanguageElement. Using this 

        member avoids construction of an empty AcceptLanguageElement 

        (e.g., AcceptLanguageElement()).

    */

	static const AcceptLanguageElement EMPTY;



	/**

	 * This member is used to notify callers of AcceptLanguages::itrNext() that the end of the container

	 * has been reached and subsequent calls to AcceptLanguages::itrNext() will fail.

	 */

	static AcceptLanguageElement EMPTY_REF;



	/** Constructor

	 */

	AcceptLanguageElement(): LanguageElement(){quality = 0;}

	

	/**

	 * Constructor

	 * @param language String - language ex: en

	 * @param country String - country code ex: US

	 * @param variant String - variant ex: rab-oof

	 * @param quality Real32 - quality associated with the language, defaults to 1.0

	 */

	AcceptLanguageElement(String language, 

				     String country,

				     String variant,

				     Real32 quality = 1.0):

				     LanguageElement(language,

		             country,

		             variant,

		             quality){}

	/** Constructor

	 * @param language_tag String IANA language value

	 * @param q Real32 quality value for language, 0 < q < 1, defaults to 1

	 */

	AcceptLanguageElement(String language_tag, 

						  Real32 quality = 1.0): 

						  LanguageElement(language_tag, quality){}



	/** Copy Constructor 

	 */

	AcceptLanguageElement(const AcceptLanguageElement &rhs): 

						  LanguageElement(rhs){}



	~AcceptLanguageElement();



	/** Gets the quality value associated with the language

	 * @return Real32 quality value

	 */

	Real32 getQuality() const;



	/** String representation of the AcceptLanguageElement

	 * @return the String

	 */

	String toString() const;

	

	/**

	 * Assignment operator, deep copy

	 * @param rhs AcceptLanguageElement

	 */

	AcceptLanguageElement operator=(AcceptLanguageElement rhs);

	

	/**

	 * Comparison based on the quality values ONLY, that is, the language tags of the elements are NOT 

	 * taken into account.

	 */

	Boolean operator>(AcceptLanguageElement rhs);

	

	/**

	 * Comparison based on the quality values ONLY, that is, the language tags of the elements are NOT 

	 * taken into account.

	 */

	Boolean operator<(AcceptLanguageElement rhs);

	

	/**

	 * True if the langauge tag portion equals that of rhs (case INSENSITIVE) AND

	 * if the quality values are equal 

	 * @param rhs AcceptLanguageElement

	 */

	Boolean operator==(AcceptLanguageElement rhs);

	

	/**

	 * True if the langauge tag portion does NOT equal that of rhs (case INSENSITIVE) OR

	 * if the quality values are NOT equal

	 * @param rhs AcceptLanguageElement 

	 */

	Boolean operator!=(AcceptLanguageElement rhs);



	/**

	 * Writes the string representation of this object to the stream

	 */

	PEGASUS_COMMON_LINKAGE friend PEGASUS_STD(ostream) &operator<<(PEGASUS_STD(ostream) &stream, AcceptLanguageElement ale);



			

}; // end AcceptLanguageElement



#ifndef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T AcceptLanguageElement

#include <Pegasus/Common/ArrayInter.h>

#undef PEGASUS_ARRAY_T

#endif


PEGASUS_NAMESPACE_END


#endif  // PEGASUS_USE_EXPERIMENTAL_INTERFACES


#endif

