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


#include "CIMInternalXmlEncoder.h"
#include "XmlWriter.h"
#include "Tracer.h"

/*
    FUTURE. The functions _putXMLInstance and _putXMLNamedInstance are the
    same except for the wrapper around the object path.  First, in at
    least some cases, the xml output removes this (CIM CIMResponseData and
    second we could simplify by making one function with a flag to
    add the wrapper. However. Should coordinate functions names, etc. with
    SCMO. Finally, there is a XmlWriter::appendValueReferenceElement which
    does the same as the internal function
    KS 28 March 2014.  I am not touching this until we sort out all the
    paths and get pull running correctly.  This goes through to many
    paths adding and removing the value.reference entity. KS April 2014
*/


PEGASUS_NAMESPACE_BEGIN

void CIMInternalXmlEncoder::_putXMLInstance(
    CIMBuffer& out,
    const CIMInstance& ci,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT, "_putXMLInstance");

    if (ci.isUninitialized())
    {
        out.putUint32(0);
        out.putUint32(0);
        out.putString(String());
        out.putNamespaceName(CIMNamespaceName());
    }
    else
    {
        Buffer buf(4096);

        // Serialize instance as XML.
        {
            XmlWriter::appendInstanceElement(
                buf,
                ci,
                includeQualifiers,
                includeClassOrigin,
                propertyList);
            buf.append('\0');

            out.putUint32(buf.size());
            out.putBytes(buf.getData(), buf.size());
            buf.clear();
        }

        // Serialize object path as XML.

        const CIMObjectPath& cop = ci.getPath();

        if (cop.getClassName().isNull())
        {
            out.putUint32(0);
            out.putString(String());
            out.putNamespaceName(CIMNamespaceName());
        }
        else
        {
            // add ValueReferenceElement
            XmlWriter::appendValueReferenceElement(buf, cop, false);
            buf.append('\0');

            out.putUint32(buf.size());
            out.putBytes(buf.getData(), buf.size());
            out.putString(cop.getHost());
            out.putNamespaceName(cop.getNameSpace());
        }
    }
    PEG_METHOD_EXIT();
}
// Same as _putXMLInstance except this function maps the objectPath directly
// appendInstanceNameElement whereas _putXMLInstance maps it to
// appendValueReferenceElement
void CIMInternalXmlEncoder::_putXMLNamedInstance(
    CIMBuffer& out,
    const CIMInstance& ci,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT, "_putXMLNamedInstance");

    if (ci.isUninitialized())
    {
        out.putUint32(0);
        out.putUint32(0);
        out.putString(String());
        out.putNamespaceName(CIMNamespaceName());
    }
    else
    {
        Buffer buf(4096);

        // Serialize instance as XML.
        {
            XmlWriter::appendInstanceElement(
                buf,
                ci,
                includeQualifiers,
                includeClassOrigin,
                propertyList);
            buf.append('\0');

            out.putUint32(buf.size());
            out.putBytes(buf.getData(), buf.size());
            buf.clear();
        }

        // Serialize object path as XML.

        const CIMObjectPath& cop = ci.getPath();

        if (cop.getClassName().isNull())
        {
            out.putUint32(0);
            out.putString(String());
            out.putNamespaceName(CIMNamespaceName());
        }
        else
        {
            XmlWriter::appendInstanceNameElement(buf, cop);
            buf.append('\0');

            out.putUint32(buf.size());
            out.putBytes(buf.getData(), buf.size());
            out.putString(cop.getHost());
            out.putNamespaceName(cop.getNameSpace());
        }
    }
    PEG_METHOD_EXIT();
}

void CIMInternalXmlEncoder::_putXMLObject(
    CIMBuffer& out,
    const CIMObject& co,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_PROVIDERAGENT, "_putXMLObject");

    if (co.isUninitialized())
    {
        out.putUint32(0);
        out.putUint32(0);
        out.putString(String());
        out.putNamespaceName(CIMNamespaceName());
    }
    else
    {
        Buffer buf(4096);

        // Serialize instance as XML.
        {
            XmlWriter::appendObjectElement(
                buf,
                co,
                includeQualifiers,
                includeClassOrigin,
                propertyList);
            buf.append('\0');

            out.putUint32(buf.size());
            out.putBytes(buf.getData(), buf.size());
            buf.clear();
        }

        // Serialize object path as XML.

        const CIMObjectPath& cop = co.getPath();

        if (cop.getClassName().isNull())
        {
            out.putUint32(0);
            out.putString(String());
            out.putNamespaceName(CIMNamespaceName());
        }
        else
        {
            _appendValueReferenceElement(buf, cop);
            buf.append('\0');

            out.putUint32(buf.size());
            out.putBytes(buf.getData(), buf.size());
            out.putString(cop.getHost());
            out.putNamespaceName(cop.getNameSpace());
        }
    }
    PEG_METHOD_EXIT();
}

// Calls appendInstanceName the other calls appendValueReference
// This is a shortcut function that puts VALUE.REFERENCE around
// InstanceNameElement directly.  It bypasses host and namespace components
// and assumes that it is an instance. Note mechanisms above to add
// host and namespace info different per function
void CIMInternalXmlEncoder::_appendValueReferenceElement(
    Buffer& out,
    const CIMObjectPath& reference)
{
    out << STRLIT("<VALUE.REFERENCE>\n");

    XmlWriter::appendInstanceNameElement(out, reference);

    out << STRLIT("</VALUE.REFERENCE>\n");
}

PEGASUS_NAMESPACE_END
