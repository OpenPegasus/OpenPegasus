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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusVersion.h>

#include <iostream>
#include <cstdio>
#include <Pegasus/Handler/CIMHandler.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Handler/IndicationFormatter.h>

#include "FileListenerDestination.h"

PEGASUS_NAMESPACE_BEGIN

void FileListenerDestination::initialize(CIMRepository* repository)
{
}

void FileListenerDestination::handleIndication(
        const OperationContext& context,
        const String nameSpace,
        CIMInstance& indication,
        CIMInstance& handler,
        CIMInstance& subscription,
        ContentLanguageList& contentLanguages)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
            "FileListenerDestination::handleIndication");

    String indicationText;

    try
    {
        PEG_TRACE ((TRC_INDICATION_GENERATION, Tracer::LEVEL4,
            "FileListenerDestination %s:%s.%s processing %s Indication",
                (const char*)(nameSpace.getCString()),
                (const char*)(handler.getClassName().getString().getCString()),
                (const char*)(handler.getProperty(
                    handler.findProperty(PEGASUS_PROPERTYNAME_LSTNRDST_FILE)).
                    getValue().toString().getCString()),
                (const char*)(indication.getClassName().getString().
                    getCString())));

        // gets formatted indication message
        indicationText = IndicationFormatter::getFormattedIndText(
            subscription, indication, contentLanguages);


        String filePath;
        handler.getProperty(handler.findProperty(
                    PEGASUS_PROPERTYNAME_LSTNRDST_FILE)).
            getValue().get(filePath);

        PEG_TRACE ((TRC_INDICATION_GENERATION, Tracer::LEVEL4,
            "FileListenerDestination writing %s Indication to file %s",
            (const char*)(indication.getClassName().getString().getCString()),
            (const char*)filePath.getCString()));

        // writes the indication to a file
        _recordIndicationToFile(
            (const char*)filePath.getCString(), indicationText);
    }
    catch(const Exception &e)
    {
        PEG_TRACE((TRC_IND_HANDLER, Tracer::LEVEL1, "Exception: %s",
                    (const char*)e.getMessage().getCString()));
        PEG_METHOD_EXIT();

        throw;
    }
    catch (...)
    {
        PEG_TRACE_CSTRING(TRC_IND_HANDLER, Tracer::LEVEL1,
                "Failed to write indication to local file.");
        PEG_METHOD_EXIT();
        throw; 
    }

    PEG_METHOD_EXIT();
}

void FileListenerDestination::_recordIndicationToFile(
        const char* path,
        const String& formattedText)
{
    PEG_METHOD_ENTER(TRC_IND_HANDLER,
            "FileListenerDestination::_recordIndicationToFile");

    FILE *fd = fopen(path, "a");
    if(fd)
    {
        if( fprintf(fd, "%s\n", (const char*)formattedText.getCString()) > 0)
        {
            fclose(fd);
        }
        else
        {
          fclose(fd);
          PEG_METHOD_EXIT();
          throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, MessageLoaderParms(
              "Handler.FileListenerDestination.FileListenerDestination."
              "FAILED_TO_WRITE_INDICATION_TO_FILE",
              "Failed to write indication to local file $0.", 
              path));
        }
    }
    else // file opening failed
    {
        PEG_METHOD_EXIT();
        ThrowCannotOpenFileException(path);
    }

    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END

PEGASUS_USING_PEGASUS;

// This is the entry point into this dynamic module.

extern "C" PEGASUS_EXPORT CIMHandler* PegasusCreateHandler(
        const String& handlerName)
{
    if (handlerName == "FileListenerDestination")
    {
        return new FileListenerDestination;
    }

    return 0;
}
