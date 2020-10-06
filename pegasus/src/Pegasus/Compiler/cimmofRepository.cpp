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
// implementation of  cimmofRepository
//
//
//
// This class acts as a buffer between the compiler and the repository
// interface.  The main thing it does is registers a non-standard
// DeclContext so we can do local checking of context for new objects
//

#include <Pegasus/Common/InternalException.h>
#include "cimmofRepository.h"

PEGASUS_USING_PEGASUS;

cimmofRepository::cimmofRepository(const String& path,
    Uint32 mode,
    compilerCommonDefs::operationType ot)
    : _cimrepository(0), _context(0), _ot(ot)
{
    // Decl context is allocated here but will be owned and deleted by
    // the CIMRepository class
    _context = new compilerDeclContext(_ot);

    // don't catch the exceptions that might be thrown.  They should go up.
    if (_ot != compilerCommonDefs::IGNORE_REPOSITORY) {
        _cimrepository = new CIMRepository(path, mode, _context);
    }

    if (_cimrepository)
    {
        _context->setRepository(_cimrepository);
    }
    else if (ot != compilerCommonDefs::IGNORE_REPOSITORY)
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "attempt to initialize repository with invalid data");
    }
}

cimmofRepository::~cimmofRepository()
{
    delete _cimrepository;
}

int cimmofRepository::addClass(const CIMNamespaceName &nameSpace,
    CIMClass *classdecl)
{
    try
    {
        _context->addClass( nameSpace,  *classdecl);
    }
    catch (CIMException& e)
    {
        // Convert the exception message to the one that would be received by
        // a client.
        throw CIMException(
            e.getCode(), TraceableCIMException(e).getDescription());
    }

    return 0;
}


int cimmofRepository::addInstance(const CIMNamespaceName &nameSpace,
    CIMInstance *instance)
{
    try
    {
        _context->addInstance(nameSpace, *instance);
    }
    catch (CIMException& e)
    {
        // Convert the exception message to the one that would be received by
        // a client.
        throw CIMException(
            e.getCode(), TraceableCIMException(e).getDescription());
    }

    return 0;
}

int cimmofRepository::addQualifier(const CIMNamespaceName &nameSpace,
    CIMQualifierDecl *qualifier)
{
    try
    {
        _context->addQualifierDecl(nameSpace, *qualifier);
    }
    catch (CIMException& e)
    {
        // Convert the exception message to the one that would be received by
        // a client.
        throw CIMException(
            e.getCode(), TraceableCIMException(e).getDescription());
    }

    return 0;
}

CIMQualifierDecl cimmofRepository::getQualifierDecl(
    const CIMNamespaceName &nameSpace,
    const CIMName &name)
{
    try
    {
        return _context->lookupQualifierDecl(nameSpace, name);
    }
    catch (CIMException& e)
    {
        // Convert the exception message to the one that would be received by
        // a client.
        throw CIMException(
            e.getCode(), TraceableCIMException(e).getDescription());
    }
}

CIMClass cimmofRepository::getClass(const CIMNamespaceName &nameSpace,
    const CIMName &classname)
{
    try
    {
        return _context->lookupClass(nameSpace, classname);
    }
    catch (CIMException& e)
    {
        // Convert the exception message to the one that would be received by
        // a client.
        throw CIMException(
            e.getCode(), TraceableCIMException(e).getDescription());
    }
}

int cimmofRepository::modifyClass(const CIMNamespaceName &nameSpace,
    CIMClass *classdecl)
{
    try
    {
        _context->modifyClass( nameSpace,  *classdecl);
    }
    catch (CIMException& e)
    {
        // Convert the exception message to the one that would be received by
        // a client.
        throw CIMException(
            e.getCode(), TraceableCIMException(e).getDescription());
    }

    return 0;
}

void cimmofRepository::createNameSpace(const CIMNamespaceName &nameSpaceName)
{
    if (_cimrepository && _ot != compilerCommonDefs::IGNORE_REPOSITORY)
    {
        try
        {
            _cimrepository->createNameSpace(nameSpaceName);
        }
        catch (CIMException& e)
        {
            // Convert the exception message to the one that would be received
            // by a client.
            throw CIMException(
                e.getCode(), TraceableCIMException(e).getDescription());
        }
    }
}
