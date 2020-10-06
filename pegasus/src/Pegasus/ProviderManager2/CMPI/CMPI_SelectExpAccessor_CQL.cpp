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

#include "CMPI_Version.h"

#include "CMPI_SelectExpAccessor_CQL.h"
#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>

#include <Pegasus/Common/String.h>
#include <Pegasus/CQL/CQLParser.h>
#include <Pegasus/CQL/CQLParserState.h>
#include <Pegasus/CQL/CQLSelectStatement.h>
#include <Pegasus/Repository/RepositoryQueryContext.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/CQL/CQLExpression.h>
#include <Pegasus/CQL/CQLSimplePredicate.h>
#include <Pegasus/CQL/CQLPredicate.h>
#include <Pegasus/CQL/CQLValue.h>
#include <Pegasus/Query/QueryCommon/QueryChainedIdentifier.h>
#include <Pegasus/Common/Tracer.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

CMPI_SelectExpAccessor_CQL::CMPI_SelectExpAccessor_CQL (
    CMPIAccessor * acc,
    void *parm,
    CQLSelectStatement *
    stmt,
    CIMObjectPath &
    objPath):accessor (acc),accParm (parm),_stmt (stmt),_objPath (objPath)
{
    /**
        Construct an _instance
    */
    _constructInstance ();
}

void CMPI_SelectExpAccessor_CQL::_constructInstance ()
{

    PEG_METHOD_ENTER(
        TRC_CMPIPROVIDERINTERFACE,
        "CMPI_SelectExpAccessor_CQL::_constructInstance()");
    _instance = CIMInstance (_objPath.getClassName ().getString ());
     /**
        Iterate throught the CQLPredicates
     */

    Array < CQLChainedIdentifier > where_Array =
    _stmt->getWhereChainedIdentifiers ();

    /**
      We will create an instance using the where_Array properties
   */

    for( Uint32 i = 0; i < where_Array.size (); i++ )
    {
        CQLIdentifier identifier = where_Array[i].getLastIdentifier ();
        String name = identifier.getName ().getString ();

        /**
           cerr << "Calling accessor function with property: " << name
                << endl;
       */

        CMPIAccessor *get = (CMPIAccessor *) accessor;
        CMPIData data = get (name.getCString (), accParm);

        /**
          Only process good values.
       */
        if( (data.state == CMPI_goodValue) || (data.state == CMPI_keyValue) )
        {

            if( data.type & CMPI_ARRAY )
            {
            /**
               Nothing yet
            */
            }
            else
            if( (data.type & (CMPI_UINT | CMPI_SINT)) == CMPI_SINT )
            {
                switch( data.type )
                {
                    case CMPI_sint32:
                        _instance.
                        addProperty (CIMProperty
                        (CIMName (name),
                        Sint32 (data.value.sint32)));
                        break;
                    case CMPI_sint16:
                        _instance.
                        addProperty (CIMProperty
                        (CIMName (name),
                        Sint16 (data.value.sint16)));
                        break;
                    case CMPI_sint8:
                        _instance.
                        addProperty (CIMProperty
                        (CIMName (name),
                        Sint8 (data.value.sint8)));
                        break;
                    case CMPI_sint64:
                        _instance.
                        addProperty (CIMProperty
                        (CIMName (name),
                        Sint64 (data.value.sint64)));
                        break;
                    default:
                        break;
                }
            }
            else
            if( data.type == CMPI_chars )
            {
                _instance.
                addProperty (CIMProperty
                (CIMName (name), String (data.value.chars)));
            }
            else
            if( data.type == CMPI_string )
            {
                CMPIStatus rc;
                _instance.
                addProperty (CIMProperty
                (CIMName (name),
                String (CMGetCharsPtr
                (data.value.string, &rc))));
            }
            else
            if( (data.type & (CMPI_UINT | CMPI_SINT)) == CMPI_UINT )
            {
                switch( data.type )
                {
                    case CMPI_uint32:
                        _instance.
                        addProperty (CIMProperty
                        (CIMName (name),
                        Uint32 (data.value.uint32)));
                        break;
                    case CMPI_uint16:
                        _instance.
                        addProperty (CIMProperty
                        (CIMName (name),
                        Uint16 (data.value.uint16)));
                        break;
                    case CMPI_uint8:
                        _instance.
                        addProperty (CIMProperty
                        (CIMName (name), Uint8 (data.value.uint8)));
                        break;
                    case CMPI_uint64:
                        _instance.
                        addProperty (CIMProperty
                        (CIMName (name),
                        Uint64 (data.value.uint64)));
                        break;
                    default:;
                        break;
                }
            }
            else
                switch( data.type )
                {
                    case CMPI_boolean:
                        _instance.
                        addProperty (CIMProperty
                        (CIMName (name),
                        Boolean (data.value.boolean)));
                        break;
                    case CMPI_real32:
                        _instance.
                        addProperty (CIMProperty
                        (CIMName (name), data.value.real32));
                        break;
                    case CMPI_real64:
                        _instance.
                        addProperty (CIMProperty
                        (CIMName (name), data.value.real64));
                        break;
                    default:
                        break;
                }
        }
    }
    PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END

