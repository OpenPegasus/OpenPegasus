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
// Author: Chip Vincent (cvincent@us.ibm.com)
//
// Modified By: Mike Day (mdday@us.ibm.com)
//              Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
//              Jair F. T. dos Santos, Hewlett-Packard Company
//                  (jair.santos@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_WMIMapperUserInfoContainer_h
#define Pegasus_WMIMapperUserInfoContainer_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Server/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_SERVER_LINKAGE WMIMapperUserInfoContainer :
    virtual public OperationContext::Container
{
public:
    static const String NAME;

    WMIMapperUserInfoContainer(const OperationContext::Container & container);
    WMIMapperUserInfoContainer(const String & password);

    virtual String getName(void) const;
    virtual OperationContext::Container * clone(void) const;
    virtual void destroy(void);

    String getPassword(void) const;

protected:
    String _password;

};

PEGASUS_NAMESPACE_END

#endif

