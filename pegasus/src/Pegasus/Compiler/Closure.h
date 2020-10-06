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
//%/////////////////////////////////////////////////////////////////////////////

#ifndef _Pegasus_Repository_Closure_h
#define _Pegasus_Repository_Closure_h

#include <Pegasus/Common/CIMClass.h>

PEGASUS_NAMESPACE_BEGIN

/** This function computes the closure of the given class. The closure of a
    class includes all classes related in any of the following ways:

        1. Through reflexivity (the class itself).
        2. Through superclass.
        3. Through references.
        4. Through reference parameters.
        5. Through EmbeddedInstance qualifiers.
        6. Through recusive application to classes discovered in 1 to 5.

    The closure is contained in the #closure# parameter upon return.
    
    NOTE: Currently the closure concept is implemented only for
    the memory resident repository (MRR)
*/
int Closure(
    const CIMName& className,
    const Array<CIMClass>& classes,
    Array<CIMName>& closure);

PEGASUS_NAMESPACE_END

#endif /*_Pegasus_Repository_Closure_h */
