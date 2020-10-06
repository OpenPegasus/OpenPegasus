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

#include "ProviderManager.h"
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Common/PegasusVersion.h>

PEGASUS_NAMESPACE_BEGIN

ProviderManager::ProviderManager()
{
}

ProviderManager::~ProviderManager()
{
}

String ProviderManager::_resolvePhysicalName(String physicalName)
{

#ifdef PEGASUS_ALLOW_ABSOLUTEPATH_IN_PROVIDERMODULE
    if ( System::is_absolute_path(physicalName.getCString()) )
    {
         return physicalName;
    }
#endif

    String fileName = FileSystem::buildLibraryFileName(physicalName);
    fileName = FileSystem::getAbsoluteFileName(
        ConfigManager::getHomedPath(
            ConfigManager::getInstance()->getCurrentValue("providerDir")),
        fileName);

    return fileName;
}

void ProviderManager::setIndicationCallback(
        PEGASUS_INDICATION_CALLBACK_T indicationCallback)
{
    _indicationCallback = indicationCallback;
}

void ProviderManager::setResponseChunkCallback(
        PEGASUS_RESPONSE_CHUNK_CALLBACK_T responseChunkCallback)
{
    _responseChunkCallback = responseChunkCallback;
}

void ProviderManager::setSubscriptionInitComplete
    (Boolean subscriptionInitComplete)
{
    _subscriptionInitComplete = subscriptionInitComplete;
}

PEGASUS_NAMESPACE_END
