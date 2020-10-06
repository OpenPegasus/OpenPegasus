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


//
// interface definition for the cimmofRepository class, a specialiazation
// of the Pegasus CIMRepository class with error handling.
//

#ifndef _CIMMOFREPOSITORY_H_
#define _CIMMOFREPOSITORY_H_


#include <Pegasus/Common/Config.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Common/String.h>
//#include <Pegasus/Compiler/cimmofParser.h>
#include <Pegasus/Compiler/compilerDeclContext.h>
#include <Pegasus/Compiler/Linkage.h>

PEGASUS_NAMESPACE_BEGIN


class PEGASUS_COMPILER_LINKAGE cimmofRepository
{
    public:
        cimmofRepository(
                const String& path,
                Uint32 mode,
                compilerCommonDefs::operationType ot);
        virtual ~cimmofRepository();

        // Add some methods for use at the compiler level
        virtual int addClass(const CIMNamespaceName &nameSpace,
                CIMClass *classdecl);
        virtual int addInstance(
                const CIMNamespaceName &nameSpace,
                CIMInstance *instance);
        virtual int addQualifier(
                const CIMNamespaceName &nameSpace,
                CIMQualifierDecl *qualifier);

        virtual CIMQualifierDecl getQualifierDecl(
                const CIMNamespaceName &nameSpace,
                const CIMName &name);
        virtual CIMClass getClass(
                const CIMNamespaceName &nameSpace,
                const CIMName &classname);

        virtual int modifyClass(const CIMNamespaceName &nameSpace,
                CIMClass *classdecl);

        virtual void createNameSpace(const CIMNamespaceName &nameSpaceName);

    private:
        CIMRepository *_cimrepository;
        compilerDeclContext *_context;
        compilerCommonDefs::operationType _ot;
};

PEGASUS_NAMESPACE_END

#endif
