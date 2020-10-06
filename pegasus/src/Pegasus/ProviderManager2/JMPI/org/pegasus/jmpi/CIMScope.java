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

//
// Author:      Adrian Schuur, schuur@de.ibm.com
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

package org.pegasus.jmpi;


/**
    Creates and instantiates a CIM scope, a qualifier attribute that
    indicates the CIM objects with which the qualifier can be used.
    For example, the qualifier 'ABSTRACT' has the following scope
    definition: Scope (Class Association Indication), meaning that
    it can only be used with classes, associations, and indications.
 */

public class CIMScope {

    private static final int BAD=0;
    public static final int SCHEMA=1;
    public static final int CLASS=2;
    public static final int ASSOCIATION=3;
    public static final int INDICATION=4;
    public static final int PROPERTY=5;
    public static final int REFERENCE=6;
    public static final int METHOD=7;
    public static final int PARAMETER=8;
    public static final int INSTANCE=9;
    public static final int ANY=10;

    private int scope=0;

    public CIMScope(int sc) {
	    scope=sc;
    }

    public static CIMScope getScope(int sc) {
        if (sc>BAD && sc<=ANY) return new CIMScope(sc);
	return(null);
    }

    public int getScope() {
	return(scope);
    }

}
