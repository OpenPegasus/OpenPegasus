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


#include <cstdlib>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/DeclContext.h>
#include <Pegasus/Common/Resolver.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;
int main(int, char** argv)
{
    try
    {
        const String NAMESPACE = "/zzz";

        // Create and populate a declaration context:

        SimpleDeclContext* context = new SimpleDeclContext;

        context->addQualifierDecl(NAMESPACE, CIMQualifierDecl(
            CIMName ("abstract"), false, CIMScope::CLASS,
                CIMFlavor::OVERRIDABLE));

        // Create some classes:

        CIMClass class1(CIMName ("PeskySuperClass"));
        class1.addQualifier(CIMQualifier(CIMName ("abstract"), true));

        CIMClass class2(CIMName ("Class"), CIMName ("PeskySuperClass"));

        Resolver::resolveClass (class1, context, NAMESPACE);
        context->addClass(NAMESPACE, class1);

        Resolver::resolveClass (class2, context, NAMESPACE);
        context->addClass(NAMESPACE, class2);

        // class1.print();
        // class2.print();

            // Create some methods:
            CIMMethod meth1(CIMName ("getHostName"), CIMTYPE_STRING);
            CIMConstMethod meth2(CIMName ("test"), CIMTYPE_STRING);
            Resolver::resolveMethod ( meth1, context, NAMESPACE, meth2);

        delete context;
    }
    catch (Exception& e)
    {
        cout << "Exception: " << e.getMessage() << endl;
        exit(1);
    }
    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
