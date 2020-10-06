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

#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/DeclContext.h>
#include <Pegasus/Common/Resolver.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

int main(int, char** argv)
{

    try
    {
    const String NAMESPACE = "/ggg";

    SimpleDeclContext context;

    CIMClass x(CIMName ("X"));
    x.addProperty(CIMProperty(CIMName ("one"), Uint32(111)));
    x.addProperty(CIMProperty(CIMName ("two"), Real32(222.222)));
    x.addProperty(CIMProperty(CIMName ("three"), String("Three")));
    context.addClass(NAMESPACE, x);
    Resolver::resolveClass (x, &context, NAMESPACE);

    CIMClass y(CIMName ("Y"), CIMName ("X"));
    y.addProperty(CIMProperty(CIMName ("three"), String("Three - Three")));
    y.addProperty(CIMProperty(CIMName ("four"), Boolean(false)));
    y.addProperty(CIMProperty(CIMName ("five"), Real32(555.555)));
    context.addClass(NAMESPACE, y);
    Resolver::resolveClass (y, &context, NAMESPACE);

    // y.print();
    }
    catch (Exception& e)
    {
    cout << "Exception: " << e.getMessage() << endl;
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
