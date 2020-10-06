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


#ifndef FILE_LISTENER_DESTINATION_H_
#define FILE_LISTENER_DESTINATION_H_
#include <Pegasus/Common/Constants.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_HANDLER_LINKAGE FileListenerDestination: public CIMHandler
{
public:

    FileListenerDestination()
    {
    }

    virtual ~FileListenerDestination()
    {
    }

    void initialize(CIMRepository* repository);

    void terminate()
    {
    }

    void handleIndication(
        const OperationContext& context,
        const String nameSpace,
        CIMInstance& indication,
        CIMInstance& handler,
        CIMInstance& subscription,
        ContentLanguageList& contentLanguages);

private:

    /**
        Writes the indication to a local file.

        @param  path     path to the file where indication is written
        @param  formattedText  the formatted indication
    */

    void _recordIndicationToFile(
        const char* path,
        const String& formattedText);
};

PEGASUS_NAMESPACE_END

#endif //FILE_LISTENER_DESTINATION_H_
