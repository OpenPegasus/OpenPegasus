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
    \file instance.c
    \brief Native CMPIInstance implementation.

    This is the native CMPIInstance implementation as used for remote
    providers. It reflects the well-defined interface of a regular
    CMPIInstance, however, it works independently from the management broker.

    It is part of a native broker implementation that simulates CMPI data
    types rather than interacting with the entities in a full-grown CIMOM.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mm.h"
#include "native.h"


struct native_instance
{
    CMPIInstance instance;
    int mem_state;

    char * classname;
    char * namespace;

    int filtered;
    char ** property_list;
    char ** key_list;

    struct native_property * props;
};

/****************************************************************************/

static void __release_list(char ** list)
{
    if (list)
    {
        char ** tmp = list;

        while (*tmp)
            tool_mm_add(*tmp++);
        tool_mm_add(list);
    }
}


static  char ** __duplicate_list(char ** list, int mem_state)
{
    char ** result = NULL;

    if (list)
    {
        size_t size = 1;
        char ** tmp = (char **)list;

        while (*tmp++)
            ++size;

        result = (char**) tool_mm_alloc(mem_state, size * sizeof(char *));

        for (tmp = result; *list; tmp++)
        {
            *tmp = strdup(*list++);
            if (mem_state == TOOL_MM_ADD)
                tool_mm_add(*tmp);
        }
    }

    return result;
}


static int __contained_list(char ** list, const char * name)
{
    if (list)
    {

        while (*list)
            if (strcmp(*list++, name) == 0)
                return 1;
    }
    return 0;
}


/****************************************************************************/


static CMPIStatus __ift_release(CMPIInstance * instance)
{
    struct native_instance * i = (struct native_instance *) instance;
    CMPIStatus rc = checkArgsReturnStatus(instance);

    if (rc.rc == CMPI_RC_OK && i->mem_state == TOOL_MM_NO_ADD)
    {
        i->mem_state = TOOL_MM_ADD;

        tool_mm_add (i);
        tool_mm_add (i->classname);
        tool_mm_add (i->namespace);

        __release_list(i->property_list);
        __release_list(i->key_list);

        propertyFT.release(i->props);

    }

    return rc;
}


static CMPIInstance * __ift_clone(
    CONST CMPIInstance * instance,
    CMPIStatus * rc)
{
    struct native_instance * i = (struct native_instance *) instance;
    struct native_instance * new;

    if (!checkArgs(instance, rc))
    {
        return 0;
    }

    new = (struct native_instance *)tool_mm_alloc(
        TOOL_MM_NO_ADD,
        sizeof(struct native_instance)
        );
    // Copy CMPIInstance and native_instance extensions.
    new->instance = i->instance;
    new->classname = strdup(i->classname);
    new->namespace = strdup(i->namespace);
    new->property_list = __duplicate_list(i->property_list, TOOL_MM_NO_ADD);
    new->key_list = __duplicate_list(i->key_list, TOOL_MM_NO_ADD);
    new->props = propertyFT.clone(i->props, rc);

    return(CMPIInstance *) new;
}


static CMPIData __ift_getProperty (
    CONST CMPIInstance * instance,
    const char * name,
    CMPIStatus * rc )
{
    struct native_instance * i = (struct native_instance *) instance;

    CMPIData data = checkArgsReturnData(instance, rc);
    if (data.state == CMPI_badValue)
    {
        return data;
    }
    if (!name)
    {
        CMSetStatus(rc, CMPI_RC_ERR_INVALID_PARAMETER);
        return data;
    }

    return propertyFT.getDataProperty(i->props, name, rc);
}


static CMPIData __ift_getPropertyAt(
    CONST CMPIInstance * instance,
    unsigned int index,
    CMPIString ** name,
    CMPIStatus * rc)
{
    struct native_instance * i = (struct native_instance *) instance;

    CMPIData data = checkArgsReturnData(instance, rc);

    if (data.state == CMPI_badValue)
    {
        return data;
    }

    return propertyFT.getDataPropertyAt(i->props, index, name, rc);
}


static unsigned int __ift_getPropertyCount(
    CONST CMPIInstance * instance,
    CMPIStatus * rc)
{
    struct native_instance * i = (struct native_instance *) instance;
    if (!checkArgs(instance, rc))
    {
        return 0;
    }

    return propertyFT.getPropertyCount(i->props, rc);
}


static CMPIStatus __ift_setProperty(
    CONST CMPIInstance * instance,
    const char * name,
    CONST CMPIValue * value,
    CMPIType type)
{
    struct native_instance * i = (struct native_instance *) instance;

    CMPIStatus rc = checkArgsReturnStatus(instance);
    if (rc.rc != CMPI_RC_OK)
    {
        return rc;
    }
    if (!name)
    {
        CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
    }
    if (i->filtered == 0 ||
        i->property_list == NULL ||
        __contained_list ( i->property_list, name ) ||
        __contained_list ( i->key_list, name ))
    {
        if (propertyFT.setProperty(i->props, i->mem_state, name, type, value))
        {
            propertyFT.addProperty(
                &i->props,
                i->mem_state,
                name,
                type,
                0,
                value);
        }
    }
    CMReturn ( CMPI_RC_OK );
}


static CMPIObjectPath * __ift_getObjectPath(
    CONST CMPIInstance * instance,
    CMPIStatus * rc )
{
    int j, f = 0;
    CMPIStatus tmp;
    struct native_instance * i = (struct native_instance *) instance;
    CMPIObjectPath * cop;

    if (!checkArgs(instance, rc))
    {
        return 0;
    }

    cop = native_new_CMPIObjectPath(
        i->namespace,
        i->classname,
        rc );

    if (rc && rc->rc != CMPI_RC_OK)
        return NULL;

    j = propertyFT.getPropertyCount(i->props, NULL);

    while (j--)
    {
        CMPIString * keyName;
        CMPIData d = propertyFT.getDataPropertyAt(i->props, j, &keyName, &tmp);
        if (d.state & CMPI_keyValue)
        {
            CMAddKey(
                cop,
                CMGetCharsPtr(keyName, NULL),
                &d.value,
                d.type);
            f++;
        }
    }
#ifndef CMPI_VER_100
    if (f==0)
    {
        CMPIData d;
        CMPIContext *ctx;
        CMPIBroker *broker;
        CMPIArray *kl;
        unsigned int e, m;

        broker=(CMPIBroker*) tool_mm_get_broker((void**)&ctx);
        kl = broker->eft->getKeyList(broker, ctx, cop, NULL);
        m = kl->ft->getSize(kl, NULL);

        for (e = 0; e < m; e++)
        {
            CMPIString *n = kl->ft->getElementAt(kl, e, NULL).value.string;
            d = propertyFT.getDataProperty(
                 i->props,
                 CMGetCharsPtr(n,NULL),
                 &tmp);
            if (tmp.rc == CMPI_RC_OK)
                CMAddKey(cop, CMGetCharsPtr(n,NULL), &d.value, d.type);
        }
    }
#endif
    return cop;
}


static CMPIStatus __ift_setPropertyFilter(
    CMPIInstance * instance,
    CONST char ** propertyList,
    CONST char ** keys)
{
    struct native_instance * i = (struct native_instance *) instance;

    CMPIStatus rc = checkArgsReturnStatus(instance);

    if (rc.rc != CMPI_RC_OK)
    {
        return rc;
    }

    if (i->filtered && i->mem_state == TOOL_MM_NO_ADD)
    {
        __release_list(i->property_list);
        __release_list(i->key_list);
    }

    i->filtered = 1;
    i->property_list = __duplicate_list((char **)propertyList, i->mem_state);
    i->key_list = __duplicate_list((char **)keys, i->mem_state);

    CMReturn(CMPI_RC_OK);
}

#ifdef CMPI_VER_100
static CMPIStatus __ift_setObjectPath(
    CMPIInstance * instance,
    const CMPIObjectPath *op)
{

    CMPIStatus rc = checkArgsReturnStatus(instance);
    if (rc.rc != CMPI_RC_OK)
    {
        return rc;
    }
    rc = checkArgsReturnStatus(op);
    if (rc.rc != CMPI_RC_OK)
    {
        CMReturn(CMPI_RC_ERR_INVALID_PARAMETER);
    }
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}
#endif

#ifdef CMPI_VER_200
static CMPIStatus __ift_setPropertyWithOrigin(const CMPIInstance * instance,
    const char *name, const CMPIValue *data, const CMPIType type,
    const char *origin)
{
    CMPIStatus rc =  __ift_setProperty(instance, name, data, type);
    struct native_instance *ni = (struct native_instance*)instance;
    if (rc.rc == CMPI_RC_OK && origin)
    {
        propertyFT.setPropertyOrigin(ni->props, name, origin, ni->mem_state);
    }

    return rc;
}
#endif

void add(char **buf, unsigned int *p, unsigned int *m, char *data)
{
    unsigned int ds = (unsigned int) strlen(data) + 1;

    if (*buf == NULL)
    {
        *buf = (char*)malloc(1024);
        *p = 0;
        *m = 1024;
    }
    if ((ds+(*p)) >= *m)
    {
        unsigned nm = *m;
        char *nb;
        while ((ds + (*p)) >= nm)
            nm*=2;
        nb = (char*)malloc(nm);
        memcpy(nb, *buf, *p);
        free(*buf);
        *buf = nb;
        *m = nm;
    }
    memcpy(*buf+(*p), data, ds);
    *p += ds-1;
}


extern char * value2Chars(CMPIType type, CMPIValue * value);

extern CMPIString *__oft_toString(CONST CMPIObjectPath * cop, CMPIStatus *rc);
extern CMPIString * __oft_getClassName(
    CONST CMPIObjectPath * cop,
    CMPIStatus * rc);

CMPIString *instance2String(CONST CMPIInstance *inst, CMPIStatus *rc)
{
    CMPIObjectPath *path;
    CMPIData data;
    CMPIString *name, *ps, *rv;
    unsigned int i, m;
    char *buf = NULL, *v;
    unsigned int bp, bm;

    if (!checkArgs(inst, rc))
    {
        return 0;
    }
    add(&buf, &bp, &bm, "Instance of ");
    path = __ift_getObjectPath(inst, NULL);
    name = __oft_getClassName(path, rc);
    add(&buf, &bp, &bm, (char*)name->hdl);
    add(&buf, &bp, &bm, " {\n");
    ps = __oft_toString(path, rc);
    add(&buf, &bp, &bm, " PATH: ");
    add(&buf, &bp, &bm, (char*)ps->hdl);
    add(&buf, &bp, &bm, "\n");

    for (i = 0, m = __ift_getPropertyCount(inst,rc); i < m; i++)
    {
        data = __ift_getPropertyAt(inst, i, &name, rc);
        add(&buf, &bp, &bm, " ");
        add(&buf, &bp, &bm, (char*)name->hdl);
        add(&buf, &bp, &bm, " = ");
        v = value2Chars(data.type, &data.value);
        add(&buf, &bp, &bm, v);
        free(v);
        add(&buf, &bp, &bm, " ;\n");
    }
    add(&buf, &bp, &bm, "}\n");
    rv = native_new_CMPIString(buf, rc);
    free(buf);
    return rv;
}

static CMPIInstanceFT ift = {
    NATIVE_FT_VERSION,
    __ift_release,
    __ift_clone,
    __ift_getProperty,
    __ift_getPropertyAt,
    __ift_getPropertyCount,
    __ift_setProperty,
    __ift_getObjectPath,
    __ift_setPropertyFilter
#ifdef CMPI_VER_100
    ,__ift_setObjectPath
#endif
#ifdef CMPI_VER_200
    ,__ift_setPropertyWithOrigin
#endif

};

CMPIInstanceFT *CMPI_Instance_FT = &ift;


CMPIInstance * native_new_CMPIInstance(
    CONST CMPIObjectPath * cop,
    CMPIStatus * rc)
{
    static CMPIInstance i = {
        "CMPIInstance",
        &ift
    };

    struct native_instance * instance =
        (struct native_instance *)
        tool_mm_alloc ( TOOL_MM_ADD, sizeof ( struct native_instance ) );

    CMPIStatus tmp1, tmp2, tmp3;
    CMPIString * str;

    int j = CMGetKeyCount(cop, &tmp1);

    instance->instance = i;
    instance->mem_state = TOOL_MM_ADD;

    str = CMGetClassName(cop, &tmp2);
    instance->classname = (char*)CMGetCharsPtr(str, NULL);
    str = CMGetNameSpace(cop, &tmp3);
    instance->namespace = (char*)CMGetCharsPtr(str, NULL);

    if (tmp1.rc != CMPI_RC_OK ||
        tmp2.rc != CMPI_RC_OK ||
        tmp3.rc != CMPI_RC_OK)
    {
        CMSetStatus(rc, CMPI_RC_ERR_FAILED);
    }
    else
    {
        while (j-- && ( tmp1.rc == CMPI_RC_OK ))
        {
            CMPIString * keyName;
            CMPIData tmp = CMGetKeyAt(cop, j, &keyName, &tmp1);

            propertyFT.addProperty(
                &instance->props,
                TOOL_MM_ADD,
                CMGetCharsPtr(keyName, NULL),
                tmp.type,
                tmp.state,
                &tmp.value);
        }

        CMSetStatus(rc, tmp1.rc);
    }

    return(CMPIInstance *) instance;
}

