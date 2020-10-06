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
// This class is the interface between the cimmof compiler, in its various
// forms, and the various Pegasus repository interfaces which as the
// time this class was created were CIMRepository and CIMClient.
//
// This class supports only the operations that the compiler needs, which
// are
//     addClass()
//     addInstance()
//     addQualifier()
//     createNameSpace()
//
// If we create compiler-like tools to do mass changes to the repository,
// then I expect that we will add methods to deal with the modification.
// This class is intended to be very light, basically making it easy
// to choose what repository and what repository interface to use.
// It includes both, since there's nothing to be saved by splitting them.
// Anything that the client or repository interface throws gets passed
// to the cimmofParser level, which is equipped to handle the exceptions
//

#ifndef CIMMOF_REPOSITORY_INTERFACE_H_
#define CIMMOF_REPOSITORY_INTERFACE_H_

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Compiler/Linkage.h>
#include "mofCompilerOptions.h"
#include <Pegasus/Repository/CIMRepository.h>

PEGASUS_NAMESPACE_BEGIN

// Forward declarations
class cimmofRepository;
class cimmofClient;
class cimmofMRR;
class CIMClass;
class CIMQualifierDecl;
class CIMInstance;

class PEGASUS_COMPILER_LINKAGE cimmofRepositoryInterface
{
    private:
        cimmofRepository *_repository;
        cimmofClient        *_client;
        cimmofMRR* _mrr;
        compilerCommonDefs::operationType _ot;
    public:
        enum _repositoryType
        {
            REPOSITORY_INTERFACE_LOCAL = 0,
            REPOSITORY_INTERFACE_CLIENT = 1,
            REPOSITORY_INTERFACE_MRR = 2
        };
        cimmofRepositoryInterface();
        virtual ~cimmofRepositoryInterface();
        /*
            Initialize a repository
            @param type repository type defines whether client, local or
            MRR repository is to be initialized
            @param location String defining location of the repository. The
            exact format depends on type paramter
            @param mode
            @param ot operationType
            @param descriptions bool that defines whether descripton
            qualifiers are to be included in the compiled output. This
            parameter is used ONLY for MMR compilations.
        */
        void init(_repositoryType type, String location, Uint32 mode,
                compilerCommonDefs::operationType ot, bool descriptions);
        Boolean ok() const
        {
            return _repository || _client || _mrr;
        }
        virtual void addClass(
                const CIMNamespaceName &nameSpace,
                CIMClass &Class) const;
        virtual void addQualifier(
                const CIMNamespaceName &nameSpace,
                CIMQualifierDecl &qual) const;
        virtual void addInstance(
                const CIMNamespaceName &nameSpace,
                CIMInstance &instance) const;
        virtual CIMQualifierDecl getQualifierDecl(
                const CIMNamespaceName &nameSpace,
                const CIMName &qualifierName) const;
        virtual CIMClass getClass(
                const CIMNamespaceName &nameSpace,
                const CIMName &className) const;
        virtual void modifyClass(
                const CIMNamespaceName &nameSpace,
                CIMClass &Class) const;
        virtual void createNameSpace(const CIMNamespaceName &nameSpace) const;
        virtual void start();
        virtual void finish();
};

PEGASUS_NAMESPACE_END

#endif




