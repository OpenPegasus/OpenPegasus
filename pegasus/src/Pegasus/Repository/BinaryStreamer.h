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

#ifndef BinaryStreamer_h
#define BinaryStreamer_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMQualifier.h>
#include <Pegasus/Common/CIMMethod.h>
#include <Pegasus/Common/CIMFlavor.h>
#include <Pegasus/Common/CIMScope.h>

#include <Pegasus/Repository/Linkage.h>
#include <Pegasus/Repository/ObjectStreamer.h>

#define BINREP_MARKER 0x11

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_REPOSITORY_LINKAGE BinaryStreamer : public ObjectStreamer
{
public:

    BinaryStreamer() { }

    ~BinaryStreamer() { }

    PEGASUS_HIDDEN_LINKAGE
    void encode(Buffer& out, const CIMClass& x);

    PEGASUS_HIDDEN_LINKAGE
    void encode(Buffer& out, const CIMInstance& x);

    PEGASUS_HIDDEN_LINKAGE
    void encode(Buffer& out, const CIMQualifierDecl& x);

    PEGASUS_HIDDEN_LINKAGE
    void decode(const Buffer& in, unsigned int pos, CIMClass& x);

    PEGASUS_HIDDEN_LINKAGE
    void decode(const Buffer& in, unsigned int pos, CIMInstance& x);

    PEGASUS_HIDDEN_LINKAGE
    void decode(const Buffer& in, unsigned int pos, CIMQualifierDecl& x);

public:

    PEGASUS_HIDDEN_LINKAGE
    static void _unpackQualifier(
        const Buffer& in, Uint32& pos, CIMQualifier& x);

    PEGASUS_HIDDEN_LINKAGE
    static void _packQualifier(Buffer& out, const CIMQualifier& x);

    PEGASUS_HIDDEN_LINKAGE
    static void _unpackProperty(
        const Buffer& in, Uint32& pos, CIMProperty& x);

    PEGASUS_HIDDEN_LINKAGE
    static void _unpackMethod(
        const Buffer& in, Uint32& pos, CIMMethod& x);

private:

    PEGASUS_HIDDEN_LINKAGE
    static void _packName(Buffer& out, const CIMName& x);

    PEGASUS_HIDDEN_LINKAGE
    static void _unpackName(const Buffer& in, Uint32& pos, CIMName& x);

    PEGASUS_HIDDEN_LINKAGE
    static void _packValue(Buffer& out, const CIMValue& x);

    PEGASUS_HIDDEN_LINKAGE
    static void _unpackValue(const Buffer& in, Uint32& pos, CIMValue& x);

    PEGASUS_HIDDEN_LINKAGE
    static void _packProperty(Buffer& out, const CIMProperty& x);

    PEGASUS_HIDDEN_LINKAGE
    static void _packParameter(Buffer& out, const CIMParameter& x);

    PEGASUS_HIDDEN_LINKAGE
    static void _unpackParameter(
        const Buffer& in, Uint32& pos, CIMParameter& x);

    PEGASUS_HIDDEN_LINKAGE
    static void _packParameters(Buffer& out, CIMMethodRep* rep);

    PEGASUS_HIDDEN_LINKAGE
    static void _unpackParameters(
        const Buffer& in, Uint32& pos, CIMMethod& x);

    PEGASUS_HIDDEN_LINKAGE
    static void _packMethod(Buffer& out, const CIMMethod& x);

    PEGASUS_HIDDEN_LINKAGE
    static void _packObjectPath(Buffer& out, const CIMObjectPath& x);

    PEGASUS_HIDDEN_LINKAGE
    static void _unpackObjectPath(
        const Buffer& in, Uint32& pos, CIMObjectPath& x);

    PEGASUS_HIDDEN_LINKAGE
    static void _packProperties(Buffer& out, CIMObjectRep* rep);

    PEGASUS_HIDDEN_LINKAGE
    static void _packMethods(Buffer& out, CIMClassRep* rep);

    PEGASUS_HIDDEN_LINKAGE
    static void _packScope(Buffer& out, const CIMScope& x);

    PEGASUS_HIDDEN_LINKAGE
    static void _unpackScope(
        const Buffer& in, Uint32& pos, CIMScope& x);

    PEGASUS_HIDDEN_LINKAGE
    static void _packFlavor(Buffer& out, const CIMFlavor& x);

    PEGASUS_HIDDEN_LINKAGE
    static void _unpackFlavor(
        const Buffer& in, Uint32& pos, CIMFlavor& x);

    PEGASUS_HIDDEN_LINKAGE
    static void _packType(Buffer& out, const CIMType& x);

    PEGASUS_HIDDEN_LINKAGE
    static void _unpackType(
        const Buffer& in, Uint32& pos, CIMType& x);
};

class PEGASUS_REPOSITORY_LINKAGE BinException : public Exception
{
public:
    BinException(const String& message) : Exception(message) { }
};

PEGASUS_NAMESPACE_END

#endif
