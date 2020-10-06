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
#include <string.h> //l10n

#include "Command.h"

PEGASUS_NAMESPACE_BEGIN

const Uint32 Command::RC_SUCCESS = 0;

const Uint32 Command::RC_ERROR = 1;

Command::~Command()
{

}

/**

    Gets command usage string.  Usage string may be retrieved and displayed
    when a CommandFormatException is caught.

    @return  String containing the command usage message

 */
String& Command::getUsage ()
{
    return _usage;
}

/**

    Sets command usage string.  Usage string should be set by Command
    subclass constructor.

    @param    usageMessage    the command usage message

 */
void Command::setUsage (String& usageMessage)
{
    _usage = usageMessage;
}

//l10n
String Command::localizeMessage(const char * resource_path,
                                const char * key,
                                const char * default_msg,
                                Formatter::Arg arg0,
                                Formatter::Arg arg1,
                                Formatter::Arg arg2,
                                Formatter::Arg arg3,
                                Formatter::Arg arg4,
                                Formatter::Arg arg5,
                                Formatter::Arg arg6,
                                Formatter::Arg arg7,
                                Formatter::Arg arg8,
                                Formatter::Arg arg9)
{
    MessageLoaderParms parms(key,default_msg,
                       arg0,arg1,arg2,arg3,arg4,
                       arg5,arg6,arg7,arg8,arg9);

    if(strlen(resource_path) > 0)
        parms.msg_src_path = resource_path;

    return MessageLoader::getMessage(parms);

}
//l10n end

PEGASUS_NAMESPACE_END
