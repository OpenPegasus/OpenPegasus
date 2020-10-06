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

#include "CMPI_String.h"
#include "CMPI_Ftabs.h"

#include <Pegasus/Common/Tracer.h>
#include <string.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

CMPI_String* string2CMPIString(const String &s)
{
    const CString st=s.getCString();
    CMPI_Object *obj= new CMPI_Object((const char*)st);
    return reinterpret_cast<CMPI_String*>(obj);
}

CMPI_String* string2CMPIString(const char* s)
{
    CMPI_Object *obj= new CMPI_Object(s);
    return reinterpret_cast<CMPI_String*>(obj);
}

CMPI_String* string2CMPIString(const char* s, Uint32 len)
{
    CMPI_Object *obj= new CMPI_Object(s,len);
    return reinterpret_cast<CMPI_String*>(obj);
}

extern "C"
{

    PEGASUS_STATIC CMPIStatus stringRelease(CMPIString *eStr)
    {
        char* str=(char*)eStr->hdl;
        if( str )
        {
            free(str);
            (reinterpret_cast<CMPI_Object*>(eStr))->unlinkAndDelete();
            str = NULL;
            CMReturn(CMPI_RC_OK);
        }
        else
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid handle eStr->hdl in \
                CMPI_String:stringRelease");
            CMReturn (CMPI_RC_ERR_INVALID_HANDLE);
        }
    }

    PEGASUS_STATIC CMPIString* stringClone(const CMPIString *eStr,
    CMPIStatus* rc)
    {
        char* str=(char*)eStr->hdl;
        if( !str )
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid handle eStr->hdl in \
                CMPI_String:stringClone");
            CMSetStatus (rc, CMPI_RC_ERR_INVALID_HANDLE);
            return NULL;
        }
        CMPI_Object* obj=new CMPI_Object(str);
        obj->unlink();
        CMSetStatus(rc,CMPI_RC_OK);
        return reinterpret_cast<CMPIString*>(obj);
    }

    PEGASUS_STATIC const char * stringGetCharPtr(const CMPIString *eStr,
    CMPIStatus* rc)
    {
        char* ptr=(char*)eStr->hdl;
        if( !ptr )
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid handle eStr->hdl in \
                CMPI_String:stringGetCharPtr");
            CMSetStatus(rc, CMPI_RC_ERR_INVALID_HANDLE);
            return NULL;
        }
        CMSetStatus(rc,CMPI_RC_OK);
        return ptr;
    }

}
static CMPIStringFT string_FT=
{
    CMPICurrentVersion,
    stringRelease,
    stringClone,
    stringGetCharPtr,
};

CMPIStringFT *CMPI_String_Ftab=&string_FT;


PEGASUS_NAMESPACE_END

