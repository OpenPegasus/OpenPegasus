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
// Author: Sushma Fernandes, Hewlett-Packard Company
//         (sushma_fernandes@hp.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_SSPModule_h
#define Pegasus_SSPModule_h

#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMQualifierDecl.h>

PEGASUS_NAMESPACE_BEGIN

/**
     When moving to a newer version of CIM schema, certain DMTF schema changes
     could make some elements in an old repository incompatible with
     the new schema.

     In order to import the impacted elements to the new repository,
     special processing may have to be performed to make them compatible
     with the newer version of the schema.

     The special processing is performed in a Schema Special
     Processsing Module.  A Schema Special Processsing Module will inherit
     from a SchemaSpecialProcessModule class defined below:

 */
class SchemaSpecialProcessModule
{

public:

    /**

        Constructor

    */
    SchemaSpecialProcessModule(){};

    /**

        Destructor

    */
    virtual ~SchemaSpecialProcessModule(){};

    /**
        Perform special processing on a CIMQualifier. This includes verifying
        whether the qualifier is impacted. If yes, it must be appropriately
        updated.

         @param       inputQual       the qualifier to be processed. This will
                                      be provided by the framework.

         @param       outputQual      updated qualifier.

         @return      true            if the qualifier should be created.
                      false           if the framework should
                                      ignore the qualifier.
    */
    virtual Boolean processQualifier(
        CIMQualifierDecl& inputQual,
        CIMQualifierDecl& outputQual) = 0;

    /**

        Perform special processing on a CIMClass. This includes verifying
        whether the class is impacted. If yes, it must be appropriately updated.

        @param       inputClass       the class to be processed. This will
                                      be provided by the framework.

        @param       outputClass      updated class.

        @return      true             if the class should be created.

                     false            if the framework should
                                      ignore the class.


    */
    virtual Boolean processClass (CIMClass& inputClass,
                                  CIMClass& outputClass) = 0;

    /**

        Perform special processing on a CIMInstance. This includes verifying
        whether the instance is impacted. If yes, it must be appropriately
        updated.

        @param       inputInstance   the qualifier to be processed. This will
                                     be provided by the framework.

        @param       outputInstance  updated instance.

        @return      true            if the instance should be created.

                     false           if the framework should ignore
                                     instance.

    */
    virtual Boolean processInstance (CIMInstance& inputInstance,
                                     CIMInstance& outputInstance) = 0;

    /**

        Defines the Special Processing Module entry point.

    */
    static const String CREATE_SSPMODULE_ENTRY_POINT;

};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_SSPModule_h */
