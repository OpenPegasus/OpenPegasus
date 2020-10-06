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
#ifndef _CMPIMsgHandleManager_H_
#define _CMPIMsgHandleManager_H_

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ReadWriteSem.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/ProviderManager2/CMPI/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

//
// This class is used to store and retrieve unique message file handles
// and map them to a pointer to MessageLoaderParms.
//
// To use this class first obtain the singleton instance using the
// getCMPIMsgHandleManager method.
//
// Note: The handle manager does not own the memory that is associated to
//       the handle through the pointer. It is the callers responsibility
//       to allocate and release the data behind the pointers.
//
class CMPIMsgHandleManager
{

public:

    ~CMPIMsgHandleManager();

    //
    // Retrieves the singleton instance of the handle manager.
    //
    static CMPIMsgHandleManager* getCMPIMsgHandleManager(void);

    //
    // Obtains a new unique handle and associates it with the given
    // data pointer.
    //
    CMPIMsgFileHandle getNewHandle(MessageLoaderParms* data);

    //
    // Returns the data pointer associated to handle.
    // Throws IndexOutOfBoundsException for unused or invalid handles.
    //
    MessageLoaderParms* getDataForHandle(CMPIMsgFileHandle handle);

    //
    // Releases the given handle, marking it as unused.
    // Throws IndexOutOfBoundsException for unused or invalid handles.
    // Note: This method does not release any storage. It is the callers
    //       responsibility to release the memory behind the data pointer
    //       returned by this method.
    //
    MessageLoaderParms* releaseHandle(CMPIMsgFileHandle handle);

private:

    CMPIMsgHandleManager();

    static AutoPtr<CMPIMsgHandleManager> _handleManagerInstance;

    // auto-initialisation due to being on the stack
    Array<MessageLoaderParms*> handleTable;

    // Semaphore to protect the handle array from concurrent updates and
    // to serialize the creation of the singleton instance
    static ReadWriteSem _rwsemHandleTable;

};

PEGASUS_NAMESPACE_END

#endif
