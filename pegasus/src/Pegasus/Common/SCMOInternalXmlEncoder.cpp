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
// This code implements part of PEP#348 - The CMPI infrastructure using SCMO
// (Single Chunk Memory Objects).
// The design document can be found on the OpenPegasus website openpegasus.org
// at https://collaboration.opengroup.org/pegasus/pp/documents/21210/PEP_348.pdf
//
//%/////////////////////////////////////////////////////////////////////////////

#include "SCMOInternalXmlEncoder.h"
#include <Pegasus/Common/SCMOXmlWriter.h>

PEGASUS_NAMESPACE_BEGIN

void SCMOInternalXmlEncoder::_putXMLInstance(
    CIMBuffer& out,
    const SCMOInstance& ci,
    const CIMPropertyList& propertyList )
{

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
        if (propertyList.isNull())
        {
            Array<Uint32> emptyNodes;
            SCMOXmlWriter::appendInstanceElement(
                buf,
                ci,
                false,
                emptyNodes);
        }
        else
        {
            Array<propertyFilterNodesArray_t> propFilterNodesArrays;
            // This searches for an already created array of nodes,
            // if not found, creates it inside propFilterNodesArrays
            const Array<Uint32> & nodes=
                SCMOXmlWriter::getFilteredNodesArray(
                    propFilterNodesArrays,
                    ci,
                    propertyList);
            SCMOXmlWriter::appendInstanceElement(
                buf,
                ci,
                true,
                nodes);
        }
        buf.append('\0');

        out.putUint32(buf.size());
        out.putBytes(buf.getData(), buf.size());
        buf.clear();

        if (0 == ci.getClassName())
        {
            out.putUint32(0);
            out.putString(String());
            out.putNamespaceName(CIMNamespaceName());
        }
        else
        {
            // Serialize object path as XML.
            SCMOXmlWriter::appendValueReferenceElement(buf, ci);
            buf.append('\0');

            out.putUint32(buf.size());
            out.putBytes(buf.getData(), buf.size());

            // Write hostname and namespace in UTF-16 format
            Uint32 len=0;
            const char* hn = ci.getHostName_l(len);
            out.putUTF8AsString(hn, len);
            const char * ns = ci.getNameSpace_l(len);
            out.putUTF8AsString(ns, len);
        }
    }
}

void SCMOInternalXmlEncoder::_putXMLNamedInstance(
    CIMBuffer& out,
    const Array<SCMOInstance> & ci,
    const CIMPropertyList& propertyList)
{
    Array<propertyFilterNodesArray_t> propFilterNodesArrays;
    Array<Uint32> emptyNodes;
    for (Uint32 i = 0, n = ci.size(); i < n; i++)
    {
        if (ci[i].isUninitialized())
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
            if (propertyList.isNull())
            {
                SCMOXmlWriter::appendInstanceElement(
                    buf,
                    ci[i],
                    false,
                    emptyNodes);
            }
            else
            {
                // This searches for an already created array of nodes,
                // if not found, creates it inside propFilterNodesArrays
                const Array<Uint32> & nodes =
                    SCMOXmlWriter::getFilteredNodesArray(
                        propFilterNodesArrays,
                        ci[i],
                        propertyList);
                SCMOXmlWriter::appendInstanceElement(
                    buf,
                    ci[i],
                    true,
                    nodes);
            }
            buf.append('\0');
            out.putUint32(buf.size());
            out.putBytes(buf.getData(), buf.size());
            buf.clear();

            if (0 == ci[i].getClassName())
            {
                out.putUint32(0);
                out.putString(String());
                out.putNamespaceName(CIMNamespaceName());
            }
            else
            {
                // Serialize object path as XML.
                SCMOXmlWriter::appendInstanceNameElement(buf, ci[i]);
                buf.append('\0');

                out.putUint32(buf.size());
                out.putBytes(buf.getData(), buf.size());

                // Write hostname and namespace in UTF-16 format
                Uint32 len=0;
                const char* hn = ci[i].getHostName_l(len);
                out.putUTF8AsString(hn, len);
                const char * ns = ci[i].getNameSpace_l(len);
                out.putUTF8AsString(ns, len);
            }
        }
    }
}

void SCMOInternalXmlEncoder::_putXMLObject(
    CIMBuffer& out,
    const Array<SCMOInstance> & co,
    const CIMPropertyList& propertyList)
{
    Array<propertyFilterNodesArray_t> propFilterNodesArrays;
    Array<Uint32> emptyNodes;
    for (Uint32 i = 0, n = co.size(); i < n; i++)
    {
        if (co[i].isUninitialized())
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
            if (propertyList.isNull())
            {
                SCMOXmlWriter::appendObjectElement(
                    buf,
                    co[i],
                    false,
                    emptyNodes);
            }
            else
            {
                // This searches for an already created array of nodes,
                // if not found, creates it inside propFilterNodesArrays
                const Array<Uint32> & nodes =
                    SCMOXmlWriter::getFilteredNodesArray(
                        propFilterNodesArrays,
                        co[i],
                        propertyList);
                SCMOXmlWriter::appendObjectElement(
                    buf,
                    co[i],
                    true,
                    nodes);
            }

            buf.append('\0');
            out.putUint32(buf.size());
            out.putBytes(buf.getData(), buf.size());
            buf.clear();

            if (0 == co[i].getClassName())
            {
                out.putUint32(0);
                out.putString(String());
                out.putNamespaceName(CIMNamespaceName());
            }
            else
            {
                // Serialize object path as XML.
                _appendValueReferenceElement(buf, co[i]);
                buf.append('\0');

                out.putUint32(buf.size());
                out.putBytes(buf.getData(), buf.size());

                // Write hostname and namespace in UTF-16 format
                Uint32 len=0;
                const char* hn = co[i].getHostName_l(len);
                out.putUTF8AsString(hn, len);
                const char * ns = co[i].getNameSpace_l(len);
                out.putUTF8AsString(ns, len);
            }
        }
    }
}

void SCMOInternalXmlEncoder::_appendValueReferenceElement(
    Buffer& out,
    const SCMOInstance& ref)
{
    out << STRLIT("<VALUE.REFERENCE>\n");

    SCMOXmlWriter::appendInstanceNameElement(out, ref);

    out << STRLIT("</VALUE.REFERENCE>\n");
}

PEGASUS_NAMESPACE_END
