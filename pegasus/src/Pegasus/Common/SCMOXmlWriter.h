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

#ifndef Pegasus_SCMOXmlWriter_h
#define Pegasus_SCMOXmlWriter_h

#include <iostream>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/XmlGenerator.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/SCMO.h>
#include <Pegasus/Common/SCMOInstance.h>
#include <Pegasus/Common/CIMDateTimeInline.h>

PEGASUS_NAMESPACE_BEGIN
typedef struct propertyFilterNodesArray_s
{
    SCMBClass_Main* classPtrMemBlock;
    Array<Uint32> nodes;
}propertyFilterNodesArray_t;

class PEGASUS_COMMON_LINKAGE SCMOXmlWriter : public XmlWriter
{
public:
    //This function is used to build the propertyFilter node array.
    //this node array can be build once per class and reused for all
    //instances.
    static void buildPropertyFilterNodesArray(
        Array<Uint32> & nodes,
        const SCMOClass * classPtr,
        const CIMPropertyList & propertyList);

    //This function is used to get the propertyFilter node array.
    static const Array<Uint32> & getFilteredNodesArray(
        Array<propertyFilterNodesArray_t> & propFilterNodesArrays,
        const SCMOInstance& scmoInstance,
        const CIMPropertyList & propertyList);

    static void appendValueSCMOInstanceElement(
        Buffer& out,
        const SCMOInstance& scmoInstance,
        bool filtered,
        const Array<Uint32> & nodes);

    static void appendValueSCMOInstanceElements(
        Buffer& out,
        const Array<SCMOInstance> & _scmoInstances,
        const CIMPropertyList & propertyList);

    // EXP_PULL_BEGIN
    static void appendValueSCMOInstanceWithPathElements(
        Buffer& out,
        const Array<SCMOInstance> & _scmoInstances,
        const CIMPropertyList & propertyList);
    //EXP_PULL_END

    static void appendInstanceNameElement(
        Buffer& out,
        const SCMOInstance& scmoInstance);

    static void appendInstanceElement(
        Buffer& out,
        const SCMOInstance& scmoInstance,
        bool filtered,
        const Array<Uint32> & nodes);

    static void appendQualifierElement(
        Buffer& out,
        const SCMBQualifier& theQualifier,
        const char* base);

    static void appendPropertyElement(
        Buffer& out,
        const SCMOInstance& scmoInstance,
        Uint32 pos);

    static void appendValueElement(
        Buffer& out,
        const SCMBValue & value,
        const char * base);

    static void appendValueReferenceElement(
        Buffer& out,
        const SCMOInstance& ref);

    static void appendClassOrInstancePathElement(
        Buffer& out,
        const SCMOInstance& ref);

    static void appendLocalInstancePathElement(
        Buffer& out,
        const SCMOInstance& instancePath);

    static void appendInstancePathElement(
        Buffer& out,
        const SCMOInstance& instancePath);

    static void appendValueObjectWithPathElement(
        Buffer& out,
        const Array<SCMOInstance> & objectWithPath,
        const CIMPropertyList& propertyList);

//EXP_PULL_BEGIN
    static void appendValueInstanceWithPathElement(
        Buffer& out,
        const SCMOInstance& namedInstance,
        bool filtered,
        const Array<Uint32> & nodes);
//EXP_PULL_END
    static void appendValueObjectWithPathElement(
        Buffer& out,
        const SCMOInstance& objectWithPath,
        bool filtered = false ,
        const Array<Uint32> & nodes = Array<Uint32> (0));

    static void appendObjectElement(
        Buffer& out,
        const SCMOInstance& object,
        bool filtered = false ,
        const Array<Uint32> & nodes = Array<Uint32> (0));

    static void appendClassElement(
        Buffer& out,
        const SCMOInstance& cimClass);

    static void appendLocalClassPathElement(
        Buffer& out,
        const SCMOInstance& classPath);

    static void appendClassPathElement(
        Buffer& out,
        const SCMOInstance& classPath);

    static void appendSCMBUnion(
        Buffer& out,
        const SCMBUnion & u,
        const CIMType & valueType,
        const char * base);

    static void appendSCMBUnionArray(
        Buffer& out,
        const SCMBUnion & u,
        const CIMType & valueType,
        Uint32 numElements,
        const char * base);

private:
    SCMOXmlWriter();

//------------------------------------------------------------------------------
//
// appendLocalNameSpacePathElement()
//
//     <!ELEMENT LOCALNAMESPACEPATH (NAMESPACE+)>
//
//------------------------------------------------------------------------------
    static void appendLocalNameSpacePathElement(
        Buffer& out,
        const char * nameSpace,
        Uint32 nameSpaceLength)
    {
        // add one byte for the closing \0
        nameSpaceLength++;
        out << STRLIT("<LOCALNAMESPACEPATH>\n");

        char fixed[64];
        char* nameSpaceCopy;
        if (nameSpaceLength > 64)
        {
            nameSpaceCopy=(char*)malloc(nameSpaceLength);
        }
        else
        {
            nameSpaceCopy = &(fixed[0]);
        }
        memcpy(nameSpaceCopy, nameSpace, nameSpaceLength);

#if !defined(PEGASUS_COMPILER_MSVC) && !defined(PEGASUS_OS_ZOS)
        char *last;
        for (const char* p = strtok_r(nameSpaceCopy, "/", &last); p;
            p = strtok_r(NULL, "/", &last))
#else
        for (const char* p = strtok(nameSpaceCopy, "/"); p;
            p = strtok(NULL, "/"))
#endif
        {
            out << STRLIT("<NAMESPACE NAME=\"") << p << STRLIT("\"/>\n");
        }
        if (nameSpaceLength > 64)
        {
            free(nameSpaceCopy);
        }
        out << STRLIT("</LOCALNAMESPACEPATH>\n");
    }

//------------------------------------------------------------------------------
//
// appendNameSpacePathElement()
//
//     <!ELEMENT NAMESPACEPATH (HOST,LOCALNAMESPACEPATH)>
//
//------------------------------------------------------------------------------

    static void appendNameSpacePathElement(
        Buffer& out,
        const char* host,
        Uint32 hostLength,
        const char * nameSpace,
        Uint32 nameSpaceLength)
    {
        out << STRLIT("<NAMESPACEPATH>\n""<HOST>");
        out.append(host, hostLength);
        out << STRLIT("</HOST>\n");
        appendLocalNameSpacePathElement(out, nameSpace, nameSpaceLength);
        out << STRLIT("</NAMESPACEPATH>\n");
    }

//------------------------------------------------------------------------------
// appendClassNameElement()
//     <!ELEMENT CLASSNAME EMPTY>
//     <!ATTLIST CLASSNAME
//              %CIMName;>
//------------------------------------------------------------------------------
    static void appendClassNameElement(
        Buffer& out,
        const char* className,
        Uint32 classNameLength)
    {
        out << STRLIT("<CLASSNAME NAME=\"");
        out.append(className, classNameLength);
        out << STRLIT("\"/>\n");
    }
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_SCMOXmlWriter_h */
