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


////////////////////////////////////////////////////////////////////////////////
//  This file contains the exception classes used in the configuration
//  classes.
////////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_ConfigExceptions_h
#define Pegasus_ConfigExceptions_h

#include <Pegasus/Config/Linkage.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/MessageLoader.h>

PEGASUS_NAMESPACE_BEGIN

/**
    UnrecognizedCommandLineOption Exception class
*/
class UnrecognizedCommandLineOption : public Exception
{
public:
    UnrecognizedCommandLineOption();
};


/**
    InvalidPropertyValue Exception class
*/
class PEGASUS_CONFIG_LINKAGE InvalidPropertyValue : public Exception
{
public:
    InvalidPropertyValue(const String& name, const String& value);
protected:
    InvalidPropertyValue(const MessageLoaderParms& theMessage);
};


/**
    InvalidDirectoryPropertyValue Exception class
*/
class InvalidDirectoryPropertyValue
   : public InvalidPropertyValue
{
public:
    InvalidDirectoryPropertyValue(const String& name, const String& value);
};

/**
    InvalidListenAddressPropertyValue Exception class
*/
class InvalidListenAddressPropertyValue
   : public InvalidPropertyValue
{
public:
    InvalidListenAddressPropertyValue(const String& name, const String& value);
};

/**
    ConfigFileSyntaxError Exception class
*/
class ConfigFileSyntaxError : public Exception
{
public:
    ConfigFileSyntaxError(const String& file, Uint32 line);

    static String _formatMessage(const String& file, Uint32 line);
};


/**
    UnrecognizedConfigProperty Exception class
*/
class UnrecognizedConfigProperty : public Exception
{
public:
    UnrecognizedConfigProperty(const String& name);
};

/**
    NonDynamicConfigProperty Exception class
*/
class NonDynamicConfigProperty : public Exception
{
public:
    NonDynamicConfigProperty(const String& name);
};

/**
    FailedSaveProperties Exception class
*/
class FailedSaveProperties : public Exception
{
public:
    FailedSaveProperties(const String& reason);
};


PEGASUS_NAMESPACE_END

#endif /* Pegasus_ConfigExceptions_h */
