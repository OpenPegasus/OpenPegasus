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
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Formatter_h
#define Pegasus_Formatter_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Linkage.h>

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

PEGASUS_NAMESPACE_BEGIN

/**
    Formatter is a class to build formatted strings from
    strings that contain variable defintions.  The
    variable definitions in the strings are of the form
    $<int>
    
    where <int> is a single digit integer (0 - 9).
    
    The variable subsituted may be String, Boolean Integer, Unsigned Integer
    or  real.
    
    The format subsitution may be escaped by preceding the
    $ with a \
    
    usage:
    Formatter::format (FormatString, variable0,.., variable9)
    
    Example:
    <pre>
    int total = 4;
    int count = 2;
    String name = "Output"
    String output = Formatter::format(
			"total $0 average $1 on $2", 
			total,
			total/count,
			name);
    produces the string
	 
      "total 4 average 2 on Output"
    
    </pre>
*/
class PEGASUS_COMMON_LINKAGE Formatter
{
public:

    class Arg
    {
    public:

	enum Type { VOIDT, STRING, BOOLEAN, INTEGER, UINTEGER, LINTEGER, 
			ULINTEGER, REAL };

	Arg() : _type(VOIDT)
	{
	}

	Arg(const String& x) : _string(x), _type(STRING)
	{
	}

	Arg(const char* x) : _string(x), _type(STRING)
	{
	}

	Arg(Boolean x) : _boolean(x), _type(BOOLEAN)
	{
	}

	Arg(Sint32 x) : _integer(x), _type(INTEGER)
	{
	}

	Arg(Uint32 x) : _uinteger(x), _type(UINTEGER)
	{
	}

	Arg(Sint64 x) : _lInteger(x), _type(LINTEGER)
	{
	}

	Arg(Uint64 x) : _lUInteger(x), _type(ULINTEGER)
	{
	}
	Arg(Real64 x) : _real(x), _type(REAL)
	{
	}

	String toString() const;
	
	friend class MessageLoader;  //l10n

    private:

	String _string;

	union
	{
	    Sint32 _integer;
	    Uint32 _uinteger;
	    Real64 _real;
	    int _boolean;
	    Sint64 _lInteger;
	    Uint64 _lUInteger;
	};

	Type _type;
    };
    /**	 Format function for the formatter
    */
    static String format(
	const String& formatString,
	const Arg& arg0 = Arg(),
	const Arg& arg1 = Arg(),
	const Arg& arg2 = Arg(),
	const Arg& arg3 = Arg(),
	const Arg& arg4 = Arg(),
	const Arg& arg5 = Arg(),
	const Arg& arg6 = Arg(),
	const Arg& arg7 = Arg(),
	const Arg& arg8 = Arg(),
	const Arg& arg9 = Arg());
	
};

PEGASUS_NAMESPACE_END

#endif /* PEGASUS_USE_EXPERIMENTAL_INTERFACE */

#endif /* Pegasus_Formatter_h */
