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
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By: Roger Kumpf, Hewlett Packard Company (roger_kumpf@hp.com)
//              Carol Ann Krug Graves, Hewlett-Packard Company
//                (carolann_graves@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Name_h
#define Pegasus_Name_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/Exception.h>

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// CIMName
//
////////////////////////////////////////////////////////////////////////////////

/**
    The CIMName class defines methods for handling CIM names.
    <p>
    The names of classes, properties, qualifiers, and methods are all
    CIM names. A CIM name must match the following regular
    expression:
    <PRE>
	[A-Z-a-z_][A-Za-z_0-9]*
    </PRE>
    <B>Examples:</B>
    <ul>
    <li>name - legal name
    <li>Type - legal name
    <li>3types - Illegal CIMName.
    </ul>
    The CIMName object includes the attribute Null which is required
    by the DMTF operations definitions.  Note that this and the regular
    expression limits on CIMName are what separate this from the String
    class. This allows the names in CIM operations such as getClass to 
    provide pattern matching tests for the classname parameter as well as
    separate the concept of empty from Null.
    
*/
class PEGASUS_COMMON_LINKAGE CIMName
{
public:

    /** Default constructor (sets isNull to true).
    */
    CIMName();

	/** Constructor creates a new CIMName object from
	    the String provided as input. The String must
	    be a legal name.
	    @param String defining the CIMName
	    @Exception InvalidNameException if the input String is 
	    not a legal CIMName
    */
    CIMName(const String& name);
    /**
	Constructor creates a new CIMName object from
			the String provided as input. The String must
			be a legal name.
			@param char* defining the CIMName text.
			@Exception InvalidNameException if the input String is 
			not a legal CIMName
	*/
    CIMName(const char* name);

    /**	Copy Constructor for CIMName object
	*/
    CIMName& operator=(const CIMName& name);
    /**	Copy constructor String object. Allows copying
	    String value into a CIMName.
	    @param String to be copied into CIMName
	    @exception InvalidNameException if the input String
	    is not a legal CIMName
	    <pre>
			CIMName n;
			String type = "type";
			n = type;
	    </pre>
	*/
    CIMName& operator=(const String& name);

#ifdef PEGASUS_USE_DEPRECATED_INTERFACES
    CIMName& operator=(const char* name);
#endif

    /** Extracts the String value of the CIMName
	    from the CIMName object.
	    @return String containing the name.
	    <pre>
			CIMName n("name");
			String s = n.getString();
	    </pre>
	*/
    const String& getString() const;

#ifdef PEGASUS_USE_DEPRECATED_INTERFACES
    operator String() const;
#endif

    /**	Tests the CIMName for NULL attribute.
	    @return true if Null or false if not Null.
	    <pre>
			CIMName n;
			assert(n.isNull());
			n = "name";
			assert(!n.isNull());
	    </pre>
	*/
    Boolean isNull() const;

    /**	Clears the CIMName and sets it to Null.
	    <pre>
		CIMMame n("name");
		n.clear();
		assert(n.isNull());
	    </pre>
	*/
    void clear();

    /** Compares the CIMName object against another CIMName object for equality.
	    @param CIMName to compare.
		@return true if the name passed is equal to the name in this
        class. CIM names are case insensitive and so is this method.
	    <pre>
	    CIMName n1 = "name";
	    CIMName n2 = "InstanceID";
	    if( n1.equal(n2) )
		    ...						// Should never get here
	    else
		    ...
	    </pre>
    */
    Boolean equal(const CIMName& name) const;

    /** Determines if the name string input is legal as
		defined in the CIMName class definition. This is a static
		method used to test String values to determine if they are
		legal names.
		@param name String to test for legality.
		@return true if the given name is legal, false otherwise.
	<pre>
	    assert(CIMName::legal("name"));
	    assert(!CIMName::legal("3types"));
	</pre>
    */
    static Boolean legal(const String& name);

private:
    String cimName;
};

PEGASUS_COMMON_LINKAGE Boolean operator==(
    const CIMName& name1,
    const CIMName& name2);

#define PEGASUS_ARRAY_T CIMName
# include "ArrayInter.h"
#undef PEGASUS_ARRAY_T


////////////////////////////////////////////////////////////////////////////////
//
// CIMNamespaceName
//
////////////////////////////////////////////////////////////////////////////////

/**
    The CIMNamespaceName class defines methods for handling CIM namespace names.
    <p>
    A CIM namespace name must match the following expression:
    <PRE>
        &lt;CIMName&gt;[ / &lt;CIMName&gt; ]*
    </PRE>
    </p>
    <B>Examples</B>
    <UL>
    <LI>root
    <LI>root/test
    </UL>
    NOTE: Pegasus uses namespaces starting with the top level name (ex. root).  It does
    not use the form /root/test with a leading slash.  The legal() test method in this class
    allows that form as a legal entity however.
*/
class PEGASUS_COMMON_LINKAGE CIMNamespaceName
{
public:

    /** Default constructor sets object Null. The Null state
	    indicates that there is no name assigned to this object.
	    The Null state can be tested with the isNull() method and
	    set with the clear() method.
	*/
    CIMNamespaceName();

    /** Constructor builds namespace from input String.
	    The String input must be a legal namespace name.
	    @param String from which the namespace object is built.
	    This must be a legal namespace name.
	    @exeception InvalidNamespaceName exception thrown if
	    the namespace name input is illegal.
	*/
    CIMNamespaceName(const String& name);

    /** Constructor builds namespace from input char*.
	    The String input must be a legal namespace name.
	    @param char* from which the namespace object is built.
	    This must be a legal namespace name.
	    @exeception InvalidNamespaceName exception thrown if
	    the namespace name input parameter is illegal.
	*/    
    CIMNamespaceName(const char* name);

    /** Assign one namespace object to another.
		@param CIMNamespaceName to assign to the object.
	*/
    CIMNamespaceName& operator=(const CIMNamespaceName& name);

    /** Assign a String object to a CIMNamespaceName object.
		@param CIMNamespaceName to assign
		@exeception InvalidNamespaceName exception thrown if
		the namespace name input parameter is illegal.
	    <pre>
			String s = "root/test";
			CIMNamespacename ns;
			ns = s;
	    </pre>
	*/
    CIMNamespaceName& operator=(const String& name);

#ifdef PEGASUS_USE_DEPRECATED_INTERFACES
    CIMNamespaceName& operator=(const char* name);
#endif

    /** Extracts the String value of the CIMNamespaceName
	    from the object.
	    @return String containing the name.
	    <pre>
			CIMNamespaceName ns("root/test");
			String s = ns.getString();
	    </pre>
	*/
    const String& getString() const;

#ifdef PEGASUS_USE_DEPRECATED_INTERFACES
    operator String() const;
#endif

    /**	Tests the CIMNamespaceName for NULL attribute. Returns
	    true if Null.  New objects without parameter and objects
	    set with clear() are Null.  When a name is set into the
	    object is is set to nonnull.  When the object is Null, it
	    returns empty string.
	    @return true if Null or false if not Null.
	    <pre>
			CIMName n;
			assert(n.isNull());
			n = "name";
			assert(!n.isNull());
	    </pre>
	*/
    Boolean isNull() const;

    /**	Clears the CIMNamespaceName and sets it to Null. A Null
	    object contains no name so that accessing it with getString
	    should return an empty String
	    <pre>
			CIMMamespaceName ns("root/test");
			ns.clear();
			assert(ns.isNull());
	    </pre>
	*/
    void clear();

    /** Compares two CIMNamespace objects for equality.
		@return true if the name passed is equal to the name in this
        class. CIM names are case insensitive and so is this method.
		<pre>
		CIMMamespaceName ns("root/test");
		CIMMamespaceName ns1("root/test");
		assert( ns.equal(ns1);
	    </pre>
    */
    Boolean equal(const CIMNamespaceName& name) const;

    /** Determines if the name string input is legal as
	defined in the CIMNamespaceName class definition.
	@param name String to test for legality.
	@return true if the given name is legal, false otherwise.
	<pre>
		assert(CIMNamespaceName::legal("root/test"));
	</pre>
    */
    static Boolean legal(const String& name);

private:
    String cimNamespaceName;
};

PEGASUS_COMMON_LINKAGE Boolean operator==(
    const CIMNamespaceName& name1,
    const CIMNamespaceName& name2);

#define PEGASUS_ARRAY_T CIMNamespaceName
# include "ArrayInter.h"
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Name_h */
