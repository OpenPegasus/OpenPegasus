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

#include <Pegasus/Provider/CMPI/cmpimacs.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#ifndef CMPI_PLATFORM_WIN32_IX86_MSVC
#include <unistd.h>
#endif

#include "cmpiUtilLib.h"

#define _LogExtension ".log"


/* --------------------------------------------------------------------------*/
/* --------------------------------------------------------------------------*/
static FILE *fd = NULL;

const CMPIBoolean CMPI_true = 1;
const CMPIBoolean CMPI_false = 0;

void PROV_LOG (const char *fmt, ...)
{

    va_list ap;
    if (!fd)
        fd = stderr;

    fprintf (fd, " ");
    va_start (ap, fmt);
    vfprintf (fd, fmt, ap);
    va_end (ap);

    fprintf (fd, "\n");
    fflush (fd);
}

void PROV_LOG_CLOSE ()
{
    if (fd != stderr)
        fclose (fd);
    fd = stderr;
}

void PROV_LOG_OPEN (const char *file, const char *location)
{
    char *path = NULL;
    const char *env;
    size_t i = 0;
    size_t j = 0;
    size_t len = strlen (file);
    size_t env_len = 0;
    size_t loc_len = strlen (location);
    size_t ext_len = strlen (_LogExtension);

    env = PEGASUS_ROOT;
    if (env)
        env_len = strlen (env);

    path = (char *) malloc (env_len + len + loc_len + ext_len);

    strncpy (path, env, env_len);

    path[env_len] = 0;
    strncat (path, location, loc_len);
    for (i = 0; i < len; i++)
        /* Only use good names. */
        if (isalpha (file[i]))
        {
            path[j + env_len + loc_len] = file[i];
            j++;
        }
    path[j + env_len + loc_len] = 0;
    strncat (path, _LogExtension, ext_len);
    path[j + env_len + loc_len + ext_len] = 0;

    fd = fopen (path, "a+");
    if (fd == NULL)
        fd = stderr;
    free (path);
}

/* --------------------------------------------------------------------------*/
/*                       Helper functions                        */
/* --------------------------------------------------------------------------*/


const char * strCMPIValueState (CMPIValueState state)
{
    switch (state)
    {
        case CMPI_goodValue:
            return "CMPI_goodValue";
        case CMPI_nullValue:
            return "CMPI_nullValue";
        case CMPI_keyValue:
            return "CMPI_keyValue";
        case CMPI_notFound:
            return "CMPI_notFound";
        case CMPI_badValue:
            return "CMPI_badValue";
        default:
            return "Unknown state";

    }

}

const char * strCMPIPredOp (CMPIPredOp op)
{
    switch (op)
    {
        case CMPI_PredOp_Equals:
            return " CMPI_PredOp_Equals ";
        case CMPI_PredOp_NotEquals:
            return " CMPI_PredOp_NotEquals ";
        case CMPI_PredOp_LessThan:
            return " CMPI_PredOp_LessThan ";
        case CMPI_PredOp_GreaterThanOrEquals:
            return " CMPI_PredOp_GreaterThanOrEquals ";
        case CMPI_PredOp_GreaterThan:
            return " CMPI_PredOp_GreaterThan ";
        case CMPI_PredOp_LessThanOrEquals:
            return " CMPI_PredOp_LessThanOrEquals ";
        case CMPI_PredOp_Isa:
            return " CMPI_PredOp_Isa ";
        case CMPI_PredOp_NotIsa:
            return " CMPI_PredOp_NotIsa ";
        case CMPI_PredOp_Like:
            return " CMPI_PredOp_Like ";
        case CMPI_PredOp_NotLike:
            return " CMPI_PredOp_NotLike ";
        default:
            return "Unknown operation";
    }
}


const char * strCMPIStatus (CMPIStatus stat)
{
    return _CMPIrcName(stat.rc);
}
/* Compare two CIM types to see if they are identical */
int _CMSameType( CMPIData value1, CMPIData value2 )
{
    return(value1.type == value2.type);
}


// ----------------------------------------------------------------------------


/* Compare two CIM data values to see if they are identical */
int _CMSameValue( CMPIData value1, CMPIData value2 )
{
    /* Return status of CIM operations */
    CMPIStatus status = {CMPI_RC_OK, NULL};
    CMPIValue v1 = value1.value;
    CMPIValue v2 = value2.value;

    /* Check if the type of the two CIM values is the same */
    if (!_CMSameType(value1, value2)) return 0;

    /* Check if the value of the two CIM values is the same */
    switch (value1.type)
    {
        case CMPI_string: {
                if (CMIsNullObject(v1.string) || CMIsNullObject(v2.string))
                    return 0;
                return(0 == strcmp(CMGetCharsPtr(v1.string,NULL),
                                   CMGetCharsPtr(v2.string,NULL)));
            }
        case CMPI_dateTime: {
                /* Compare dateTime's using their binary representation */
                CMPIUint64 dateTime1, dateTime2;
                if (CMIsNullObject(v1.dateTime) || CMIsNullObject(v2.dateTime))
                    return 0;
                dateTime1 = CMGetBinaryFormat(v1.dateTime, &status);
                if (status.rc != CMPI_RC_OK) return 0;
                dateTime2 = CMGetBinaryFormat(v2.dateTime, &status);
                if (status.rc != CMPI_RC_OK) return 0;
                return(dateTime1 == dateTime2);
            }
        case CMPI_boolean:
            return(v1.boolean == v2.boolean);
        case CMPI_char16:
            return(v1.char16 == v2.char16);
        case CMPI_uint8:
            return(v1.uint8 == v2.uint8);
        case CMPI_sint8:
            return(v1.sint8 == v2.sint8);
        case CMPI_uint16:
            return(v1.uint16 == v2.uint16);
        case CMPI_sint16:
            return(v1.sint16 == v2.sint16);
        case CMPI_uint32:
            return(v1.uint32 == v2.uint32);
        case CMPI_sint32:
            return(v1.sint32 == v2.sint32);
        case CMPI_uint64:
            return(v1.uint64 == v2.uint64);
        case CMPI_sint64:
            return(v1.sint64 == v2.sint64);
        case CMPI_real32:
            return(v1.real32 == v2.real32);
        case CMPI_real64:
            return(v1.real64 == v2.real64);
    }
    return 0;
}


// ----------------------------------------------------------------------------


/* Compare two CIM object paths to see if they are identical */
int _CMSameObject(const CMPIObjectPath * object1,
                  const CMPIObjectPath * object2 )
{
    /* Return status of CIM operations */
    CMPIStatus status = {CMPI_RC_OK, NULL};
    CMPIString * namespace1, * namespace2;
    CMPIString * classname1, * classname2;
    CMPIData key1, key2;
    CMPIString * keyname = NULL;
    int numkeys1, numkeys2, i;

    /* Check if the two object paths have the same namespace */
    namespace1 = CMGetNameSpace(object1, &status);
    if ((status.rc != CMPI_RC_OK) || CMIsNullObject(namespace1))
        return 0;
    namespace2 = CMGetNameSpace(object2, &status);
    if ((status.rc != CMPI_RC_OK) || CMIsNullObject(namespace2))
        return 0;
    if (strcmp(
        CMGetCharsPtr(namespace1,NULL),
        CMGetCharsPtr(namespace2,NULL)) != 0)
    {
        return 0;
    }

    /* Check if the two object paths have the same class */
    classname1 = CMGetClassName(object1, &status);
    if ((status.rc != CMPI_RC_OK) || CMIsNullObject(classname1))
        return 0;
    classname2 = CMGetClassName(object2, &status);
    if ((status.rc != CMPI_RC_OK) || CMIsNullObject(classname2))
        return 0;
    if (strcmp(
        CMGetCharsPtr(classname1,NULL),
        CMGetCharsPtr(classname2,NULL)) != 0)
    {
        return 0;
    }
    /* Check if the two object paths have the same number of keys */
    numkeys1 = CMGetKeyCount(object1, &status);
    if (status.rc != CMPI_RC_OK) return 0;
    numkeys2 = CMGetKeyCount(object2, &status);
    if (status.rc != CMPI_RC_OK) return 0;
    if (numkeys1 != numkeys2) return 0;

    /* Go through the keys for the 1st object path and */
    /* compare to the 2nd object path */
    for (i=0; i<numkeys1; i++)
    {
        /* Retrieve the same key from both object paths */
        key1 = CMGetKeyAt(object1, i, &keyname, &status);
        if ((status.rc != CMPI_RC_OK) || CMIsNullObject(keyname))
            return 0;
        key2 = CMGetKey(object2, CMGetCharsPtr(keyname,NULL), &status);
        if (status.rc != CMPI_RC_OK) return 0;

        /* Check if both keys are not nullValue and have the same value */
        if (CMIsNullValue(key1) ||
            CMIsNullValue(key2) ||
            !_CMSameValue(key1,key2))
        {
            return 0;
        }
    }

    /* If got here then everything matched! */
    return 1;
}


/*--------------------------------------------------------------------------*/


/* Compare two CIM instances to see if they are identical */
int _CMSameInstance( const CMPIInstance * instance1,
                     const CMPIInstance * instance2 )
{
    /* Return status of CIM operations */
    CMPIStatus status = {CMPI_RC_OK, NULL};
    CMPIObjectPath * objectpath1, * objectpath2;
    CMPIData property1, property2;
    CMPIString * propertyname = NULL;
    int numproperties1, numproperties2, i;

    /* Check that the two instances have the same object path */
    objectpath1 = CMGetObjectPath(instance1, &status);
    if ((status.rc != CMPI_RC_OK) || CMIsNullObject(objectpath1))
        return 0;
    objectpath2 = CMGetObjectPath(instance2, &status);
    if ((status.rc != CMPI_RC_OK) || CMIsNullObject(objectpath2))
        return 0;
    if (!_CMSameObject(objectpath1, objectpath2))
        return 0;

    /* Check if the two instances have the same number of properties */
    numproperties1 = CMGetPropertyCount(instance1, &status);
    if (status.rc != CMPI_RC_OK) return 0;
    numproperties2 = CMGetPropertyCount(instance2, &status);
    if (status.rc != CMPI_RC_OK) return 0;
    if (numproperties1 != numproperties2) return 0;

    /* Go through the properties for the 1st instance and */
    /* compare to the 2nd instance */
    for (i=0; i<numproperties1; i++)
    {
        /* Retrieve the same property from both instances */
        property1 = CMGetPropertyAt(instance1, i, &propertyname, &status);
        if ((status.rc != CMPI_RC_OK) || CMIsNullObject(propertyname))
            return 0;
        property2 = CMGetProperty(instance2,
                                  CMGetCharsPtr(propertyname,NULL),
                                  &status);

        if (status.rc != CMPI_RC_OK) return 0;

        /* Check if both properties are not nullValue */
        /* and have the same value */
        if (CMIsNullValue(property1) ||
            CMIsNullValue(property2) ||
            !_CMSameValue(property1,property2))
        {
                return 0;
        }
    }

    /* If got here then everything matched! */
    return 1;
}


// ----------------------------------------------------------------------------


/* Get string name of CMPIStatus return code */
const char * _CMPIrcName ( CMPIrc rc )
{
    switch (rc)
    {
        case CMPI_RC_OK:
            return "CMPI_RC_OK";
        case CMPI_RC_ERR_FAILED:
            return "CMPI_RC_ERR_FAILED";
        case CMPI_RC_ERR_ACCESS_DENIED:
            return "CMPI_RC_ERR_ACCESS_DENIED";
        case CMPI_RC_ERR_INVALID_NAMESPACE:
            return "CMPI_RC_ERR_INVALID_NAMESPACE";
        case CMPI_RC_ERR_INVALID_PARAMETER:
            return "CMPI_RC_ERR_INVALID_PARAMETER";
        case CMPI_RC_ERR_INVALID_CLASS:
            return "CMPI_RC_ERR_INVALID_CLASS";
        case CMPI_RC_ERR_NOT_FOUND:
            return "CMPI_RC_ERR_NOT_FOUND";
        case CMPI_RC_ERR_NOT_SUPPORTED:
            return "CMPI_RC_ERR_NOT_SUPPORTED";
        case CMPI_RC_ERR_CLASS_HAS_CHILDREN:
            return "CMPI_RC_ERR_CLASS_HAS_CHILDREN";
        case CMPI_RC_ERR_CLASS_HAS_INSTANCES:
            return "CMPI_RC_ERR_CLASS_HAS_INSTANCES";
        case CMPI_RC_ERR_INVALID_SUPERCLASS:
            return "CMPI_RC_ERR_INVALID_SUPERCLASS";
        case CMPI_RC_ERR_ALREADY_EXISTS:
            return "CMPI_RC_ERR_ALREADY_EXISTS";
        case CMPI_RC_ERR_NO_SUCH_PROPERTY:
            return "CMPI_RC_ERR_NO_SUCH_PROPERTY";
        case CMPI_RC_ERR_TYPE_MISMATCH:
            return "CMPI_RC_ERR_TYPE_MISMATCH";
        case CMPI_RC_ERR_QUERY_LANGUAGE_NOT_SUPPORTED:
            return "CMPI_RC_ERR_QUERY_LANGUAGE_NOT_SUPPORTED";
        case CMPI_RC_ERR_INVALID_QUERY:
            return "CMPI_RC_ERR_INVALID_QUERY";
        case CMPI_RC_ERR_METHOD_NOT_AVAILABLE:
            return "CMPI_RC_ERR_METHOD_NOT_AVAILABLE";
        case CMPI_RC_ERR_METHOD_NOT_FOUND:
            return "CMPI_RC_ERR_METHOD_NOT_FOUND";
        case CMPI_RC_NO_MORE_ELEMENTS:
            return "CMPI_RC_NO_MORE_ELEMENTS";
        case CMPI_RC_ERR_INVALID_HANDLE:
            return "CMPI_RC_ERR_INVALID_HANDLE";
        case CMPI_RC_ERROR_SYSTEM:
            return "CMPI_RC_ERROR_SYSTEM";
        case CMPI_RC_ERROR:
            return "CMPI_RC_ERROR";
        case CMPI_RC_ERR_INVALID_DATA_TYPE:
            return "CMPI_RC_ERR_INVALID_DATA_TYPE";
        default:
            return "Unknown";
    }
}


/*---------------------------------------------------------------------------*/


/* Get string name of CMPIType */
const char * _CMPITypeName (CMPIType type)
{
    switch (type)
    {
        case CMPI_null:
            return "CMPI_null";
        case CMPI_boolean:
            return "CMPI_boolean";
        case CMPI_char16:
            return "CMPI_char16";
        case CMPI_real32:
            return "CMPI_real32";
        case CMPI_real64:
            return "CMPI_real64";
        case CMPI_uint8:
            return "CMPI_uint8";
        case CMPI_uint16:
            return "CMPI_uint16";
        case CMPI_uint32:
            return "CMPI_uint32";
        case CMPI_uint64:
            return "CMPI_uint64";
        case CMPI_sint8:
            return "CMPI_sint8";
        case CMPI_sint16:
            return "CMPI_sint16";
        case CMPI_sint32:
            return "CMPI_sint32";
        case CMPI_sint64:
            return "CMPI_sint64";
        case CMPI_instance:
            return "CMPI_instance";
        case CMPI_ref:
            return "CMPI_ref";
        case CMPI_args:
            return "CMPI_args";
        case CMPI_class:
            return "CMPI_class";
        case CMPI_filter:
            return "CMPI_filter";
        case CMPI_enumeration:
            return "CMPI_enumeration";
        case CMPI_string:
            return "CMPI_string";
        case CMPI_chars:
            return "CMPI_chars";
        case CMPI_dateTime:
            return "CMPI_dateTime";
        case CMPI_ptr:
            return "CMPI_ptr";
        case CMPI_charsptr:
            return "CMPI_charsptr";
        case CMPI_ARRAY:
            return "CMPI_ARRAY";

        default:
            return "Unknown";
    }
}


/*--------------------------------------------------------------------------*/


/* Get string representation of CMPIData value */
/* Note - Caller *MUST* free this string when done with it */
char * _CMPIValueToString (CMPIData data)
{
    /* TODO - make better effort to determine size of formatted text values */
    char * valuestring = NULL;
    const char * str = NULL;
    CMPIString * datetimestr = NULL;

    if (CMIsNullValue(data)) return NULL;

    switch (data.type)
    {
        case CMPI_char16:
            valuestring = malloc(100*sizeof(char));
            sprintf(valuestring, "0x%04X", data.value.char16);
            return valuestring;
        case CMPI_sint8:
            valuestring = malloc(100*sizeof(char));
            sprintf(valuestring, "%d", (int)data.value.sint8);
            return valuestring;
        case CMPI_uint8:
            valuestring = malloc(100*sizeof(char));
            sprintf(valuestring, "%u", (unsigned int)data.value.uint8);
            return valuestring;
        case CMPI_sint16:
            valuestring = malloc(100*sizeof(char));
            sprintf(valuestring, "%hd", data.value.sint16);
            return valuestring;
        case CMPI_uint16:
            valuestring = malloc(100*sizeof(char));
            sprintf(valuestring, "%hu", data.value.uint16);
            return valuestring;
        case CMPI_sint32:
            valuestring = malloc(100*sizeof(char));
            sprintf(valuestring, "%d", data.value.sint32);
            return valuestring;
        case CMPI_uint32:
            valuestring = malloc(100*sizeof(char));
            sprintf(valuestring, "%u", data.value.uint32);
            return valuestring;
        case CMPI_sint64:
            valuestring = malloc(100*sizeof(char));
            sprintf(valuestring,
                    "%" PEGASUS_64BIT_CONVERSION_WIDTH "d",
                    data.value.sint64);
            return valuestring;
        case CMPI_uint64:
            valuestring = malloc(100*sizeof(char));
            sprintf(valuestring,
                    "%" PEGASUS_64BIT_CONVERSION_WIDTH "u",
                    data.value.uint64);
            return valuestring;
        case CMPI_string: {
                if (CMIsNullObject(data.value.string)) return NULL;
                str = CMGetCharsPtr(data.value.string,NULL);
                if (str == NULL) return NULL;
                valuestring = strdup(str);
                return valuestring;
            }
        case CMPI_boolean:
            valuestring = malloc(100*sizeof(char));
            sprintf(valuestring, "%s", (data.value.boolean)? "TRUE":"FALSE");
            return valuestring;
        case CMPI_real32:
            valuestring = malloc(100*sizeof(char));
            sprintf(valuestring, "%.6f", (double)data.value.real32);
            return valuestring;
        case CMPI_real64:
            valuestring = malloc(100*sizeof(char));
            sprintf(valuestring, "%.12f", (double)data.value.real64);
            return valuestring;
        case CMPI_dateTime: {
                CMPIStatus status = {CMPI_RC_OK, NULL};
                if (CMIsNullObject(data.value.dateTime)) return NULL;
                datetimestr = CMGetStringFormat(data.value.dateTime, &status);
                if ((status.rc != CMPI_RC_OK) || CMIsNullObject(datetimestr))
                    return NULL;
                valuestring = strdup(CMGetCharsPtr(datetimestr,NULL));
                return valuestring;
            }

        default:
            return NULL;
    }
}

void check_CMPIStatus (CMPIStatus rc)
{
    if (rc.rc != CMPI_RC_OK)
    {
        PROV_LOG ("!!!! %s [%s]", strCMPIStatus (rc),
                  (rc.msg == 0) ? "" : CMGetCharsPtr (rc.msg, NULL));
    }
}


CMPIBoolean evalute_selectcond (const CMPISelectCond * cond,
                                CMPIAccessor *accessor,
                                void *parm)
{
    CMPIStatus rc_String = { CMPI_RC_OK, NULL};
    CMPIStatus rc = { CMPI_RC_OK, NULL};
    int sub_type;
    CMPISelectCond *selectcond_clone = NULL;
    CMPISubCond *subcnd = NULL;
    CMPISubCond *subcnd_clone = NULL;
    /* Predicate operations */
    CMPICount pred_cnt;
    unsigned int pred_idx;
    CMPIPredicate *pred = NULL;
    CMPIPredicate *pred2 = NULL;
    CMPIPredicate *pred_clone = NULL;
    CMPIType pred_type;
    CMPIPredOp pred_op;
    CMPIString *left_side = NULL;
    CMPIString *right_side = NULL;
#ifdef CMPI_VER_100
    CMPIBoolean evalRes = CMPI_false;
#endif
    CMPICount cnt;
    unsigned int idx;

    if (cond != NULL)
    {
        selectcond_clone = CMClone(cond, NULL);
        if (selectcond_clone)
        {
            CMRelease(selectcond_clone);
        }
        PROV_LOG ("---   #A CMGetSubCondCountAndType ");
        cnt = CMGetSubCondCountAndType (cond, &sub_type, &rc);
        check_CMPIStatus (rc);

        PROV_LOG ("---- Number of disjunctives: %d, Type: %X", cnt, sub_type);

        /* Parsing the disjunctives */
        for (idx = 0; idx < cnt; idx++)
        {
            PROV_LOG ("---    #B CMGetSubCondAt @ %d ", idx);
            subcnd = CMGetSubCondAt (cond, idx, &rc);
            check_CMPIStatus (rc);

            /* Try to copy it */
            subcnd_clone = CMClone (subcnd, &rc);
            if (subcnd_clone)
                CMRelease (subcnd_clone);

            PROV_LOG ("---    #C CMGetPredicateCount");
            pred_cnt = CMGetPredicateCount (subcnd, &rc);
            check_CMPIStatus (rc);
            PROV_LOG ("---- Number of predicates in the conjuctives: %d",
                      pred_cnt);

            /* Parsing throught conjuctives */
            for (pred_idx = 0; pred_idx < pred_cnt; pred_idx++)
            {
                PROV_LOG ("--- #D CMGetPredicateAt, %d", pred_idx);
                pred = CMGetPredicateAt (subcnd, pred_idx, &rc);
                check_CMPIStatus (rc);

                pred_clone = CMClone (pred, &rc);
                if (pred_clone)
                    CMRelease (pred_clone);

                PROV_LOG ("--- #E CMGetPredicateData");
                rc = CMGetPredicateData (pred,
                                         &pred_type,
                                         &pred_op, &left_side, &right_side);

                check_CMPIStatus (rc);
                PROV_LOG ("----- Type: %s , CMPIPredOp: %s, LS: %s, RS: %s",
                          _CMPITypeName (pred_type), strCMPIPredOp (pred_op),
                          CMGetCharsPtr (left_side, &rc_String),
                          CMGetCharsPtr (right_side, &rc_String));

                check_CMPIStatus (rc_String);
                // LS has the name. Get the predicate using another mechanism.
                pred2 =
                CMGetPredicate (subcnd, CMGetCharsPtr (left_side, &rc_String),
                                &rc);
                if (pred2)
                {
                    check_CMPIStatus (rc);
                    if (pred == pred2)
                        PROV_LOG ("--- Both predicates are the same.");
                }
#ifdef CMPI_VER_100
                PROV_LOG ("--- #F Evaluate using predicate");
// The CMEvaluatePredicate is gone in the CMPI 1.0 standard.
                evalRes =
                CMEvaluatePredicateUsingAccessor (pred, accessor,
                                                  parm, &rc);
                check_CMPIStatus (rc);
#endif

            }

        }
    }
    return 0;
}
CMPIBoolean evaluate(const CMPISelectExp *se,
                     const CMPIInstance *inst,
                     CMPIAccessor *inst_accessor,
                     void *parm )
{
    CMPIStatus rc_Eval = { CMPI_RC_OK, NULL};
    CMPIStatus rc = { CMPI_RC_OK, NULL};
    /* Select Condition and sub conditions */
    CMPISelectCond *doc_cond = NULL;
    CMPISelectCond *cod_cond = NULL;

    CMPIBoolean evalRes = CMPI_false;
    CMPIBoolean evalResAccessor = CMPI_false;
    CMPIBoolean evalCOD = CMPI_false;
    CMPIBoolean evalDOC = CMPI_false;

    if (se)
    {
        PROV_LOG ("-- #A Evaluate using instance");
        evalRes = CMEvaluateSelExp (se, inst, &rc_Eval);
        if (evalRes==CMPI_true)
        {
            PROV_LOG ("-- Evalute using instance returns true");
        }
        check_CMPIStatus (rc_Eval);
        // Do the same, but with errors.

        PROV_LOG ("-- #B Evalute using accessor");
        evalResAccessor =
        CMEvaluateSelExpUsingAccessor (se, inst_accessor, parm,
                                       &rc_Eval);
        check_CMPIStatus (rc_Eval);
        if (evalResAccessor==CMPI_true)
        {
            PROV_LOG ("-- Evalute using accessor returns true");
        }

        PROV_LOG ("-- #C Continue evaluating using GetDOC");
        {
            doc_cond = CMGetDoc (se, &rc);
            check_CMPIStatus (rc);
            evalDOC=evalute_selectcond (doc_cond, inst_accessor, parm);
            if (evalDOC==CMPI_true)
            {
                PROV_LOG ("-- Evalute using DOC returns true");
            }
        }
        PROV_LOG ("-- #D Continue evaluating using GetCOD");
        {
            /* Currently this is not supported in Pegasus. */
            cod_cond = CMGetCod (se, &rc);
            check_CMPIStatus (rc);
            evalCOD=evalute_selectcond (cod_cond, inst_accessor, parm);
            if (evalCOD==CMPI_true)
            {
                PROV_LOG ("-- Evalute using COD returns true");
            }
        }
    }

    return(evalRes|evalResAccessor|evalDOC|evalCOD);
}
/* --------------------------------------------------------------------------*/
/*                       CMPI Accessor function                        */
/* --------------------------------------------------------------------------*/

CMPIData instance_accessor (const char *name, void *param)
{

    CMPIData data = { 0, CMPI_null, {0}};
    CMPIStatus rc = { CMPI_RC_OK, NULL};
    const CMPIBroker *broker = (CMPIBroker *) param;

    PROV_LOG ("      -- Instance_accessor (Property: %s)", name);

    if (strcmp ("PropertyA", name) == 0)
    {
        data.type = CMPI_string;
        data.state = CMPI_goodValue;
        data.value.string = CMNewString (broker, "AccessorPropertyA", &rc);
    }
    else if (strcmp ("PropertyB", name) == 0)
    {
        data.type = CMPI_string;
        data.state = CMPI_goodValue;
        data.value.string = CMNewString (broker, "AccessorPropertyB", &rc);
    }
    else if (strcmp ("c", name) == 0 && (strlen(name)==1))
    {
        data.type = CMPI_chars;
        data.state = CMPI_goodValue;
        data.value.chars = "c";
    }
    else if (strcmp ("n64", name) == 0)
    {
        data.type = CMPI_uint64;
        data.state = CMPI_goodValue;
        data.value.uint64 = 64;
    }
    else if (strcmp ("n32", name) == 0)
    {
        data.type = CMPI_uint32;
        data.state = CMPI_goodValue;
        data.value.uint32 = 32;
    }
    else if (strcmp ("n16", name) == 0)
    {
        data.type = CMPI_uint16;
        data.state = CMPI_goodValue;
        data.value.uint16 = 16;
    }
    else if (strcmp ("n8", name) == 0)
    {
        data.type = CMPI_uint8;
        data.state = CMPI_goodValue;
        data.value.uint8 = 8;
    }
    else if (strcmp ("s64", name) == 0)
    {
        data.type = CMPI_sint64;
        data.state = CMPI_goodValue;
        data.value.sint64 = 0xFFFFFFFF;
    }
    else if (strcmp ("s32", name) == 0)
    {
        data.type = CMPI_sint32;
        data.state = CMPI_goodValue;
        data.value.sint32 = 0xDEADBEEF;
    }
    else if (strcmp ("s16", name) == 0)
    {
        data.type = CMPI_sint16;
        data.state = CMPI_goodValue;
        data.value.sint16 = (CMPISint16)0xFFFF;
    }
    else if (strcmp ("s8", name) == 0)
    {
        data.type = CMPI_sint8;
        data.state = CMPI_goodValue;
        data.value.sint8 = (CMPISint8)0xFF;
    }
    else if (strcmp ("r64", name) == 0)
    {
        data.type = CMPI_real64;
        data.state = CMPI_goodValue;
        data.value.real64 = 3.1415678928283;
    }
    else if (strcmp ("r32", name) == 0)
    {
        data.type = CMPI_real32;
        data.state = CMPI_goodValue;
        data.value.real32 = (CMPIReal32)1.23;
    }

    else if (strcmp ("b", name) == 0 && (strlen(name)==1))
    {
        data.type = CMPI_boolean;
        data.state = CMPI_goodValue;
        data.value.boolean = 1;
    }
    else if ((strcmp ("s", name) == 0) && (strlen(name) == 1))
    {
        data.type = CMPI_string;
        data.state = CMPI_goodValue;
        data.value.string = CMNewString (broker, "s", &rc);
    }
    else if ((strcmp("d", name) == 0) && strlen(name) == 1)
    {
        data.type = CMPI_dateTime;
        data.state = CMPI_goodValue;
        data.value.dateTime = CMNewDateTime(broker, &rc);
    }
    check_CMPIStatus(rc);
    return data;
}

