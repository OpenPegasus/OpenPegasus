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

#include "CMPI_Object.h"
#include "CMPI_ThreadContext.h"
#include "CMPI_Ftabs.h"

#include <string.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

CMPI_Object::CMPI_Object(CMPI_Object *obj)
{
    hdl = obj->hdl;
    ftab = obj->ftab;
}
// Add a flag here?
CMPI_Object::CMPI_Object(CIMInstance* ci)
{
    CMPI_ThreadContext::addObject(this);
    hdl = (void*)ci;
    ftab = CMPI_Instance_Ftab;
}

CMPI_Object::CMPI_Object(SCMOInstance* si, SCMOInstanceObjectType type)
{
    CMPI_ThreadContext::addObject(this);
    hdl = (void*)si;
    if (type == ObjectTypeInstance)
    {
        ftab = CMPI_Instance_Ftab;
    }
    else
    {
        ftab = CMPI_ObjectPath_Ftab;
    }
}

CMPI_Object::CMPI_Object(CIMObjectPath* cop)
{
    CMPI_ThreadContext::addObject(this);
    hdl = (void*)cop;
    ftab = CMPI_ObjectPath_Ftab;
}

CMPI_Object::CMPI_Object(CIMDateTime* cdt)
{
    CMPI_ThreadContext::addObject(this);
    hdl = (void*)cdt;
    ftab = CMPI_DateTime_Ftab;
}

CMPI_Object::CMPI_Object(CIMError* cer)
{
    CMPI_ThreadContext::addObject(this);
    hdl = (void*)cer;
    ftab = CMPI_Error_Ftab;
}

CMPI_Object::CMPI_Object(const String& str)
{
    CMPI_ThreadContext::addObject(this);
    const CString st = str.getCString();
    hdl = (void*)strdup((const char*)st);
    ftab = CMPI_String_Ftab;
}

CMPI_Object::CMPI_Object(const char *str)
{
    CMPI_ThreadContext::addObject(this);
    hdl = str ? (void*)strdup(str) : (void*)strdup("");
    ftab = CMPI_String_Ftab;
}

CMPI_Object::CMPI_Object(const char *str, Uint32 len)
{
    CMPI_ThreadContext::addObject(this);
    char * newStr = (char*)malloc(len+1);
    if (0!=str)
    {
        memcpy(newStr, str, len);
    }
    newStr[len]='\0';
    hdl = newStr;
    ftab = CMPI_String_Ftab;
}

CMPI_Object::CMPI_Object(Array<CIMParamValue> *args)
{
    CMPI_ThreadContext::addObject(this);
    hdl = (void*)args;
    ftab = CMPI_Args_Ftab;
}

CMPI_Object::CMPI_Object(CMPI_Array *arr)
{
    CMPI_ThreadContext::addObject(this);
    hdl = (void*)arr;
    ftab = CMPI_Array_Ftab;
}

CMPI_Object::CMPI_Object(CMPISelectCond *dta)
{
    CMPI_ThreadContext::addObject(this);
    hdl = (void*)dta;
    ftab = CMPI_SelectCond_Ftab;
}

CMPI_Object::CMPI_Object(CMPISubCond *dta)
{
    CMPI_ThreadContext::addObject(this);
    hdl = (void*)dta;
    ftab = CMPI_SubCond_Ftab;
}

CMPI_Object::CMPI_Object(CMPIPredicate *dta)
{
    CMPI_ThreadContext::addObject(this);
    hdl = (void*)dta;
    ftab = CMPI_Predicate_Ftab;
}

CMPI_Object::CMPI_Object(CMPI_ObjEnumeration *dta)
{
    CMPI_ThreadContext::addObject(this);
    hdl = (void*)dta;
    ftab = CMPI_ObjEnumeration_Ftab;
}

CMPI_Object::CMPI_Object(CMPI_InstEnumeration *dta)
{
    CMPI_ThreadContext::addObject(this);
    hdl = (void*)dta;
    ftab = CMPI_InstEnumeration_Ftab;
}

CMPI_Object::CMPI_Object(CMPI_OpEnumeration *dta)
{
    CMPI_ThreadContext::addObject(this);
    hdl = (void*)dta;
    ftab = CMPI_OpEnumeration_Ftab;
}

void CMPI_Object::unlinkAndDelete()
{
    CMPI_ThreadContext::remObject(this);
    delete this;
};

void CMPI_Object::unlink()
{
    CMPI_ThreadContext::remObject(this);
};

PEGASUS_NAMESPACE_END




