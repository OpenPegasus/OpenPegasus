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

#ifndef _SCMODUMP_H_
#define _SCMODUMP_H_
# ifdef PEGASUS_DEBUG

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/SCMO.h>
#include <Pegasus/Common/SCMOInstance.h>
#include <Pegasus/Common/SCMOClass.h>


PEGASUS_NAMESPACE_BEGIN


// The definiton of a dump class for SCMO
class PEGASUS_COMMON_LINKAGE SCMODump
{

public:

    SCMODump();
    ~SCMODump();
    SCMODump(const char *filename);
    // Methods for SCMOClass only
    void hexDumpSCMOClass(SCMOClass& testCls) const;
    void dumpSCMOClass(SCMOClass& testCls,Boolean inclMemHdr = true) const;
    void dumpSCMOClassQualifiers(SCMOClass& testCls) const;
    void dumpKeyPropertyMask(SCMOClass& testCls) const;
    void dumpClassProperties(SCMOClass& testCls) const;
    void dumpKeyIndexList(SCMOClass& testCls) const;
    void dumpClassPropertyNodeArray(SCMOClass& testCls) const;
    void dumpKeyBindingSet(SCMOClass& testCls) const;
    void dumpClassKeyBindingNodeArray(SCMOClass& testCls) const;

    // Methods for SCMOInstance only
    void dumpSCMOInstance(
        SCMOInstance& testInst,
        Boolean inclMemHdr = true,
        Boolean verbose = false) const;
    void dumpSCMOInstanceKeyBindings(
        SCMOInstance& testInst,
        Boolean verbose = false) const ;
    void dumpSCMOInstancePropertyFilter(SCMOInstance& testInst) const ;
    void dumpPropertyFilter(SCMOInstance& testInst) const;
    void dumpPropertyFilterIndexMap(SCMOInstance& testInst) const;
    void dumpInstanceProperties(
        SCMOInstance& testInst,
        Boolean verbose = false) const;


    // Methods for SCMOClass and SCMOInstance
    void dumpHashTable(Uint32* hashTable,Uint32 size)const;

    // Methods use files for dumping.
    void openFile(const char *filename);
    void closeFile();
    String getFileName()
    {
        return _filename;
    }

    Boolean compareFile(String master);
    void deleteFile();

    void printSCMOValue(
        const SCMBValue& theValue,
        char* base,
        Boolean verbose = false) const;

    void printArrayValue(
        CIMType type,
        Uint32 size,
        SCMBUnion u,
        char* base,
        Boolean verbose = false) const;

    void printUnionValue(
        CIMType type,
        SCMBUnion u,
        char* base,
        Boolean verbose = false) const;

private:

    // context sensitive methods
    void _dumpQualifierArray(
        Uint64 start,
        Uint32 size,
        char* clsbase) const;

    void _dumpQualifier(
        const SCMBQualifier& theQualifier,
        char* clsbase) const;

    void _dumpClassProperty(
        const SCMBClassProperty& prop,
        char* clsbase) const;

    void _dumpSCMBMgmt_Header(SCMBMgmt_Header& header,char* base) const;

    void _hexDump(char* buffer,Uint64 length) const;

    void _dumpEmbeddedInstance(SCMBUnion u, Boolean verbose) const;

    void _dumpUserDefinedPropertyElement(char* instbase,
        SCMBUserPropertyElement* theElement,
        Boolean verbose) const;

    Boolean _fileOpen;
    FILE *_out;
    String _filename;
};

PEGASUS_NAMESPACE_END

# endif // PEGASUS_DEBUG
#endif
