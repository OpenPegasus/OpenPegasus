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

/*!
    \file native.h
    \brief Header file for the native encapsulated CMPI data type
    implementation.

    This file defines all the data types and functions necessary to use
    native encapsulated CMPI data objects. These are clones of the regular
    CMPI data types like CMPIObjectPath, CMPIInstance etc., however, they
    can be instantiated and manipulated without a full blown CIMOM. Instead,
    they use an autononmous CIMOM clone that provides all the functions
    to create these objects as defined by the CMPIBrokerEncFT.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mm.h"
#include "native.h"


struct native_cop
{
    CMPIObjectPath cop;
    int mem_state;

    char * namespace;
    char * classname;
    char *hostname;
    struct native_property * keys;
};


static struct native_cop * __new_empty_cop(
    int,
    const char *,
    const char *,
    const char*,
    CMPIStatus * );

/****************************************************************************/


static CMPIStatus __oft_release(CMPIObjectPath * cop)
{
    struct native_cop * o = (struct native_cop *) cop;
    CMPIStatus rc = checkArgsReturnStatus(cop);


    if (rc.rc == CMPI_RC_OK && o->mem_state == TOOL_MM_NO_ADD)
    {

        o->mem_state = TOOL_MM_ADD;
        tool_mm_add(o);
        tool_mm_add(o->classname);
        tool_mm_add(o->namespace);
        tool_mm_add(o->hostname);
        propertyFT.release(o->keys);

    }
    return rc;
}


static CMPIObjectPath * __oft_clone(CONST CMPIObjectPath * cop, CMPIStatus * rc)
{
    CMPIStatus tmp;
    struct native_cop * o = (struct native_cop *) cop;
    struct native_cop *new;

    if (!checkArgs(cop, rc))
    {
        return 0;
    }
    new  = __new_empty_cop(
        TOOL_MM_NO_ADD,
        o->namespace,
        o->classname,
        o->hostname,
        &tmp);

    if (tmp.rc == CMPI_RC_OK)
    {
        new->keys = propertyFT.clone(o->keys, rc);
    }
    else
    {
        CMSetStatus(rc, tmp.rc);
    }

    return(CMPIObjectPath *) new;
}


static CMPIStatus __oft_setNameSpace(
    CMPIObjectPath * cop,
    const char * namespace)
{
    struct native_cop * o = (struct native_cop *) cop;

    char * ns;
    CMPIStatus rc = checkArgsReturnStatus(cop);
    if (rc.rc != CMPI_RC_OK)
    {
        return rc;
    }

    ns = (namespace) ? strdup(namespace) : NULL;

    if (o->mem_state == TOOL_MM_NO_ADD)
    {
        free(o->namespace);
    }
    else
    {
        tool_mm_add(ns);
    }

    o->namespace = ns;
    CMReturn(CMPI_RC_OK);
}


static CMPIString * __oft_getNameSpace(
    CONST CMPIObjectPath * cop,
    CMPIStatus * rc)
{
    struct native_cop * o = (struct native_cop *) cop;

    if (!checkArgs(cop, rc))
    {
        return 0;
    }
    return native_new_CMPIString(o->namespace, rc);
}


static CMPIStatus __oft_setHostName(CMPIObjectPath * cop, const char * hostname)
{
    struct native_cop * o = (struct native_cop *) cop;
    char * hn;

    CMPIStatus rc = checkArgsReturnStatus(cop);

    if (rc.rc != CMPI_RC_OK)
    {
        return rc;
    }

    hn = (hostname) ? strdup(hostname) : NULL;

    if (o->mem_state == TOOL_MM_NO_ADD)
    {
        free(o->hostname);
    }
    else
    {
        tool_mm_add(hn);
    }
    o->hostname = hn;
    CMReturn(CMPI_RC_OK);
}


static CMPIString * __oft_getHostName(
    CONST CMPIObjectPath * cop,
    CMPIStatus * rc)
{
    struct native_cop * o = (struct native_cop *) cop;
    if (!checkArgs(cop, rc))
    {
        return 0;
    }

    return native_new_CMPIString(o->hostname, rc);
}


static CMPIStatus __oft_setClassName(
    CMPIObjectPath * cop,
    const char * classname)
{
    struct native_cop * o = (struct native_cop *) cop;

    char * cn;
    CMPIStatus rc = checkArgsReturnStatus(cop);

    if (rc.rc != CMPI_RC_OK)
    {
        return rc;
    }

    cn = (classname) ? strdup(classname) : NULL;

    if (o->mem_state == TOOL_MM_NO_ADD)
    {
        free(o->classname);
    }
    else
    {
        tool_mm_add(cn);
    }

    o->classname = cn;
    CMReturn(CMPI_RC_OK);
}


CMPIString * __oft_getClassName(
    CONST CMPIObjectPath * cop,
    CMPIStatus * rc)
{
    struct native_cop * o = (struct native_cop *) cop;

    if (!checkArgs(cop, rc))
    {
        return 0;
    }
    return native_new_CMPIString(o->classname, rc);
}


static CMPIStatus __oft_addKey(
    CMPIObjectPath * cop,
    const char * name,
    CONST CMPIValue * value,
    CONST CMPIType type)
{
    struct native_cop * o = (struct native_cop *) cop;

    CMPIStatus rc = checkArgsReturnStatus(cop);
    if (rc.rc != CMPI_RC_OK)
    {
        return rc;
    }

    CMReturn(
        (propertyFT.addProperty(
        &o->keys,
        o->mem_state,
        name,
        type,
        CMPI_keyValue,
        value))
        ? CMPI_RC_ERR_ALREADY_EXISTS : CMPI_RC_OK);
}


static CMPIData __oft_getKey(
    CONST CMPIObjectPath * cop,
    const char * name,
    CMPIStatus * rc)
{
    struct native_cop * o = (struct native_cop *) cop;
    CMPIData data = checkArgsReturnData(cop, rc);

    if (data.state == CMPI_badValue)
    {
        return data;
    }

    return propertyFT.getDataProperty(o->keys, name, rc);
}


static CMPIData __oft_getKeyAt(
    CONST CMPIObjectPath * cop,
    unsigned int index,
    CMPIString ** name,
    CMPIStatus * rc)
{
    struct native_cop * o = (struct native_cop *) cop;
    CMPIData data = checkArgsReturnData(cop, rc);

    if (data.state == CMPI_badValue)
    {
        return data;
    }

    return propertyFT.getDataPropertyAt(o->keys, index, name, rc);
}


static unsigned int __oft_getKeyCount(
    CONST CMPIObjectPath * cop,
    CMPIStatus * rc)
{
    struct native_cop * o = (struct native_cop *) cop;

    if (!checkArgs(cop, rc))
    {
        return 0;
    }

    return propertyFT.getPropertyCount(o->keys, rc);
}


static CMPIStatus __oft_setNameSpaceFromObjectPath(
    CMPIObjectPath * cop,
    CONST CMPIObjectPath * src)
{
    struct native_cop * s = (struct native_cop *) src;
    CMPIStatus rc = checkArgsReturnStatus(cop);

    if (rc.rc != CMPI_RC_OK)
    {
        return rc;
    }

    rc = checkArgsReturnStatus(src);
    if (rc.rc != CMPI_RC_OK)
    {
        return rc;
    }

    return __oft_setNameSpace(cop, s->namespace);
}


static CMPIStatus __oft_setHostAndNameSpaceFromObjectPath(
    CMPIObjectPath * cop,
    CONST CMPIObjectPath * src)
{
    CMPIStatus rc;
    struct native_cop * s = (struct native_cop *) src;

    rc = checkArgsReturnStatus(cop);
    if (rc.rc != CMPI_RC_OK)
    {
        return rc;
    }

    rc = checkArgsReturnStatus(src);
    if (rc.rc != CMPI_RC_OK)
    {
        return rc;
    }

    rc =  __oft_setNameSpace(cop, s->namespace);
    if (rc.rc == CMPI_RC_OK)
    {
        return __oft_setHostName(cop, s->hostname);
    }

    return rc;
}

extern char * value2Chars(CMPIType type, CMPIValue * value);

// Returnc CMPIObjectPath as CMPIString in format:
//         "//atp:9999/root/cimv25:TennisPlayer.first="Patrick",last="Rafter";
CMPIString *__oft_toString( CONST CMPIObjectPath * cop, CMPIStatus *rc)
{
    char str[2048] = {0};
    CMPIString *ns;
    CMPIString *cn;
    CMPIString *hn;
    CMPIString *name;
    CMPIData data;
    unsigned int i, m;
    char *v;

    if (!checkArgs(cop, rc))
    {
        return 0;
    }
    hn = __oft_getHostName(cop, rc);
    if (hn && hn->hdl)
    {
        strcat(str, (char*)hn->hdl);
        strcat(str, ":");
    }
    ns = __oft_getNameSpace(cop, rc);
    if (ns && ns->hdl)
    {
        strcat(str, (char*)ns->hdl);
        strcat(str, ":");
    }
    cn = __oft_getClassName(cop, rc);
    strcat(str, (char*)cn->hdl);
    for (i = 0, m = __oft_getKeyCount(cop, rc); i < m; i++)
    {
        data = __oft_getKeyAt(cop, i, &name, rc);
        if (i) strcat(str, ",");
        else strcat(str, ".");
        strcat(str, (char*)name->hdl);
        strcat(str, "=");
        v = value2Chars(data.type, &data.value);
        strcat(str, v);
        free(v);
    };
    return native_new_CMPIString(str, rc);
}

static CMPIObjectPathFT oft = {
    NATIVE_FT_VERSION,
    __oft_release,
    __oft_clone,
    __oft_setNameSpace,
    __oft_getNameSpace,
    __oft_setHostName,
    __oft_getHostName,
    __oft_setClassName,
    __oft_getClassName,
    __oft_addKey,
    __oft_getKey,
    __oft_getKeyAt,
    __oft_getKeyCount,
    __oft_setNameSpaceFromObjectPath,
    __oft_setHostAndNameSpaceFromObjectPath,
    NULL,
    NULL,
    NULL,
    NULL,
    __oft_toString
};

CMPIObjectPathFT *CMPI_ObjectPath_FT=&oft;

static struct native_cop * __new_empty_cop(
    int mm_add,
    const char * namespace,
    const char * classname,
    const char *hostname,
    CMPIStatus * rc)
{
    static CMPIObjectPath o = {
        "CMPIObjectPath",
        &oft
    };

    struct native_cop * cop =
        (struct native_cop *)tool_mm_alloc(mm_add, sizeof(struct native_cop));

    cop->cop = o;
    cop->mem_state = mm_add;
    cop->classname = (classname) ? strdup(classname) : NULL;
    cop->namespace = (namespace) ? strdup(namespace) : NULL;
    cop->hostname = (hostname) ? strdup(hostname) : NULL;
    if (mm_add == TOOL_MM_ADD)
    {
        tool_mm_add(cop->classname);
        tool_mm_add(cop->namespace);
        tool_mm_add(cop->hostname);
    }

    CMSetStatus(rc, CMPI_RC_OK);
    return cop;
}


CMPIObjectPath * native_new_CMPIObjectPath(
    const char * namespace,
    const char * classname,
    CMPIStatus * rc)
{
    return(CMPIObjectPath *)__new_empty_cop(
        TOOL_MM_ADD,
        namespace,
        classname,
        NULL,
        rc);
}

