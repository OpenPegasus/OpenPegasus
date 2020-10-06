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
// This code implements part of PEP#348 - The CMPI infrastructure using SCMO
// (Single Chunk Memory Objects).
// The design document can be found on the OpenPegasus website openpegasus.org
// at https://collaboration.opengroup.org/pegasus/pp/documents/21210/PEP_348.pdf
//
//%/////////////////////////////////////////////////////////////////////////////

#include "CMPI_Version.h"

#include "CMPI_String.h"
#include "CMPI_Value.h"
#include "CMPISCMOUtilities.h"

#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/CIMDateTimeInline.h>
#include <Pegasus/Common/StringConversion.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

CIMDateTimeRep* CMPISCMOUtilities::scmoDateTimeFromCMPI(CMPIDateTime* cmpidt)
{
    CIMDateTimeRep* cimdt = 0;

    if (cmpidt && cmpidt->hdl)
    {
        cimdt = ((CIMDateTime*)cmpidt->hdl)->_rep;
    }
    return cimdt;
}


// Function to copy all key bindings from one SCMOInstance to another
CMPIrc CMPISCMOUtilities::copySCMOKeyProperties( const SCMOInstance* sourcePath,
                                                 SCMOInstance* targetPath )
{
    PEG_METHOD_ENTER(
        TRC_CMPIPROVIDERINTERFACE,
        "CMPISCMOUtilities::copySCMOKeyProperties()");

    if ((0!=sourcePath) && (0!=targetPath))
    {
        SCMO_RC rc;
        const char* keyName = 0;
        const SCMBUnion* keyValue = 0;
        CIMType keyType;

        Uint32 numKeys = sourcePath->getKeyBindingCount();
        for (Uint32 x=0; x < numKeys; x++)
        {
            rc = sourcePath->getKeyBindingAt(
                x, &keyName, keyType, &keyValue);
            if (rc==SCMO_OK)
            {
                rc = targetPath->setKeyBinding(
                    keyName, keyType, keyValue);
                if (keyType == CIMTYPE_STRING)
                {
                    free((void*)keyValue);
                }
                if (rc != SCMO_OK)
                {
                    PEG_TRACE_CSTRING(
                        TRC_CMPIPROVIDERINTERFACE,
                        Tracer::LEVEL2,
                        "Failed to set keybinding");
                    PEG_METHOD_EXIT();
                    return CMPI_RC_ERR_FAILED;
                }
            }
            else
            {
                if (rc!=SCMO_NULL_VALUE)
                {
                    PEG_TRACE_CSTRING(
                        TRC_CMPIPROVIDERINTERFACE,
                        Tracer::LEVEL2,
                        "Failed to retrieve keybinding");
                    PEG_METHOD_EXIT();
                    return CMPI_RC_ERR_FAILED;
                }
            }
        }
    }
    else
    {
        PEG_TRACE_CSTRING(
            TRC_CMPIPROVIDERINTERFACE,
            Tracer::LEVEL1,
            "Called with Nullpointer for source or target");
        PEG_METHOD_EXIT();
        return CMPI_RC_ERR_FAILED;
    }

    return CMPI_RC_OK;
}

// Function to convert a CIMInstance into an SCMOInstance
// CAUTION: This function requires access to the CMPIClassCache, and
//          therefore can only be called from within a CMPI provider !!!
SCMOInstance* CMPISCMOUtilities::getSCMOFromCIMInstance(
    const CIMInstance& cimInst,
    const char* ns,
    const char* cls)
{
    const CIMObjectPath& cimPath = cimInst.getPath();

    const CString nameSpace = cimPath.getNameSpace().getString().getCString();
    const CString className = cimPath.getClassName().getString().getCString();

    if (!ns)
    {
        ns = (const char*)nameSpace;
    }
    if (!cls)
    {
        cls = (const char*)className;
    }

    SCMOInstance* scmoInst=0;

    SCMOClass* scmoClass = mbGetSCMOClass(ns,strlen(ns),cls,strlen(cls));

    if (0 != scmoClass)
    {
        scmoInst = new SCMOInstance(*scmoClass, cimInst);
    }
    else
    {
        SCMOClass localDirtySCMOClass(cls,ns);
        scmoInst = new SCMOInstance(localDirtySCMOClass, cimInst);
        scmoInst->markAsCompromised();
    }

    return scmoInst;
}

// Function to convert a CIMObjectPath into an SCMOInstance
// CAUTION: This function requires access to the CMPIClassCache, and
//          therefore can only be called from within a CMPI provider !!!
SCMOInstance* CMPISCMOUtilities::getSCMOFromCIMObjectPath(
    const CIMObjectPath& cimPath,
    const char* ns,
    const char* cls)
{
    CString nameSpace = cimPath.getNameSpace().getString().getCString();
    CString className = cimPath.getClassName().getString().getCString();
    SCMOInstance* scmoRef;

    if (!ns)
    {
        ns = (const char*)nameSpace;
    }
    if (!cls)
    {
        cls = (const char*)className;
    }


    SCMOClass* scmoClass = mbGetSCMOClass(ns,strlen(ns),cls,strlen(cls));

    if (0 != scmoClass)
    {
        scmoRef = new SCMOInstance(*scmoClass, cimPath);
    }
    else
    {
        SCMOClass localDirtySCMOClass(cls,ns);
        scmoRef = new SCMOInstance(localDirtySCMOClass, cimPath);
        scmoRef->markAsCompromised();
    }


    return scmoRef;
}

// Converts SCMOUnion values to CMPIData for keys.
// Includes special handling for the following types:
//  Real32/64 -> Converted to CMPI_String
//  DateTime  -> Converted to CMPI_String
CMPIrc CMPISCMOUtilities::scmoValue2CMPIKeyData(
    const SCMBUnion* scmoValue,
    CMPIType type,
    CMPIData *data)
{
    //Initialize CMPIData object
    data->type = type;
    data->value.uint64 = 0;
    data->state = CMPI_goodValue|CMPI_keyValue;

    //Check for NULL CIMValue
    if( scmoValue == 0 )
    {
        data->state |= CMPI_nullValue;
        return CMPI_RC_OK;
    }

    switch (type)
    {
        case CMPI_uint8:
            data->value.uint64=(CMPIUint64)scmoValue->simple.val.u8;
            data->type=CMPI_keyInteger;
            break;
        case CMPI_uint16:
            data->value.uint64=(CMPIUint64)scmoValue->simple.val.u16;
            data->type=CMPI_keyInteger;
            break;
        case CMPI_uint32:
            data->value.uint64=(CMPIUint64)scmoValue->simple.val.u32;
            data->type=CMPI_keyInteger;
            break;
        case CMPI_uint64:
            data->value.uint64=scmoValue->simple.val.u64;
            data->type=CMPI_keyInteger;
            break;
        case CMPI_sint8:
            data->value.sint64=(CMPISint64)scmoValue->simple.val.s8;
            data->type=CMPI_keyInteger;
            break;
        case CMPI_sint16:
            data->value.sint64=(CMPISint64)scmoValue->simple.val.s16;
            data->type=CMPI_keyInteger;
            break;
        case CMPI_sint32:
            data->value.sint64=(CMPISint64)scmoValue->simple.val.s32;
            data->type=CMPI_keyInteger;
            break;
        case CMPI_sint64:
            data->value.sint64=scmoValue->simple.val.s64;
            data->type=CMPI_keyInteger;
            break;
        case CMPI_char16:
            data->value.uint64=(CMPIUint64)scmoValue->simple.val.c16;
            data->type=CMPI_keyInteger;
            break;
        case CMPI_boolean:
            data->value.boolean=scmoValue->simple.val.bin;
            data->type=CMPI_keyBoolean;
            break;
        case CMPI_real32:
            data->value.real32=(CMPIReal32)scmoValue->simple.val.r32;
            data->type=CMPI_real32;
            break;
        case CMPI_real64:
            data->value.real64=(CMPIReal64)scmoValue->simple.val.r64;
            data->type=CMPI_real64;
            break;

        case CMPI_charsptr:
        case CMPI_chars:
        case CMPI_string:
            {
                if (scmoValue->extString.pchar)
                {
                    data->value.string = reinterpret_cast<CMPIString*>(
                        new CMPI_Object(scmoValue->extString.pchar));
                    data->type = CMPI_string;
                }
                else
                {
                    data->state|=CMPI_nullValue;
                }
                data->type=CMPI_keyString;
                break;
            }

        case CMPI_dateTime:
            {
                CIMDateTime* cimdt =
                    new CIMDateTime(&scmoValue->dateTimeValue);
                data->value.dateTime = reinterpret_cast<CMPIDateTime*>
                    (new CMPI_Object(cimdt));
                data->type=CMPI_dateTime;
                break;
            }

        case CMPI_ref:
            {
                SCMOInstance* ref =
                    new SCMOInstance(*(scmoValue->extRefPtr));
                data->value.ref = reinterpret_cast<CMPIObjectPath*>
                    (new CMPI_Object(
                        ref,
                        CMPI_Object::ObjectTypeObjectPath));
            }
            break;

        default:
            {
                // Not supported for this CMPItype
                data->state = CMPI_badValue;
                return CMPI_RC_ERR_NOT_SUPPORTED;
            }
    }

    if (!(type&CMPI_ENC))
    {
        // For non-encapsulated type simply verify that we have a valid value
        // otherwise set to CMPI_nullValue
        if (!scmoValue->simple.hasValue)
        {
            data->value.uint64 = 0;
            data->state = CMPI_nullValue;
        }
    }

    return CMPI_RC_OK;
}


// Function to convert a SCMO Value into a CMPIData structure
CMPIrc CMPISCMOUtilities::scmoValue2CMPIData(
    const SCMBUnion* scmoValue,
    CMPIType type,
    CMPIData *data,
    Uint32 arraySize)
{
    //Initialize CMPIData object
    data->type = type;
    data->value.uint64 = 0;
    data->state = CMPI_goodValue;

    //Check for NULL CIMValue
    if( scmoValue == 0 )
    {
        data->state = CMPI_nullValue;
        return CMPI_RC_OK;
    }

    if (type & CMPI_ARRAY)
    {
        // Get the type of the element of the CMPIArray
        CMPIType aType = type&~CMPI_ARRAY;

        //Allocate CMPIData array to hold the values
        CMPIData *arrayRoot = new CMPIData[arraySize+1];

        // Advance array pointer to first array element
        CMPIData *aData = arrayRoot;
        aData++;

        // Set the type, state and value of array elements
        for( Uint32 i=0; i<arraySize; i++ )
        {
            CMPIrc rc = scmoValue2CMPIData(&(scmoValue[i]),aType,&(aData[i]));
            if (rc != CMPI_RC_OK)
            {
                return rc;
            }
        }

        // Create array encapsulation object
        arrayRoot->type = aType;
        arrayRoot->value.sint32 = arraySize;
        CMPI_Array *arr = new CMPI_Array(arrayRoot);

        // Set the encapsulated array as data
        data->value.array =
            reinterpret_cast<CMPIArray*>(new CMPI_Object(arr));
    }
    else
    {
        // Check for encpsulated type, which need special handling
        if (type&CMPI_ENC)
        {
            switch (type)
            {
            case CMPI_chars:
            case CMPI_string:
                {
                    if (scmoValue->extString.pchar)
                    {
                        data->value.string = reinterpret_cast<CMPIString*>(
                            new CMPI_Object(scmoValue->extString.pchar));
                        data->type = CMPI_string;
                    }
                    else
                    {
                        data->state=CMPI_nullValue;
                    }
                    break;
                }

            case CMPI_dateTime:
                {
                    CIMDateTime* cimdt =
                        new CIMDateTime(&scmoValue->dateTimeValue);
                    data->value.dateTime = reinterpret_cast<CMPIDateTime*>
                        (new CMPI_Object(cimdt));
                    break;
                }

            case CMPI_ref:
                {
                    SCMOInstance* ref =
                        new SCMOInstance(*(scmoValue->extRefPtr));
                    data->value.ref = reinterpret_cast<CMPIObjectPath*>
                        (new CMPI_Object(
                            ref,
                            CMPI_Object::ObjectTypeObjectPath));
                }
                break;

            case CMPI_instance:
                {
                    SCMOInstance* inst =
                        new SCMOInstance(*(scmoValue->extRefPtr));
                    data->value.inst = reinterpret_cast<CMPIInstance*>
                        (new CMPI_Object(
                            inst,
                            CMPI_Object::ObjectTypeInstance));
                }
                break;
            default:
                {
                    // Not supported for this CMPItype
                    return CMPI_RC_ERR_NOT_SUPPORTED;
                }
            }
        }
        else
        {
            // For non-encapsulated type simply copy the first 64bit
            // of the SCMBUnion to CMPIValue
            if (scmoValue->simple.hasValue)
            {
                data->value.uint64 = scmoValue->simple.val.u64;
            }
            else
            {
                data->value.uint64 = 0;
                data->state = CMPI_nullValue;
            }
        }
    }
    return CMPI_RC_OK;
}

PEGASUS_NAMESPACE_END

