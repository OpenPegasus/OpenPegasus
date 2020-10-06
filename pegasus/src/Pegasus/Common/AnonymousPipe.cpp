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

#include <Pegasus/Common/CIMBinMsgSerializer.h>
#include <Pegasus/Common/CIMBinMsgDeserializer.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/Tracer.h>

#include "AnonymousPipe.h"


#if defined (PEGASUS_OS_TYPE_WINDOWS)
# include "AnonymousPipeWindows.cpp"
#elif defined (PEGASUS_OS_TYPE_UNIX)
# include "AnonymousPipePOSIX.cpp"
#elif defined (PEGASUS_OS_VMS)
# include "AnonymousPipePOSIX.cpp"
#else
# error "Unsupported platform"
#endif


PEGASUS_NAMESPACE_BEGIN

AnonymousPipe::Status AnonymousPipe::writeMessage (CIMMessage * message)
{
    PEG_METHOD_ENTER (TRC_OS_ABSTRACTION, "AnonymousPipe::writeMessage");

    //
    // Serialize the request
    //
    CIMBuffer messageBuffer(4096);

    try
    {
        CIMBinMsgSerializer::serialize(messageBuffer, message);
    }
    catch (Exception & e)
    {
        PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL2,
            "Failed to serialize message: %s",
            (const char*)e.getMessage().getCString()));
        PEG_METHOD_EXIT ();
        throw;
    }

    //
    // Write the serialized message to the pipe
    //
    Uint32 messageLength = messageBuffer.size();
    const char * messageData = messageBuffer.getData ();

    Status writeStatus =
        writeBuffer((const char*) &messageLength, sizeof(Uint32));

    if (writeStatus == STATUS_SUCCESS)
    {
        writeStatus = writeBuffer(messageData, messageLength);
    }

    PEG_METHOD_EXIT ();
    return writeStatus;
}

AnonymousPipe::Status AnonymousPipe::readMessage (CIMMessage * & message)
{
    PEG_METHOD_ENTER (TRC_OS_ABSTRACTION, "AnonymousPipe::readMessage");

    message = 0;

    //
    //  Read the message length
    //
    Uint32 messageLength;
    Status readStatus = readBuffer ((char *) &messageLength, sizeof (Uint32));

    if (readStatus != STATUS_SUCCESS)
    {
        PEG_METHOD_EXIT ();
        return readStatus;
    }

    if (messageLength == 0)
    {
        //
        //  Null message
        //
        PEG_METHOD_EXIT ();
        return STATUS_SUCCESS;
    }

    //
    //  Read the message data
    //
    // CIMBuffer uses realloc() and free() so the buffer must be allocated
    // with malloc().
    AutoPtr<char, FreeCharPtr> messageBuffer((char*)malloc(messageLength + 1));

    //
    //  We know a message is coming
    //  Keep reading even if interrupted
    //
    do
    {
        readStatus = readBuffer (messageBuffer.get (), messageLength);
    } while (readStatus == STATUS_INTERRUPT);

    if (readStatus != STATUS_SUCCESS)
    {
        PEG_METHOD_EXIT ();
        return readStatus;
    }

    try
    {
        //
        //  De-serialize the message
        //
        // CIMBuffer frees messageBuffer upon destruction.
        CIMBuffer buf(messageBuffer.release(), messageLength);
        message = CIMBinMsgDeserializer::deserialize(buf, messageLength);

        if (!message)
        {
            throw CIMException(CIM_ERR_FAILED, "deserialize() failed");
        }
    }
    catch (Exception & e)
    {
        //
        //  De-serialization failed
        //
        PEG_TRACE ((TRC_OS_ABSTRACTION, Tracer::LEVEL2,
            "Failed to de-serialize message: %s",
            (const char*)e.getMessage().getCString()));
        PEG_METHOD_EXIT ();
        throw;
    }

    PEG_METHOD_EXIT ();
    return readStatus;
}

PEGASUS_NAMESPACE_END
