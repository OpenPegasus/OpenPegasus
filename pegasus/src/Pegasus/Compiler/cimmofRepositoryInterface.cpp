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
#include <Pegasus/Common/CIMQualifierDecl.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include "cimmofRepository.h"
#include "cimmofClient.h"
#include "cimmofParser.h"
#include "cimmofMessages.h"
#include "cimmofRepositoryInterface.h"

#ifdef PEGASUS_ENABLE_MRR_GENERATION
# include "cimmofMRR.h"
#endif

PEGASUS_USING_PEGASUS;

cimmofRepositoryInterface::cimmofRepositoryInterface() :
  _repository(0),
  _client(0),
  _mrr(0),
  _ot(compilerCommonDefs::USE_REPOSITORY)
{
}

cimmofRepositoryInterface::~cimmofRepositoryInterface()
{
    delete _repository;
    delete _client;
#ifdef PEGASUS_ENABLE_MRR_GENERATION
    delete _mrr;
#endif
}

// The descriptions parameter controls inclusion of descriptions in
// mof output. When false, descriptions are not used.  Today it is
// used only with the MRR Generation because that is where size is
// critical and the description qualifiers add significant size.
void cimmofRepositoryInterface::init(_repositoryType type, String location,
    Uint32 mode,
    compilerCommonDefs::operationType ot,
    bool descriptions)
{
    String message;
    cimmofMessages::arglist arglist;
    _ot = ot;
    if (type == REPOSITORY_INTERFACE_LOCAL)
    {
        // create a cimmofRepository object and put it in _repository
        cimmofParser *p = cimmofParser::Instance();
        const String NameSpace = p->getDefaultNamespacePath();
        if (location != "")
        {
            try
            {
                _repository = new cimmofRepository(location, mode, _ot);
            }
            catch(Exception &e)
            {
                arglist.append(location);
                arglist.append(e.getMessage());
                cimmofMessages::getMessage(message,
                    cimmofMessages::REPOSITORY_CREATE_ERROR,
                    arglist);
                p->elog(message);
                delete _repository;
                _repository = 0;
            }
        }
    }
    else if (type == REPOSITORY_INTERFACE_CLIENT)
    {
        // create a CIMClient object and put it in _client
        _client = new cimmofClient();
        try
        {
            _client->init(ot);
        }
        catch (const CannotConnectException &)
        {
            delete _client;
            _client = 0;
            throw;
        }
        catch(Exception &e)
        {
            arglist.append(location);
            arglist.append(e.getMessage());
            cimmofMessages::getMessage(message,
                cimmofMessages::REPOSITORY_CREATE_ERROR,
                arglist);
            cimmofParser *p = cimmofParser::Instance();
            p->elog(message);
            delete _client;
            _client = 0;
        }
    }
#ifdef PEGASUS_ENABLE_MRR_GENERATION
    else if (type == REPOSITORY_INTERFACE_MRR)
    {
        // create memory-resident repository handler.
        _mrr = new cimmofMRR(descriptions);
    }
    else   // Not valid type
    {
        arglist.append(location);
        arglist.append("Compiler Internal Error");
        cimmofMessages::getMessage(message,
                cimmofMessages::REPOSITORY_CREATE_ERROR,
                arglist);
        cimmofParser *p = cimmofParser::Instance();
        p->elog(message);
    }
#else   
    else // not valid type
    {
        // hide the compiler warning that the descriptions parameter is not
        // used except with MRR generation
        (void)descriptions;

        
        // Generate error message and Terminate
        arglist.append(location);
        arglist.append("Compiler Internal Error");
        cimmofMessages::getMessage(message,
                cimmofMessages::REPOSITORY_CREATE_ERROR,
                arglist);
        cimmofParser *p = cimmofParser::Instance();
        p->elog(message);
    }
#endif
}

void cimmofRepositoryInterface::addClass(const CIMNamespaceName &nameSpace,
    CIMClass &Class)  const
{
    if (_repository)
        _repository->addClass(nameSpace, &Class);
    if (_client)
        _client->addClass(nameSpace, Class);
#ifdef PEGASUS_ENABLE_MRR_GENERATION
    if (_mrr)
        _mrr->addClass(nameSpace, Class);
#endif
}

void cimmofRepositoryInterface::addQualifier(const CIMNamespaceName &nameSpace,
                    CIMQualifierDecl &qualifier) const
{
    if (_repository)
        _repository->addQualifier(nameSpace, &qualifier);
    if (_client)
        _client->addQualifier(nameSpace, qualifier);
#ifdef PEGASUS_ENABLE_MRR_GENERATION
    if (_mrr)
        _mrr->addQualifier(nameSpace, qualifier);
#endif
}

void cimmofRepositoryInterface::addInstance(const CIMNamespaceName &nameSpace,
                       CIMInstance &instance) const
{
    if (_repository)
        _repository->addInstance(nameSpace, &instance);
    if (_client)
        _client->addInstance(nameSpace, instance);
#ifdef PEGASUS_ENABLE_MRR_GENERATION
    if (_mrr)
        _mrr->addInstance(nameSpace, instance);
#endif
}

CIMQualifierDecl cimmofRepositoryInterface::getQualifierDecl(
        const CIMNamespaceName &nameSpace,
        const CIMName &qualifierName) const
{
    if (_repository)
        return (_repository->getQualifierDecl(nameSpace, qualifierName));
    else if (_client)
        return (_client->getQualifierDecl(nameSpace, qualifierName));
#ifdef PEGASUS_ENABLE_MRR_GENERATION
    else if (_mrr)
        return (_mrr->getQualifierDecl(nameSpace, qualifierName));
#endif
    else
        return CIMQualifierDecl();
}

CIMClass cimmofRepositoryInterface::getClass(const CIMNamespaceName &nameSpace,
                    const CIMName &className) const
{
    if (_repository)
        return (_repository->getClass(nameSpace, className));
    else if (_client)
        return (_client->getClass(nameSpace, className));
#ifdef PEGASUS_ENABLE_MRR_GENERATION
    else if (_mrr)
        return (_mrr->getClass(nameSpace, className));
#endif
    else
        return CIMClass();
}

void cimmofRepositoryInterface::modifyClass(const CIMNamespaceName &nameSpace,
    CIMClass &Class) const
{
    if (_repository)
    {
        _repository->modifyClass(nameSpace, &Class);
    }
    if (_client)
    {
        _client->modifyClass(nameSpace, Class);
    }
#ifdef PEGASUS_ENABLE_MRR_GENERATION
    if (_mrr)
    {
        _mrr->modifyClass(nameSpace, Class);
    }
#endif
}

void cimmofRepositoryInterface::createNameSpace(
    const CIMNamespaceName &nameSpace) const
{
    if (_repository)
        _repository->createNameSpace(nameSpace);
    else if (_client)
    {
        _client->createNameSpace(nameSpace);
    }
#ifdef PEGASUS_ENABLE_MRR_GENERATION
    else if (_mrr)
    {
        _mrr->createNameSpace(nameSpace);
    }
#endif
}

void cimmofRepositoryInterface::start()
{
#ifdef PEGASUS_ENABLE_MRR_GENERATION
    if (_mrr)
        _mrr->start();
#endif
}

void cimmofRepositoryInterface::finish()
{
#ifdef PEGASUS_ENABLE_MRR_GENERATION
    if (_mrr)
        _mrr->finish();
#endif
}
