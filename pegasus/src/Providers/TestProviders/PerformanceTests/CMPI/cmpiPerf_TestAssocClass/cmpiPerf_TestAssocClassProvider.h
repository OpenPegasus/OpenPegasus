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

#ifndef _CMPI_TESTASSOCCLASS_H_
#define _CMPI_TESTASSOCCLASS_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>

#if defined(CMPI_PLATFORM_WIN32_IX86_MSVC)
#define strcasecmp stricmp
#endif

/* ---------------------------------------------------------------------------*/
/*                      _assoc_targetClass_Name()                             */
/* ---------------------------------------------------------------------------*/
/*                  method to get the name of the target class                */
/*                                                                            */
/* return value NULL : association is not responsible for this request        */
/* ---------------------------------------------------------------------------*/
const char * _assoc_targetClass_Name(
    const CMPIBroker * _broker,
    const CMPIObjectPath * ref,
    const char * _RefLeftClass,
    const char * _RefRightClass,
    CMPIStatus * rc)
{
    CMPIString * sourceClass = NULL;
    sourceClass = CMGetClassName(ref, rc);
    if ( sourceClass == NULL )
    {
        CMSetStatusWithChars(
            _broker,
            rc,
            CMPI_RC_ERR_FAILED,
            "Could not get classname of source object path.");
        return NULL;
    }
    if ( strcmp(CMGetCharsPtr(sourceClass, NULL), _RefLeftClass ) == 0 )
    {
        /* pathName = left end -> get right end */
        return _RefRightClass;
    }
    else if ( strcmp(CMGetCharsPtr(sourceClass, NULL), _RefRightClass ) == 0 )
    {
        /* pathName = right end -> get left end */
        return _RefLeftClass;
    }
    else if ( CMClassPathIsA(_broker,ref,_RefLeftClass,rc) == 1 )
    {
        /* pathName = left end -> get right end */
        return _RefRightClass;
    }
    else if ( CMClassPathIsA(_broker,ref,_RefRightClass,rc) == 1 )
    {
        /* pathName = right end -> get left end */
        return _RefLeftClass;
    }

    CMSetStatusWithChars(
        _broker,
        rc,
        CMPI_RC_ERR_FAILED,
        "referenced Class is not covered by this Association.");
    return NULL;
}


/* ---------------------------------------------------------------------------*/
/*                       _assoc_targetClass_OP()                              */
/* ---------------------------------------------------------------------------*/
/*        method to create an empty CMPIObjectPath of the target class        */
/*                                                                            */
/* return value NULL : association is not responsible for this request        */
/* ---------------------------------------------------------------------------*/

CMPIObjectPath * _assoc_targetClass_OP(
    const CMPIBroker * _broker,
    const CMPIObjectPath * ref,
    const char * _RefLeftClass,
    const char * _RefRightClass,
    CMPIStatus * rc)
{
    CMPIObjectPath  * op = NULL;
    const char * targetName = NULL;

    targetName=_assoc_targetClass_Name(
                   _broker,
                   ref,
                   _RefLeftClass,
                   _RefRightClass,
                   rc);

    if ( targetName != NULL )
    {
        op = CMNewObjectPath(
                 _broker,
                 CMGetCharsPtr(CMGetNameSpace(ref,rc), NULL),
                 targetName,
                 rc);
        if (CMIsNullObject(op))
        {
            CMSetStatusWithChars(
                _broker,
                rc,
                CMPI_RC_ERR_FAILED,
                "Create CMPIObjectPath failed." );
        }
    }
    return op;
}

/* ---------------------------------------------------------------------------*/
/*                    _assoc_check_parameter_const()                          */
/* ---------------------------------------------------------------------------*/
/*    method to check the input parameter resultClass, role and resultRole    */
/*           submitted to the methods of the association interface            */
/*                                                                            */
/* return value 0 : association is not responsible for this request           */
/* ---------------------------------------------------------------------------*/

int _assoc_check_parameter_const(
    const CMPIBroker * _broker,
    const CMPIObjectPath * cop,
    const char * _RefLeft,
    const char * _RefRight,
    const char * _RefLeftClass,
    const char * _RefRightClass,
    const char * resultClass,
    const char * role,
    const char * resultRole,
    CMPIStatus * rc )
{
    CMPIObjectPath * op          = NULL;
    CMPIObjectPath * scop        = NULL;
    CMPIObjectPath * rcop        = NULL;
    CMPIString     * sourceClass = NULL;
    int              intrc       = 0;

    sourceClass = CMGetClassName(cop, rc);
    scop=CMNewObjectPath(
             _broker,
             CMGetCharsPtr(CMGetNameSpace(cop,rc), NULL),
             CMGetCharsPtr(sourceClass, NULL),
             rc);

    if (strcasecmp(CMGetCharsPtr(sourceClass, NULL),_RefLeftClass) == 0 ||
        CMClassPathIsA(_broker,scop,_RefLeftClass,rc) == 1)
    {
        intrc = 1;
    }
    else if (strcasecmp(CMGetCharsPtr(sourceClass, NULL),_RefRightClass) == 0 ||
             CMClassPathIsA(_broker,scop,_RefRightClass,rc) == 1)
    {
        intrc = 1;
    }
    else
    {
        return intrc;
    }

    if (!(resultClass || role || resultRole))
    {
        return 1;
    }
    /* check if resultClass is parent */
    /* or the class itself of the target class */
    if (resultClass)
    {
        if (strcasecmp(CMGetCharsPtr(sourceClass, NULL),_RefLeftClass) == 0 ||
            CMClassPathIsA(_broker,scop,_RefLeftClass,rc) == 1)
        {
            op = CMNewObjectPath(
                     _broker,
                     CMGetCharsPtr(CMGetNameSpace(cop,rc), NULL),
                     _RefRightClass,
                     rc);
        }
        else if (strcasecmp(
            CMGetCharsPtr(sourceClass, NULL),
            _RefRightClass)==0 ||
            CMClassPathIsA(_broker,scop,_RefRightClass,rc) == 1)
        {
            op=CMNewObjectPath(
                   _broker,
                   CMGetCharsPtr(CMGetNameSpace(cop,rc), NULL),
                   _RefLeftClass,
                   rc);
        }
        else
        {
            return intrc;
        }

        rcop=CMNewObjectPath(
                 _broker,
                 CMGetCharsPtr(CMGetNameSpace(cop,rc), NULL),
                 resultClass,
                 rc);

        if (CMClassPathIsA(_broker,op,resultClass,rc) == 1)
        {
            intrc = 1;
        }
        else if ((CMClassPathIsA(_broker,rcop,_RefRightClass,rc) == 1 &&
            strcasecmp(CMGetCharsPtr(sourceClass, NULL),_RefLeftClass)==0) ||
            (CMClassPathIsA(_broker,rcop,_RefRightClass,rc) == 1 &&
            CMClassPathIsA(_broker,scop,_RefLeftClass,rc) == 1 ))
        {
            intrc = 1;
        }
        else if ((CMClassPathIsA(_broker,rcop,_RefLeftClass,rc) == 1 &&
            strcasecmp(CMGetCharsPtr(sourceClass, NULL), _RefRightClass)==0) ||
            (CMClassPathIsA(_broker,rcop,_RefLeftClass,rc) == 1 &&
            CMClassPathIsA(_broker,scop,_RefRightClass,rc) == 1 ))
        {
            intrc = 1;
        }
        else
        {
            return 0;
        }
    }

    /* check if the source object (cop) plays the Role ( specified in input
     * parameter 'role' within this association */
    if (role)
    {
        if (strcasecmp(
            CMGetCharsPtr(sourceClass, NULL),_RefLeftClass) == 0 ||
            CMClassPathIsA(_broker,scop,_RefLeftClass,rc) == 1)
        {
            if (strcasecmp(role,_RefLeft) != 0)
            {
                return 0;
            }
            else
            {
                intrc = 1;
            }
        }
        else if (strcasecmp(
            CMGetCharsPtr(sourceClass, NULL),_RefRightClass) == 0 ||
            CMClassPathIsA(_broker,scop,_RefRightClass,rc) == 1)
        {
            if (strcasecmp(role,_RefRight) != 0)
            {
                return 0;
            }
            else
            {
                intrc = 1;
            }
        }
        else
        {
            return intrc;
        }
    }

    /* check if the target object plays the Role ( specified in input
     * parameter 'resultRole' within this association */
    if (resultRole)
    {
        if (strcasecmp(
            CMGetCharsPtr(sourceClass, NULL),_RefLeftClass) == 0 ||
            CMClassPathIsA(_broker,scop,_RefLeftClass,rc) == 1)
        {
            if (strcasecmp(resultRole,_RefRight) != 0)
            {
                return 0;
            }
            else
            {
                intrc = 1;
            }
        }
        else if (strcasecmp(
            CMGetCharsPtr(sourceClass, NULL),_RefRightClass) == 0 ||
            CMClassPathIsA(_broker,scop,_RefRightClass,rc) == 1)
        {
            if (strcasecmp(resultRole,_RefLeft) != 0)
            {
                return 0;
            }
            else
            {
                intrc = 1;
            }
        }
        else
        {
            return intrc;
        }
    }
    return intrc;
}

/* ---------------------------------------------------------------------------*/
/*                           _assoc_get_inst()                                */
/* ---------------------------------------------------------------------------*/
/*     method to get CMPIInstance object of an association CMPIObjectPath     */
/*                                                                            */
/* ---------------------------------------------------------------------------*/
CMPIInstance * _assoc_get_inst(
    const CMPIBroker * _broker,
    const CMPIContext * ctx,
    const CMPIObjectPath * cop,
    const char * _ClassName,
    const char * _RefLeft,
    const char * _RefRight,
    CMPIStatus * rc)
{
    CMPIInstance   * ci = NULL;
    CMPIObjectPath * op = NULL;
    CMPIData         dtl;
    CMPIData         dtr;

    dtl = CMGetKey( cop, _RefLeft, rc);
    if (dtl.value.ref == NULL)
    {
        CMSetStatusWithChars(
            _broker,
            rc,
            CMPI_RC_ERR_FAILED,
            "CMGetKey( cop, _RefLeft, rc)");
        return ci;
    }

    CMSetNameSpace(dtl.value.ref,CMGetCharsPtr(CMGetNameSpace(cop,rc), NULL));
    ci = CBGetInstance(_broker, ctx, dtl.value.ref, NULL, rc);
    if (ci == NULL)
    {
        if (rc->rc == CMPI_RC_ERR_FAILED)
        {
            CMSetStatusWithChars(
                _broker,
                rc,
                CMPI_RC_ERR_FAILED,
                "GetInstance of left reference failed.");
        }
        if (rc->rc == CMPI_RC_ERR_NOT_FOUND)
        {
            CMSetStatusWithChars(
                _broker,
                rc,
                CMPI_RC_ERR_NOT_FOUND,
                "Left reference not found.");
        }
        return ci;
    }

    dtr = CMGetKey( cop, _RefRight, rc);
    if (dtr.value.ref == NULL)
    {
        CMSetStatusWithChars(
            _broker,
            rc,
            CMPI_RC_ERR_FAILED,
            "CMGetKey( cop, _RefRight, rc)");
        return ci;
    }

    CMSetNameSpace(dtr.value.ref,CMGetCharsPtr(CMGetNameSpace(cop,rc), NULL));
    ci = CBGetInstance(_broker, ctx, dtr.value.ref, NULL, rc);
    if (ci == NULL)
    {
        if (rc->rc == CMPI_RC_ERR_FAILED)
        {
            CMSetStatusWithChars(
                _broker,
                rc,
                CMPI_RC_ERR_FAILED,
                "GetInstance of right reference failed.");
        }
        if (rc->rc == CMPI_RC_ERR_NOT_FOUND)
        {
            CMSetStatusWithChars(
                _broker,
                rc,
                CMPI_RC_ERR_NOT_FOUND,
                "Right reference not found.");
        }
        return ci;
    }

    op = CMNewObjectPath(
             _broker,
             CMGetCharsPtr(CMGetNameSpace(cop,rc), NULL),
             _ClassName,
             rc);
    if (CMIsNullObject(op))
    {
        CMSetStatusWithChars(
            _broker,
            rc,
            CMPI_RC_ERR_FAILED,
            "Create CMPIObjectPath failed." );
        return ci;
    }

    ci = CMNewInstance(_broker, op, rc);
    if (CMIsNullObject(ci))
    {
        CMSetStatusWithChars(
            _broker,
            rc,
            CMPI_RC_ERR_FAILED,
            "Create CMPIInstance failed.");
        return ci;
    }

    CMSetProperty(ci,_RefLeft,(CMPIValue*)&(dtl.value.ref),CMPI_ref );
    CMSetProperty(ci,_RefRight,(CMPIValue*)&(dtr.value.ref),CMPI_ref );

    return ci;
}

/* ---------------------------------------------------------------------------*/
/*                      _assoc_create_refs_1toN()                             */
/* ---------------------------------------------------------------------------*/
/* method to return CMPIInstance(s) / CMPIObjectPath(s) of related objects    */
/* and the association itself                                                 */
/*                                                                            */
/* combination of int <inst> and int <associators> :
 * 0 0 -> referenceNames()
 * 1 0 -> references()
 * 0 1 -> associatorNames()
 * 1 1 -> associators()
 */
/* return value : SUCCESS = 0 ; FAILED = -1                                   */
/* !!! this method returns each found CMPIInstance / CMPIObjectPath object to */
/* the Object Manager (OM)                                                    */
/* ---------------------------------------------------------------------------*/

int _assoc_create_refs_1toN(
    const CMPIBroker * _broker,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char * _ClassName,
    const char * _RefLeftClass,
    const char * _RefRightClass,
    const char * _RefLeft,
    const char * _RefRight,
    int inst,
    int associators,
    CMPIStatus * rc)
{

    CMPIInstance    * cis = NULL;
    CMPIInstance    * ci  = NULL;
    CMPIObjectPath  * op  = NULL;
    CMPIObjectPath  * rop = NULL;
    CMPIObjectPath  * cop = NULL;
    CMPIEnumeration * en  = NULL;
    CMPIData          data ;
    const char      * targetName = NULL;

    /* check if source instance does exist */
    cis = CBGetInstance(_broker, ctx, ref, NULL, rc);
    if (cis == NULL)
    {
        if (rc->rc == CMPI_RC_ERR_FAILED)
        {
            CMSetStatusWithChars(
                _broker,
                rc,
                CMPI_RC_ERR_FAILED,
                "GetInstance of source object failed.");
        }
        else if (rc->rc == CMPI_RC_ERR_NOT_FOUND)
        {
            CMSetStatusWithChars(
                _broker,
                rc,
                CMPI_RC_ERR_NOT_FOUND,
                "Source object not found.");
        }
        else if (rc->rc == CMPI_RC_ERR_ACCESS_DENIED)
        {
            CMSetStatusWithChars(
                _broker,
                rc,
                CMPI_RC_ERR_ACCESS_DENIED,
                "Access denied for source object.");
        }
        else
        {
            CMSetStatusWithChars(
                _broker,
                rc,
                rc->rc,
                "Unexpected error.");
        }
        return -1;
    }

    op = _assoc_targetClass_OP(_broker,ref,_RefLeftClass,_RefRightClass,rc);
    if (op == NULL)
    {
        return 0;
    }

    rop = CMNewObjectPath(
              _broker,
              CMGetCharsPtr(CMGetNameSpace(ref,rc), NULL),
              _ClassName,
              rc);
    if (CMIsNullObject(rop))
    {
        CMSetStatusWithChars(
            _broker,
            rc,
            CMPI_RC_ERR_FAILED,
            "Create CMPIObjectPath failed.");
        return -1;
    }

    if ((associators == 1) && (inst == 1))
    {
        /* associators() */
        en = CBEnumInstances( _broker, ctx, op, NULL, rc);
        if (en == NULL)
        {
            CMSetStatusWithChars(
                _broker,
                rc,
                CMPI_RC_ERR_FAILED,
                CMGetCharsPtr(rc->msg, NULL));
            return -1;
        }
        while (CMHasNext( en, rc))
        {
            data = CMGetNext( en, rc);
            if (data.value.inst == NULL)
            {
                CMSetStatusWithChars(
                    _broker,
                    rc,
                    CMPI_RC_ERR_FAILED,
                    "CMGetNext( en, rc)");
                return -1;
            }
            CMReturnInstance( rslt, data.value.inst );
            CMRelease( data.value.inst );
        }
    }
    else
    {
        en = CBEnumInstanceNames(_broker, ctx, op, rc);
        if (en == NULL)
        {
            CMSetStatusWithChars(
                _broker,
                rc,
                CMPI_RC_ERR_FAILED,
                CMGetCharsPtr(rc->msg, NULL));
            return -1;
        }

        while (CMHasNext( en, rc))
        {
            data = CMGetNext( en, rc);
            if (data.value.ref == NULL)
            {
                CMSetStatusWithChars(
                    _broker,
                    rc,
                    CMPI_RC_ERR_FAILED,
                    "CMGetNext( en, rc)" );
                return -1;
            }
            if (associators == 0)
            {
                /* references() || referenceNames() */
                ci = CMNewInstance( _broker, rop, rc);
                if (CMIsNullObject(ci))
                {
                    CMSetStatusWithChars(
                        _broker,
                        rc,
                        CMPI_RC_ERR_FAILED,
                        "Create CMPIInstance failed.");
                    return -1;
                }

                targetName=_assoc_targetClass_Name(
                               _broker,
                               ref,
                               _RefLeftClass,
                               _RefRightClass,
                               rc);
                if (strcmp(targetName,_RefRightClass) == 0)
                {
                    CMSetProperty(ci,_RefLeft,(CMPIValue*)&(ref), CMPI_ref );
                    CMSetProperty(
                        ci,
                        _RefRight,
                        (CMPIValue*)&(data.value.ref),
                        CMPI_ref);
                }
                else if (strcmp( targetName,_RefLeftClass) == 0)
                {
                    CMSetProperty(
                        ci,
                        _RefLeft,
                        (CMPIValue*)&(data.value.ref),
                        CMPI_ref );
                    CMSetProperty(ci,_RefRight,(CMPIValue*)&(ref),CMPI_ref);
                }
                if (inst == 0)
                {
                    cop = CMGetObjectPath(ci,rc);
                    if (cop == NULL)
                    {
                        CMSetStatusWithChars(
                            _broker,
                            rc,
                            CMPI_RC_ERR_FAILED,
                            "CMGetObjectPath(ci,rc)" );
                        return -1;
                    }
                    CMSetNameSpace(
                        cop,
                        CMGetCharsPtr(CMGetNameSpace(ref,rc), NULL));
                    CMReturnObjectPath( rslt, cop );
                    CMRelease( cop );
                }
                else
                {
                    CMReturnInstance( rslt, ci );
                    CMRelease( ci );
                }
            }
            else
            {
                /* associatorNames() */
                if (inst == 0)
                {
                    CMReturnObjectPath( rslt, data.value.ref );
                    CMRelease( data.value.ref );
                }
            }

        }
    }
    return 0;
}

/* ---------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/*                      _assoc_create_inst_1toN()                             */
/* ---------------------------------------------------------------------------*/
/*     method to create CMPIInstance(s) / CMPIObjectPath(s) of association    */
/*                                                                            */
/* combination of int <left> and int <inst> :
 * 0 0 -> enum CMPIObjectPath(s) of right end;
 *        return CMPIObjectPath(s) of association
 * 1 0 -> enum CMPIObjectPath(s) of left end;
 *        return CMPIObjectPath(s) of association
 * 0 1 -> enum CMPIObjectPath(s) of right end;
 *        return CMPIInstance(s) of association
 * 1 1 -> enum CMPIObjectPath(s) of left end;
 *        return CMPIInstance(s) of association
 */
/* return value : SUCCESS = 0 ; FAILED = -1                                   */
/* !!! while calling _assoc_create_refs_1toN(), this method returns each      */
/* found CMPIInstance / CMPIObjectPath object to the Object Manager (OM)      */
/* ---------------------------------------------------------------------------*/

int _assoc_create_inst_1toN(
    const CMPIBroker * _broker,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const char * _ClassName,
    const char * _RefLeftClass,
    const char * _RefRightClass,
    const char * _RefLeft,
    const char * _RefRight,
    int left,
    int inst,
    CMPIStatus * rc)
{
    CMPIObjectPath  * op  = NULL;
    CMPIEnumeration * en  = NULL;
    CMPIData          data ;
    int               arc = 0;

    if (left == 0)
    {
        op = CMNewObjectPath(
                 _broker,
                 CMGetCharsPtr(CMGetNameSpace(cop,rc), NULL),
                 _RefRightClass,
                 rc);
    }
    else
    { /* left == 1 */
        op = CMNewObjectPath(
                 _broker,
                 CMGetCharsPtr(CMGetNameSpace(cop,rc), NULL),
                 _RefLeftClass,
                 rc);
    }
    if (CMIsNullObject(op))
    {
        CMSetStatusWithChars(
            _broker,
            rc,
            CMPI_RC_ERR_FAILED,
            "Create CMPIObjectPath failed.");
        return -1;
    }

    en = CBEnumInstanceNames( _broker, ctx, op, rc);
    if (en == 0)
    {
        CMSetStatusWithChars(
            _broker,
            rc,
            CMPI_RC_ERR_FAILED,
            CMGetCharsPtr(rc->msg, NULL));
        return -1;
    }

    /* this approach works only for 1 to N relations
     * int <left> contains the information, which side of the association has
     * only one instance
     */
    while (CMHasNext(en,rc))
    {
        data = CMGetNext( en, rc);
        if (data.value.ref == NULL)
        {
            CMSetStatusWithChars(
                _broker,
                rc,
                CMPI_RC_ERR_FAILED,
                "CMGetNext(en,rc)" );
            return -1;
        }
        arc = _assoc_create_refs_1toN(
                  _broker,
                  ctx,
                  rslt,
                  data.value.ref,
                  _ClassName,
                  _RefLeftClass,
                  _RefRightClass,
                  _RefLeft,
                  _RefRight,
                  inst,
                  0,
                  rc);
    }

    return arc;
}

#endif /* _CMPI_TESTASSOCCLASS_H_ */

