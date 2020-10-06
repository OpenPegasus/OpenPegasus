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

#ifndef Pegasus_ParserTestProvider_h
#define Pegasus_ParserTestProvider_h

#include <iostream>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>

using namespace std;

#define DEBUG(X)  cout << "ParserTestProvider:" <<  X << endl
// This is not reqd. if IBMKR provider is not being used.
#define DIRECTORY "/home/subodh/ProjectDir/BadFileNames"
// Till here
//
#define CLASS_NAME "SampleXML_ParserClass"
#define CLASS_1 "Sample_PClassOne"
#define CLASS_2 "Sample_PClassTwo"
#define CLASS_3 "Sample_PClassThree"

#define BADSTR_1 "\0\0\0\'\'\'\'\'0000<<<<<<<>"
#define BADSTR_2 "\0\0\0\'\'\'\'\'0000        "
#define BADSTR_3 "     \0\0\0\'\'\'\'\'0000\0\0\0\0\0"

PEGASUS_NAMESPACE_BEGIN

class ParserTestProvider :
    public CIMInstanceProvider
{
public:
    ParserTestProvider(void);
    virtual ~ParserTestProvider(void);
        virtual Boolean tryTerminate(void);

    // CIMProvider interface
    virtual void initialize(CIMOMHandle & cimom);
    virtual void terminate(void);

    // CIMInstanceProvider interface
    virtual void getInstance(
        const OperationContext & context,
        const CIMObjectPath & ref,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        InstanceResponseHandler & handler);

    virtual void enumerateInstances(
        const OperationContext & context,
        const CIMObjectPath & ref,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        InstanceResponseHandler & handler);

    virtual void enumerateInstanceNames(
        const OperationContext & context,
        const CIMObjectPath & ref,
        ObjectPathResponseHandler & handler);

    virtual void modifyInstance(
        const OperationContext & context,
        const CIMObjectPath & ref,
        const CIMInstance & obj,
        const Boolean includeQualifiers,
        const CIMPropertyList & propertyList,
        ResponseHandler & handler);

    virtual void createInstance(
        const OperationContext & context,
        const CIMObjectPath & ref,
        const CIMInstance & obj,
        ObjectPathResponseHandler & handler);

    virtual void deleteInstance(
        const OperationContext & context,
        const CIMObjectPath & ref,
        ResponseHandler & handler);

protected:
    Array<CIMInstance> _instances;
    Array<CIMObjectPath> _instanceNames;
};

PEGASUS_NAMESPACE_END

#endif /*Pegasus_ParserTestProvider_h */
