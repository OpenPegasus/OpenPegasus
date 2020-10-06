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

#ifndef Pegasus_AnonymousPipe_h
#define Pegasus_AnonymousPipe_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMMessage.h>


PEGASUS_NAMESPACE_BEGIN

/**
    The AnonymousPipe class implements an anonymous pipe.

    @author  Hewlett-Packard Company

*/
class PEGASUS_COMMON_LINKAGE AnonymousPipe
{
public:
    /**
        Constructs an AnonymousPipe instance.

        @exception   Exception (Failed to create pipe)
    */
    AnonymousPipe ();

    /**
        Constructs an AnonymousPipe instance, given the read and/or write handle
        in char form.

        NOTE: before using this form of the constructor, the pipe must already
        exist (a previous invocation of the AnonymousPipe () form of the
        constructor), and the specified handle(s) should be open for the
        specified operation (read or write).  The read or write handle
        should be obtained via a call to exportReadHandle () or
        exportWriteHandle (), respectively.

        @param   readHandle       char [] representation of the read handle to
                                    the pipe
        @param   writeHandle      char [] representation of the write handle to
                                    the pipe

        @exception   Exception (Failed to create pipe)
    */
    AnonymousPipe (
        const char * readHandle,
        const char * writeHandle);

    /**
        Destructs the AnonymousPipe instance.

        Closes the open pipe handles.
    */
    ~AnonymousPipe ();

    /**
        Defines symbolic constants for return values from read and write
        methods.
    */
    enum Status {STATUS_INTERRUPT = -2,
                 STATUS_ERROR     = -1,
                 STATUS_CLOSED    =  0,
                 STATUS_SUCCESS   =  1};

    /**
        Writes data from a buffer to the AnonymousPipe.

        @param   buffer           pointer to the input data buffer
        @param   bytesToWrite     Number of bytes to write

        @return  STATUS_SUCCESS   on success;
                 STATUS_CLOSED    on closed connection;
                 STATUS_ERROR     on error;
    */
    Status writeBuffer (
        const void * buffer,
        Uint32 bytesToWrite);

    /**
        Writes a CIM message to the AnonymousPipe.

        The message is serialized, then written to the pipe.

        @param   message          pointer to the message

        @return  STATUS_SUCCESS   on success;
                 STATUS_CLOSED    on closed connection;
                 STATUS_ERROR     on error;
    */
    Status writeMessage (
        CIMMessage * message);

    /**
        Reads data into a buffer from the AnonymousPipe.

        @param   buffer           pointer to the output data buffer
        @param   bytesToRead      Number of bytes to read

        @return  STATUS_SUCCESS   on success;
                 STATUS_CLOSED    on closed connection;
                 STATUS_ERROR     on error;
                 STATUS_INTERRUPT on interrupt
    */
    Status readBuffer (
        void * buffer,
        Uint32 bytesToRead);

    /**
        Reads a CIM message from the AnonymousPipe.

        A message is read from the pipe, then de-serialized.

        @param   message          pointer to the message (output parameter)

        @return  STATUS_SUCCESS   on success;
                 STATUS_CLOSED    on closed connection;
                 STATUS_ERROR     on error;
                 STATUS_INTERRUPT on interrupt
    */
    Status readMessage (
    CIMMessage * & message);

    /**
        Gets a char [] form of the pipe handle for reading from the
        AnonymousPipe instance.

        NOTE: the caller must supply the buffer.  The buffer size must be at
              least 32.
    */
    void exportReadHandle (
        char * buffer) const;

    /**
        Gets a char [] form of the pipe handle for writing to the
        AnonymousPipe instance.

        NOTE: the caller must supply the buffer.  The buffer size must be at
              least 32.
    */
    void exportWriteHandle (
        char * buffer) const;

    /**
        Closes the pipe handle for reading from the AnonymousPipe instance.
    */
    void closeReadHandle ();

    /**
        Closes the pipe handle for writing to the AnonymousPipe instance.
    */
    void closeWriteHandle ();

private:

    /**
        Private, unimplemented copy constructor to avoid implicit use of
        the default copy constructor
    */
    AnonymousPipe (const AnonymousPipe & anonymousPipe);

    /**
        Private, unimplemented assignment operator to avoid implicit use of
        the default assignment operator
    */
    AnonymousPipe & operator= (const AnonymousPipe & anonymousPipe);

#if defined (PEGASUS_OS_TYPE_WINDOWS)
    typedef HANDLE AnonymousPipeHandle;
#else
    typedef int AnonymousPipeHandle;
#endif

    /**
        Stores the read pipe handle.
    */
    AnonymousPipeHandle _readHandle;

    /**
        Stores the write pipe handle.
    */
    AnonymousPipeHandle _writeHandle;

    /**
        Indicates whether the read handle is open.
    */
    Boolean _readOpen;

    /**
        Indicates whether the write handle is open.
    */
    Boolean _writeOpen;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_AnonymousPipe_h */
