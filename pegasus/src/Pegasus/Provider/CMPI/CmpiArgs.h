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

#ifndef _CmpiArgs_h_
#define _CmpiArgs_h_

#include "cmpidt.h"
#include "cmpift.h"

#include "CmpiBaseMI.h"
#include "CmpiObject.h"
#include "CmpiStatus.h"
#include "CmpiObjectPath.h"
#include "CmpiBroker.h"
#include "CmpiBaseMI.h"

/**
    This class wraps a CIMOM specific input and output arguments
    passed to methodInvocation functions.
*/

class PEGASUS_CMPI_PROVIDER_LINKAGE CmpiArgs : public CmpiObject
{
    friend class CmpiBroker;
    friend class CmpiMethodMI;
protected:

    /**
        Protected constructor used by MIDrivers to encapsulate CMPIArgs.
    */
    CmpiArgs(CMPIArgs* enc);

    /**
        Gets the encapsulated CMPIArgs.
    */
    CMPIArgs *getEnc() const;

    void *makeArgs(CMPIBroker* mb);
private:
public:

    /**
        Constructor - Empty argument container.
    */
    CmpiArgs();

    /**
        getArgCount-Gets the number of arguments
        defined for this argument container.*
        @return Number of arguments in this container.
    */
    unsigned int getArgCount() const;

    /**
        getArg - Gets the CmpiData object representing the value
        associated with the argument  name
        @param name argument name.
        @return CmpiData value object associated with the argument.
    */
    CmpiData getArg(const char* name) const;

    /**
        getArgument - Gets the CmpiData object defined
        by the input index parameter.
        @param index Index into the argument array.
        @param name Optional output parameter returning the argument name.
        @return CmpiData value object corresponding to the index.
    */
    CmpiData getArg(const int index, CmpiString *name=NULL) const;

    /**
        setArg - adds/replaces an argument value defined by the
        input parameter to the container.
        @param name Property name.
        @param data Type and Value to be added.
    */
    void setArg(const char* name, const CmpiData& data);
};

#endif
