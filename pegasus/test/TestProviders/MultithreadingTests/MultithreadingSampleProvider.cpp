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

#include <stdio.h>
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "MultithreadingSampleProvider.h"
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/InternalException.h>

#define NO_OF_INSTANCES 5

PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_STD;

void writer (const char* message, int count, FILE* stream)
{
        for (; count > 0; --count) {
        // Write the message to the stream, and send it off immediately.
            fprintf (stream,"%s\n" , message);
            fflush (stream);
        // Sleeping for a long time in the writer thread so as to get a timeout
        // when CLI is executed from inside the reader thread
        sleep (100);
        }
}

void reader (FILE* stream)
{
    char buffer[1024];
    // Read until we hit the end of the stream. fgets reads until either
    // a newline or the end-of-file.
    while (!feof(stream)
        && !ferror(stream)
        && fgets(buffer, sizeof(buffer), stream) != NULL)
        fputs (buffer, stdout);
}

int CreateThreads()
{
    char* arg_list[] = {
            "CLI",
            "niall",
            "-n",
            "root/SampleProvider",
            NULL
    };
    int fds[2];
    pid_t cpid;

    pipe(fds);

    cpid = fork();
    if (cpid == (pid_t)0)
    {
        FILE* stream;
        // This is the child process. Close our copy of the write end
        // of the file descriptor.
        close(fds[1]);
        // Connect the read end of the pipe to standard input.
        dup2(fds[0], STDIN_FILENO);
        // Convert the read file descriptor to a FILE object, and read from it
        stream = fdopen(fds[0],"r" );
        reader(stream);
        close(fds[0]);
        // Replace the child process with the CLI program
        execvp ("CLI", arg_list);
    }
    else
    {
        FILE* stream;
        // Close our copy of the read end of the file descriptor.
        close (fds[0]);
        // Convert the write file descriptor to a FILE object,and write to it.
        stream = fdopen(fds[1],"w" );

        writer("Test Writer from Parent process\n", 2, stream);
        close (fds[1]);
        // Wait for child process to finish
        waitpid(cpid, NULL, 0);
    }
    return 0;
}

MultithreadingSampleProvider::MultithreadingSampleProvider(void)
{
}

MultithreadingSampleProvider::~MultithreadingSampleProvider(void)
{
}

void MultithreadingSampleProvider::initialize(CIMOMHandle & cimom)
{
    cout <<" MultithreadingSampleProvider::initialize" << endl;
}

void MultithreadingSampleProvider::terminate(void)
{
    cout <<" MultithreadingSampleProvider::terminate" << endl;
}

void MultithreadingSampleProvider::getInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        String(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());

    CIMName className = instanceReference.getClassName();
    cout << "className (Method: getInstance) " << className.getString() << endl;
    CIMNamespaceName nameSpace = instanceReference.getNameSpace();

    cout << "[className: " << className.getString()
         << "], [in namespace: " << nameSpace.getString()
         << "] " << endl;
    // instance index corresponds to reference index
    for(Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
        if(localReference == _instanceNames[i])
            {
            // deliver requested instance
            handler.deliver(_instances[i]);
            break;
            }
        }
    // complete processing the request
    handler.complete();
}

void MultithreadingSampleProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
    cout << "MultithreadingSampleProvider::enumerateInstancesNames" << endl;
    // begin processing the request
    handler.processing();

    for(Uint32 i = 0, n = _instances.size(); i < n; i++)
    // deliver instance
        handler.deliver(_instanceNames[i]);
    // complete processing the request
    handler.complete();
}

void MultithreadingSampleProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    int retval;
    char buffer[NO_OF_INSTANCES];
    CIMInstance Instances[NO_OF_INSTANCES];
    CIMObjectPath References[NO_OF_INSTANCES];

    cout << "MultithreadingSampleProvider::enumerateInstances" << endl;
    cout << "_instances.size = " << _instances.size() << endl;


    // begin processing the request
    handler.processing();

    CIMName clName = classReference.getClassName();
    String className = clName.getString();
    cout << "className = " << className<< endl;

    for (Uint32 i = 0; i < NO_OF_INSTANCES; i++)
    {
        sprintf(buffer, "%d", i);
        Instances[i] = CIMInstance("ThreadSampleOne");
        References[i] = CIMObjectPath("ThreadSampleOne.Id="+String(buffer));

        Instances[i].addProperty(CIMProperty("Id", Uint16(i)));
        Instances[i].addProperty(CIMProperty("Message", String(buffer)));


    _instances.append(Instances[i]);
    _instanceNames.append(References[i]);

    }
    retval = CreateThreads();
    cout << "Return from CreateThreads" << retval << endl;

    for(Uint32 i = 0, n = NO_OF_INSTANCES; i < n; i++)
    // deliver reference
        handler.deliver(_instances[i]);

    // complete processing the request
    handler.complete();
}

void MultithreadingSampleProvider::modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList & propertyList,
    ResponseHandler & handler)
{
    cout << "MultithreadingSampleProvider::modifyInstance" << endl;
    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        String(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());

    // begin processing the request
    handler.processing();

    // instance index corresponds to reference index
    for(Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
        if(localReference == _instanceNames[i])
        {
            // overwrite existing instance
            _instances[i] = instanceObject;

            break;
        }
    }
    // complete processing the request
    handler.complete();
}

void MultithreadingSampleProvider::createInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    ObjectPathResponseHandler & handler)
{
    cout << "MultithreadingSampleProvider::createInstance" << endl;
    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        String(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());

    // instance index corresponds to reference index
    for(Uint32 i = 0, n = _instanceNames.size(); i < n; i++)
    {
        if(localReference == _instanceNames[i])
        {
            throw CIMObjectAlreadyExistsException(
                                  localReference.toString());
        }
    }

    // begin processing the request
    handler.processing();

    // add the new instance to the array
    _instances.append(instanceObject);
    _instanceNames.append(instanceReference);

    // deliver the new instance
    handler.deliver(_instanceNames[_instanceNames.size() - 1]);

    // complete processing the request
    handler.complete();
}

void MultithreadingSampleProvider::deleteInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    ResponseHandler & handler)
{
    cout << "MultithreadingSampleProvider::deleteInstance" << endl;
    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        String(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());

    // begin processing the request
    handler.processing();

    // instance index corresponds to reference index
    for(Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
        if(localReference == _instanceNames[i])
        {
            // save the instance locally
            CIMInstance cimInstance(_instances[i]);

            // remove instance from the array
            _instances.remove(i);
            _instanceNames.remove(i);

            // exit loop
            break;
        }
    }

    // complete processing the request
    handler.complete();
}

PEGASUS_NAMESPACE_END
