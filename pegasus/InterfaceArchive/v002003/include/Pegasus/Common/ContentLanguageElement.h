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



#ifndef Pegasus_ContenetLanguageElement_h

#define Pegasus_ContenetLanguageElement_h



#include <Pegasus/Common/LanguageElement.h>


#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES


PEGASUS_NAMESPACE_BEGIN



//////////////////////////////////////////////////////////////

//

// ContentLanguageElement::

//

//////////////////////////////////////////////////////////////



class PEGASUS_COMMON_LINKAGE ContentLanguageElement : public LanguageElement{

	

public:



	/**	This member is used to represent an empty ContentLanguageElement. Using this 

        member avoids construction of an empty ContentLanguageElement

        (e.g., ContentLanguageElement()).

    */

	static const ContentLanguageElement EMPTY;

	

	/**

	 * This member is used to notify callers of ContentLanguages::itrNext() that the end of the container

	 * has been reached and subsequent calls to ContentLanguages::itrNext() will fail.

	 */

	static ContentLanguageElement EMPTY_REF;



	/**

	 * Default Constructor

	 */

	ContentLanguageElement():LanguageElement(){}



	/**

	 * Constructor

	 * @param language_tag String - language tag ex: en-US-mn

	 */

	ContentLanguageElement(String language_tag);

	

	/**

	 * Constructor

	 * @param language String - language ex: en

	 * @param country String - country code ex: US

	 * @param variant String - variant ex: rab-oof

	 */

	ContentLanguageElement(String language, String country, String variant);



	/**

	 * Copy Constructor

	 */

	ContentLanguageElement(const ContentLanguageElement &rhs);

	

	/**

	 * Destructor

	 */

	~ContentLanguageElement();



}; // end ContentLanguageElement



#ifndef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T ContentLanguageElement

#include <Pegasus/Common/ArrayInter.h>

#undef PEGASUS_ARRAY_T

#endif



PEGASUS_NAMESPACE_END


#endif  // PEGASUS_USE_EXPERIMENTAL_INTERFACES


#endif

