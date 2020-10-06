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
//%////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_JSONWriter_h
#define Pegasus_JSONWriter_h

#include <Pegasus/RSServer/Linkage.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Buffer.h>
#include <Pegasus/Common/CIMNameCast.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/RSServer/RsURI.h>

PEGASUS_NAMESPACE_BEGIN


class PEGASUS_RSSERVER_LINKAGE JSONWriter
{
public:
    JSONWriter(Buffer& buf);

    ~JSONWriter();

    void append(CIMAssociatorsResponseMessage*,
                CIMRepository* repository,
                RsURI& requestUri);

    void append(CIMReferencesResponseMessage* referencesResult,
                CIMRepository* repository,
                RsURI& requestUri);

    void append(CIMInvokeMethodResponseMessage*,
                CIMRepository* repository,
                RsURI& requestUri);

    //void append(CIMEnumerateInstancesResponseMessage*);
    void append(CIMEnumerateInstancesResponseMessage*,
                Uint32 first, Uint32 last,
                CIMRepository* _repository,
                RsURI& requestUri);

    void append(CIMGetClassResponseMessage*);

    void append(CIMGetInstanceResponseMessage*,
                CIMRepository* repository,
                RsURI& requestUri);

    void append(CIMException&, String& httpMethod, RsURI& reqURI);
    void append(Exception&, String& httpMethod, RsURI& reqURI);

    Uint32 getEnumerationCount();


    void _append(const CIMConstQualifier&);

    void _append(Array<CIMObject>& objArray,
                 CIMRepository* repository,
                 RsURI& requestUri);

    void _append(const CIMConstClass& cimClass);
    void _append(const CIMConstInstance&, Boolean, Boolean,
                 CIMRepository* repository,
                 RsURI& requestUri,
                 Boolean includeMethods = true);
private:
    void _append(const CIMConstProperty&);
    void _append(const CIMValue&, CIMRepository* repository,
                 RsURI& requestUri);

    void _append(Boolean);
    void _append(Uint8);
    void _append(Uint16);
    void _append(Uint32);
    void _append(Uint64);
    void _append(Sint8);
    void _append(Sint16);
    void _append(Sint32);
    void _append(Sint64);
    void _append(Real32);
    void _append(Real64);
    void _append(const CIMDateTime& v);

    void _append(const CIMObjectPath& v);

    void _append(const String&, Boolean uriEncoded = false);


    Buffer& _buffer;
    Buffer** _props;
    Uint32 _propCount;
    CIMName _propClass;
    Uint32 _numObjectsEnumerated;
    void _append(Buffer&, const String&);

    void _loadPropertyNames(const CIMConstInstance&);
    void _deletePropertyNames();
    void _appendMethods(
        const CIMClass &cimClass,
        Buffer instanceUri,
        const CIMConstInstance& cimInstance,
        Boolean useAbsoluteUri);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_JSONWriter_h */
