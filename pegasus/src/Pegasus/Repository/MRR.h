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

#ifndef _Pegasus_Repository_MRR_h
#define _Pegasus_Repository_MRR_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Buffer.h>
#include "MRRTypes.h"
#include "Linkage.h"

PEGASUS_NAMESPACE_BEGIN

// Installs the initialize callback that is called when the repository is
// initially created (from the MemoryResidentRepository constructor).
PEGASUS_REPOSITORY_LINKAGE void MRRInstallInitializeCallback(
    void (*callback)(class CIMRepository* repository, void * data),
    void *data);

// Installs the global save callback that is called when the memory-resident
// instance repository is modified. The buffer argument is a serialized
// copy of the memory-resident instance repository. The callback can do
// things such as save the buffer on disk for later use.
PEGASUS_REPOSITORY_LINKAGE void MRRInstallSaveCallback(
    void (*callback)(const Buffer& buffer, void* data),
    void* data);

// Installs the global load callback that is called when an instance of
// MemoryResidentRepository is created in order to load the initial set
// of instances (if any).
PEGASUS_REPOSITORY_LINKAGE void MRRInstallLoadCallback(
    void (*callback)(Buffer& buffer, void* data),
    void* data);

// Add the given namespace of qualifier declarations and classes.
PEGASUS_REPOSITORY_LINKAGE Boolean MRRAddNameSpace(
    const MRRNameSpace* nameSpace);

PEGASUS_NAMESPACE_END

#endif /* _Pegasus_Repository_MRR_h */
