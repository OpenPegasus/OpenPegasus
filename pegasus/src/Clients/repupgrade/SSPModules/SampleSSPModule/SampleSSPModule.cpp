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

#include "SampleSSPModule.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

static const String _REQUIRED_QUAL_NAME = "Required";

SampleSSPModule::SampleSSPModule()
{
}

SampleSSPModule::~SampleSSPModule()
{
}

/**
    Performs special processing on a qualifier.

    @param  CIMQualifier qualifier.

    @return     CIMQualifier updated qualifier.
*/
Boolean SampleSSPModule::processQualifier (CIMQualifierDecl& inputQual,
                                           CIMQualifierDecl& outputQual)
{
    return true;
}

/**
    Interface for special processing on a class.
*/
Boolean SampleSSPModule::processClass (CIMClass& inputClass,
                                       CIMClass& outputClass)
{
/* This is a sample implementation to update a CIM Class
   Uncomment for implementation

    //
    // Check if the input class defines "Required" qualifier and
    // if the flavor on the qualifier is set to OVERRIDABLE.
    // If true unset it in the output class.
    //

    Uint32 count = inputClass.getQualifierCount();
    for ( Uint32 i = 0; i < count; i++ )
    {
         CIMQualifier q = inputClass.getQualifier(i);

         if ( q.getName() == _REQUIRED_QUAL_NAME )
         {
             // change the qualifier flavor
             if ( q.getFlavor().hasFlavor( CIMFlavor::OVERRIDABLE))
             {
                 q.unsetFlavor (CIMFlavor::OVERRIDABLE);
                 outputClass.removeQualifier(i);
                 outputClass.addQualifier(q);
             }
         }
    }

    //
    // Check if any property defines "Required" qualifier.
    //

    Array<CIMProperty>      cimProperty;

    for ( Uint32 i = 0; i < inputClass.getPropertyCount(); i++ )
    {
        CIMProperty  p = inputClass.getProperty(i);

        Uint32 qualCount = p.getQualifierCount();

        for ( Uint32 qctr = 0; qctr < qualCount; qctr++)
        {
            CIMQualifier q = p.getQualifier(qctr);

            if ( q.getName() == _REQUIRED_QUAL_NAME )
            {
                // change the qualifier flavor
                if ( q.getFlavor().hasFlavor(
                         CIMFlavor::OVERRIDABLE))
                {
                    q.unsetFlavor (CIMFlavor::OVERRIDABLE);
                    p.removeQualifier(qctr);
                    p.addQualifier(q);
                    cimProperty.append(p);
                    break;
                }
            }
        }
    }

    for ( Uint32 i=0; i<cimProperty.size(); i++)
    {
        Uint32 pos = outputClass.findProperty(cimProperty[i].getName());
        outputClass.removeProperty(pos);
        outputClass.addProperty (cimProperty[i]);
    }

    //
    // Check if any Method defines "Required" qualifier.
    //


    for ( Uint32 i = 0; i < inputClass.getMethodCount(); i++ )
    {
         CIMMethod  m = inputClass.getMethod (i);

         Uint32 qualCount = m.getQualifierCount();

         for ( Uint32 qctr = 0; qctr < qualCount; qctr++)
         {
             CIMQualifier q = m.getQualifier(qctr);

             if ( q.getName() == _REQUIRED_QUAL_NAME )
             {
                 // change the qualifier flavor
                 if ( q.getFlavor().hasFlavor(
                           CIMFlavor::OVERRIDABLE))
                 {
                     q.unsetFlavor (CIMFlavor::OVERRIDABLE);
                     outputClass.removeMethod (i);
                     outputClass.addMethod (m);
                 }
                 break;
             }
         }
    }

    //
    // Check if any Method parameter defines
    // "Required" qualifier.
    //

    for ( Uint32 i = 0; i < inputClass.getMethodCount(); i++ )
    {
        CIMMethod  m = inputClass.getMethod (i);

        // Check the method parameters.
        Boolean methodChanged = false;
        Uint32 paramCount = m.getParameterCount();

        for ( Uint32 pctr = 0; pctr < paramCount; pctr++)
        {
            CIMParameter p1  = m.getParameter(pctr);

            Uint32 pqCount = p1.getQualifierCount();
            for ( Uint32 pqctr = 0; pqctr < pqCount; pqctr++)
            {
                CIMQualifier q = p1.getQualifier(pqctr);

                if ( q.getName() == _REQUIRED_QUAL_NAME )
                {
                     // change the qualifier flavor
                    if ( q.getFlavor().hasFlavor
                               ( CIMFlavor::OVERRIDABLE))
                    {
                        q.unsetFlavor (CIMFlavor::OVERRIDABLE);
                        methodChanged = true;
                        p1.removeQualifier(pqctr);
                        p1.addQualifier (q);
                        m.removeParameter(pctr);
                        m.addParameter(p1);
                    }
                    break;
                }
            }
        }
        if ( methodChanged)
        {
            outputClass.removeMethod (i);
            outputClass.addMethod (m);
        }
    }

Uncomment for implementation */

    return true;
}

/**
    Interface for special processing on an instance.
*/
Boolean SampleSSPModule::processInstance (CIMInstance& inputInstance,
                                                  CIMInstance& outputInstance)
{

/* This is a sample implementation to update a CIM Instance.
   Uncomment for implementation

    Array<CIMProperty> cimProperty;

    //
    // Check if the input instance defines "Required" qualifier and
    // if the flavor on the qualifier is set to OVERRIDABLE.
    // If true unset it in the output instance.
    //

    for ( Uint32 i = 0; i < inputInstance.getPropertyCount(); i++ )
    {
        CIMProperty  p = inputInstance.getProperty (i);

        Uint32 qualCount = p.getQualifierCount();

        for ( Uint32 qctr = 0; qctr < qualCount; qctr++)
        {
            CIMQualifier q = p.getQualifier(qctr);

            if ( q.getName() == _REQUIRED_QUAL_NAME)

            {
                // change the qualifier flavor
                if ( q.getFlavor().hasFlavor(
                    CIMFlavor::OVERRIDABLE))
                {
                    q.unsetFlavor (CIMFlavor::OVERRIDABLE);
                    p.removeQualifier(qctr);
                    p.addQualifier(q);
                    cimProperty.append(p);
                }
                break;
            }
        }
    }
    for ( Uint32 i=0; i<cimProperty.size(); i++)
    {
         outputInstance.removeProperty(
             outputInstance.findProperty (cimProperty[i].getName()));
         outputInstance.addProperty (cimProperty[i]);
    }

Uncomment for implementation */

    return true;
}

extern "C" PEGASUS_EXPORT SchemaSpecialProcessModule * PegasusCreateSSPModule()
{
    return(new SampleSSPModule());
}
PEGASUS_NAMESPACE_END

