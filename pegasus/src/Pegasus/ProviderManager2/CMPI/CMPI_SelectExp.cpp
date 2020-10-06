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

#include "CMPI_SelectExp.h"
#include "CMPI_Ftabs.h"
#include "CMPI_Value.h"
#include "CMPI_String.h"
#include "CMPI_SelectExpAccessor_WQL.h"
#include "CMPI_ThreadContext.h"

#ifdef PEGASUS_ENABLE_CQL
# include "CMPI_SelectExpAccessor_CQL.h"
# include <Pegasus/CQL/CQLSelectStatement.h>
# include <Pegasus/CQL/CQLParser.h>
#endif

#include <Pegasus/WQL/WQLInstancePropertySource.h>
#include <Pegasus/Provider/CIMOMHandleQueryContext.h>
#include <Pegasus/WQL/WQLParser.h>
#include <Pegasus/Common/Tracer.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

extern "C"
{

    PEGASUS_STATIC CMPIStatus selxRelease (CMPISelectExp * eSx)
    {
        CMPI_SelectExp *se = (CMPI_SelectExp*)eSx;
        if( !se->persistent )
        {
            /**
                Do not call unlinkAndDelete - b/c the CMPI_Object::
                unlinkAndDelete casts the structure to a CMPI_Object and
                deletes it. But this is a CMPI_SelectExp structure so not
                all of the variables get deleted. Hence we delete them here.
                ((CMPI_Object*)se)->unlinkAndDelete();
            */
            (reinterpret_cast<CMPI_Object*>(se))->unlink();
        }
        delete se;

        CMReturn (CMPI_RC_OK);
    }

    /**
       This will not clone all the CMPISelectExp objects. It clones only when
       original object has either CQLSelectStatement or WQLSelectStatement.
       Any other properties of original object may cause clone to retun error
       CMPI_RC_ERR_NOT_SUPPORTED. Use this only when you have just created
      CMPISelectExp object with CMNewselectExp
      (broker,query,lang,projection,rc)
    */

    PEGASUS_STATIC CMPISelectExp *selxClone (
        const CMPISelectExp * eSx,
        CMPIStatus * rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_SelectExp:selxClone()");

        //initialise to silence uninitialised use warning
        CMPI_SelectExp *new_se = NULL;
        CMPI_SelectExp *se = (CMPI_SelectExp*) eSx;

        if((
#ifdef PEGASUS_ENABLE_CQL
        !se->cql_stmt &&
#endif
        !se->wql_stmt) || se->_context || se->hdl )
        {
            CMSetStatus (rc, CMPI_RC_ERR_NOT_SUPPORTED);
            PEG_METHOD_EXIT();
            return NULL;
        }

#ifdef PEGASUS_ENABLE_CQL
        CQLSelectStatement *cql_stmt;
#endif

        WQLSelectStatement *wql_stmt;
        Boolean  disable_cql = true;

#ifdef PEGASUS_ENABLE_CQL
        if( se->cql_stmt )
        {
            cql_stmt = new CQLSelectStatement (*se->cql_stmt);
            new_se = new CMPI_SelectExp (cql_stmt, true);
            disable_cql = false;
        }
#endif

        if( disable_cql )
        {
            wql_stmt = new WQLSelectStatement (*se->wql_stmt);
            new_se = new CMPI_SelectExp (wql_stmt, true);
        }

        PEG_METHOD_EXIT();
        return(CMPISelectExp*) new_se;
    }

    /**
       Helper functions
    */
    PEGASUS_STATIC CMPIBoolean _check_WQL (
        CMPI_SelectExp * sx,
        CMPIStatus * rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_SelectExp:_check_WQL()");
        if( sx->wql_stmt == NULL )
        {
            WQLSelectStatement *stmt = new WQLSelectStatement ();
            try
            {
                WQLParser::parse (sx->cond, *stmt);
            }
            catch( const Exception &e )
            {
                PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
                    "Exception: _check_WQL - msg: %s",
                    (const char*)e.getMessage().getCString()));

                if( rc )
                {
                    CMSetStatusWithString(
                        rc,
                        CMPI_RC_ERR_INVALID_QUERY,
                        (CMPIString*)string2CMPIString(e.getMessage()));
                }

                delete stmt;
                PEG_METHOD_EXIT();
                return false;
            }
            catch( ... )
            {
                PEG_TRACE_CSTRING(
                    TRC_CMPIPROVIDERINTERFACE,
                    Tracer::LEVEL1,
                    "Exception: Unknown Exception in _check_WQL");
                delete stmt;
                CMSetStatus (rc, CMPI_RC_ERR_INVALID_QUERY);
                PEG_METHOD_EXIT();
                return false;
            }
            /**
               Only set it for success
            */
            sx->wql_stmt = stmt;

        /* sx->wql_stmt ... */
        }
        PEG_METHOD_EXIT();
        return true;
    }
#ifdef PEGASUS_ENABLE_CQL
    PEGASUS_STATIC CMPIBoolean _check_CQL (
        CMPI_SelectExp * sx,
        CMPIStatus * rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_SelectExp:_check_CQL()");
        Boolean fail = false;
        if( sx->cql_stmt == NULL )
        {
            /**
                The constructor should set this to a valid pointer.
            */
            if( sx->_context == NULL )
            {
                CMSetStatus (rc, CMPI_RC_ERROR_SYSTEM);
                PEG_METHOD_EXIT();
                return false;
            }
            CQLSelectStatement *selectStatement =
            new CQLSelectStatement (sx->lang, sx->cond, *sx->_context);
            try
            {
                CQLParser::parse (sx->cond, *selectStatement);

                selectStatement->validate ();
            }
            catch( const Exception &e )
            {
                PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
                    "Exception: _check_CQL - msg: %s",
                    (const char*)e.getMessage().getCString()));

                if( rc )
                {
                    CMSetStatusWithString(
                    rc,
                    CMPI_RC_ERR_INVALID_QUERY,
                    (CMPIString*)string2CMPIString(e.getMessage()));
                }

                fail = true;
            }
            catch( ... )
            {
                PEG_TRACE_CSTRING(
                    TRC_CMPIPROVIDERINTERFACE,
                    Tracer::LEVEL1,
                    "Exception: Unknown Exception in _check_CQL");
                CMSetStatus (rc, CMPI_RC_ERR_INVALID_QUERY);
                fail = true;
            }
            if( fail )
            {
                delete selectStatement;
                PEG_METHOD_EXIT();
                return false;
            }
            sx->cql_stmt = selectStatement;
        }
        PEG_METHOD_EXIT();
        return true;
    }
#endif
    PEGASUS_STATIC CMPIBoolean selxEvaluate (
    const CMPISelectExp * eSx,
    const CMPIInstance * inst,
    CMPIStatus * rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_SelectExp:selxEvaluate()");
        CMPI_SelectExp *sx = (CMPI_SelectExp *) eSx;
        if( !inst )
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid Parameter in \
                CMPI_SelectExp:selxEvaluate");
            CMSetStatus (rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return false;
        }
        if( !inst->hdl )
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid parameter inst->hdl in \
                CMPI_SelectExp:selxEvaluate");
            CMSetStatus (rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return false;
        }

        SCMOInstance * scmoInst = (SCMOInstance*) inst->hdl;
        CIMInstance cimInstance;
        SCMO_RC smrc = scmoInst->getCIMInstance(cimInstance);
        if (SCMO_OK != smrc)
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Failed to convert SCMOInstance to CIMInstance");
            CMSetStatus (rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return false;
        }

        /**
           WQL
        */
        if( strncmp (sx->lang.getCString (),
        CALL_SIGN_WQL, CALL_SIGN_WQL_SIZE) == 0 )
        {
            if( _check_WQL (sx, rc) )
            {
                try
                {
                    PEG_METHOD_EXIT();
                    return sx->wql_stmt->evaluate (cimInstance);
                }
                catch( const Exception &e )
                {
                    PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
                        "Exception: selxEvaluateWQL - msg: %s",
                        (const char*)e.getMessage().getCString()));

                    if( rc )
                    {
                        CMSetStatusWithString(rc,CMPI_RC_ERR_FAILED,
                        (CMPIString*)string2CMPIString(e.getMessage()));
                    }
                    PEG_METHOD_EXIT();
                    return false;
                }
                catch( ... )
                {
                    PEG_TRACE_CSTRING(
                        TRC_CMPIPROVIDERINTERFACE,
                        Tracer::LEVEL1,
                        "Exception: Unknown Exception in selxEvaluate");
                    CMSetStatus (rc, CMPI_RC_ERR_FAILED);
                    PEG_METHOD_EXIT();
                    return false;
                }
            }
            else
            {
                PEG_METHOD_EXIT();
                return false;
            }
        }
        /**
         CIM:CQL
       */
#ifdef PEGASUS_ENABLE_CQL
        if( (strncmp (sx->lang.getCString(),
        CALL_SIGN_CQL, CALL_SIGN_CQL_SIZE) == 0) ||
        (strncmp (sx->lang.getCString(),
        "CIM:CQL", 7) == 0) )
        {
            if( _check_CQL (sx, rc) )
            {
                try
                {
                    PEG_METHOD_EXIT();
                    return sx->cql_stmt->evaluate (cimInstance);
                }
                catch( const Exception &e )
                {
                    PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
                        "Exception: selxEvaluateCQL - msg: %s",
                        (const char*)e.getMessage().getCString()));

                    if( rc )
                    {
                        CMSetStatusWithString(
                        rc,
                        CMPI_RC_ERR_FAILED,
                        (CMPIString*)string2CMPIString(e.getMessage()));
                    }
                    PEG_METHOD_EXIT();
                    return false;
                }
                catch( ... )
                {
                    PEG_TRACE_CSTRING(
                        TRC_CMPIPROVIDERINTERFACE,
                        Tracer::LEVEL1,
                        "Exception: Unknown Exception in selxEvaluate");
                    CMSetStatus (rc, CMPI_RC_ERR_FAILED);
                    PEG_METHOD_EXIT();
                    return false;
                }
            }
            else
            {
                PEG_METHOD_EXIT();
                return false;
            }
        }
#endif
         /**
          Tried some other weird query language which we don't support
        */
        CMSetStatus (rc, CMPI_RC_ERR_NOT_SUPPORTED);
        PEG_METHOD_EXIT();
        return false;
    }

    PEGASUS_STATIC CMPIBoolean selxEvaluateUsingAccessor (
        const CMPISelectExp * eSx,
        CMPIAccessor * accessor,
        void *parm, CMPIStatus * rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_SelectExp:selxEvaluateUsingAccessor()");
        CMPI_SelectExp *sx = (CMPI_SelectExp *) eSx;
        if( !accessor )
        {
            PEG_TRACE_CSTRING(
                TRC_CMPIPROVIDERINTERFACE,
                Tracer::LEVEL1,
                "Invalid Parameter in \
                CMPI_SelectExp:selxEvaluateUsingAccessor");
            CMSetStatus (rc, CMPI_RC_ERR_INVALID_PARAMETER);
            PEG_METHOD_EXIT();
            return false;
        }

        if( strncmp (sx->lang.getCString (),
        CALL_SIGN_WQL, CALL_SIGN_WQL_SIZE) == 0 )
        {
            if( _check_WQL (sx, rc) )
            {
                CMPI_SelectExpAccessor_WQL ips (accessor, parm);
                try
                {
                    CMSetStatus (rc, CMPI_RC_OK);
                    PEG_METHOD_EXIT();
                    return sx->wql_stmt->evaluateWhereClause (&ips);
                }
                catch( const Exception &e )
                {
                    PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
                        "Exception: selxEvaluateUsingAccessorWQL - msg: %s",
                        (const char*)e.getMessage().getCString()));

                    if( rc )
                    {
                        CMSetStatusWithString(
                        rc,
                        CMPI_RC_ERR_FAILED,
                        (CMPIString*)string2CMPIString(e.getMessage()));
                    }
                    PEG_METHOD_EXIT();
                    return false;
                }
                catch( ... )
                {
                    PEG_TRACE_CSTRING(
                        TRC_CMPIPROVIDERINTERFACE,
                        Tracer::LEVEL1,
                        "Exception: Unknown Exception in \
                        selxEvaluateUsingAccessor");
                    CMSetStatus (rc, CMPI_RC_ERR_FAILED);
                    PEG_METHOD_EXIT();
                    return false;
                }
            }
            else
            {
                PEG_METHOD_EXIT();
                return false;
            }
        }
#ifdef PEGASUS_ENABLE_CQL
        if( (strncmp (sx->lang.getCString(),
            CALL_SIGN_CQL, CALL_SIGN_CQL_SIZE) == 0) ||
            (strncmp (sx->lang.getCString(),
            "CIM:CQL", 7) == 0) )
        {
            if( _check_CQL (sx, rc) )
            {
                CMPI_SelectExpAccessor_CQL ips (accessor, parm, sx->cql_stmt,
                sx->classNames[0]);
                try
                {
                    CMSetStatus (rc, CMPI_RC_OK);
                    return sx->cql_stmt->evaluate (ips.getInstance ());
                }
                catch( const Exception &e )
                {
                    PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
                        "Exception: selxEvaluateUsingAccessorCQL - msg: %s",
                        (const char*)e.getMessage().getCString()));

                    if( rc )
                    {
                        CMSetStatusWithString(rc,CMPI_RC_ERR_FAILED,
                        (CMPIString*)string2CMPIString(e.getMessage()));
                    }
                    PEG_METHOD_EXIT();
                    return false;
                }
                catch( ... )
                {
                    PEG_TRACE_CSTRING(
                        TRC_CMPIPROVIDERINTERFACE,
                        Tracer::LEVEL1,
                        "Exception: Unknown Exception in \
                        selxEvaluateUsingAccessor");
                    CMSetStatus (rc, CMPI_RC_ERR_FAILED);
                    PEG_METHOD_EXIT();
                    return false;
                }
            }
            else
            {
                PEG_METHOD_EXIT();
                return false;
        }
        }
#endif
        PEG_METHOD_EXIT();
        return false;
    }

    PEGASUS_STATIC CMPIString *selxGetString (
        const CMPISelectExp * eSx,
        CMPIStatus * rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_SelectExp:selxGetString()");
        CMPI_SelectExp *sx = (CMPI_SelectExp *) eSx;
        CMSetStatus (rc, CMPI_RC_OK);
        PEG_METHOD_EXIT();
        return string2CMPIString (sx->cond);
    }

    PEGASUS_STATIC CMPISelectCond *selxGetDOC (
        const CMPISelectExp * eSx,
        CMPIStatus * rc)
    {
        PEG_METHOD_ENTER(
            TRC_CMPIPROVIDERINTERFACE,
            "CMPI_SelectExp:selxGetDOC()");
        CMPI_SelectExp *sx = (CMPI_SelectExp *) eSx;
        CMPISelectCond *sc = NULL;

        if( strncmp (
               sx->lang.getCString (),
               CALL_SIGN_WQL, CALL_SIGN_WQL_SIZE) == 0 )
        {
            if( sx->wql_dnf == NULL )
            {
                CMPI_Wql2Dnf *dnf = NULL;
                try
                {
                    dnf = new CMPI_Wql2Dnf (String (sx->cond), String::EMPTY);
                }
                catch( const Exception &e )
                {
                    PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
                        "Exception: selxGetDOCWQL - msg: %s",
                        (const char*)e.getMessage().getCString()));

                    if( rc )
                    {
                        CMSetStatusWithString(
                            rc,
                            CMPI_RC_ERR_FAILED,
                            (CMPIString*)string2CMPIString(e.getMessage()));
                    }
                    if( dnf )
                    {
                        delete dnf;
                    }

                    PEG_METHOD_EXIT();
                    return NULL;
                }
                catch( ... )
                {
                    PEG_TRACE_CSTRING(
                        TRC_CMPIPROVIDERINTERFACE,
                        Tracer::LEVEL1,
                        "Exception: Unknown Exception in selxGetDOC for WQL");
                    delete dnf;
                    CMSetStatus (rc, CMPI_RC_ERR_FAILED);
                    PEG_METHOD_EXIT();
                    return NULL;
                }
                sx->wql_dnf = dnf;
                sx->tableau = sx->wql_dnf->getTableau ();
            }
            sc = (CMPISelectCond *) new CMPI_SelectCond (sx->tableau, 0);
        }
#ifdef PEGASUS_ENABLE_CQL
        if( (strncmp (sx->lang.getCString(),
             CALL_SIGN_CQL, CALL_SIGN_CQL_SIZE) == 0) ||
             (strncmp (sx->lang.getCString(),
             "CIM:CQL", 7) == 0) )
        {
            if( sx->cql_dnf == NULL )
            {
                /* The constructor should set this to a valid pointer. */
                if( sx->_context == NULL )
                {
                    CMSetStatus (rc, CMPI_RC_ERROR_SYSTEM);
                    PEG_METHOD_EXIT();
                    return NULL;
                }

                CQLSelectStatement selectStatement (
                    sx->lang, sx->cond,
                    *sx->_context);
                    CMPI_Cql2Dnf *dnf = NULL;
                try
                {
                    CQLParser::parse (sx->cond, selectStatement);
                    dnf = new CMPI_Cql2Dnf (selectStatement);
                }
                catch( const Exception &e )
                {
                    PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
                        "Exception: selxGetDOCCQL - msg: %s",
                        (const char*)e.getMessage().getCString()));

                    if( rc ) CMSetStatusWithString(rc,CMPI_RC_ERR_FAILED,
                        (CMPIString*)string2CMPIString(e.getMessage()));
                    if( dnf )
                        delete dnf;
                    PEG_METHOD_EXIT();
                    return NULL;
                }
                catch( ... )
                {
                    PEG_TRACE_CSTRING(
                        TRC_CMPIPROVIDERINTERFACE,
                        Tracer::LEVEL1,
                        "Exception: Unknown Exception in selxGetDOC for CQL");
                    delete dnf;
                    CMSetStatus (rc, CMPI_RC_ERR_FAILED);
                    PEG_METHOD_EXIT();
                    return NULL;
                }
                sx->cql_dnf = dnf;
                sx->tableau = sx->cql_dnf->getTableau ();
            }
            sc = (CMPISelectCond *) new CMPI_SelectCond (sx->tableau, 0);
        }
#endif
        if( sc )
        {
            CMSetStatus (rc, CMPI_RC_OK);
            CMPI_Object *obj = new CMPI_Object (sc);
            obj->priv = ((CMPI_SelectCond *) sc)->priv;
            PEG_METHOD_EXIT();
            return reinterpret_cast < CMPISelectCond * >(obj);
        }

        /*
          If the sc was null, we just exit
        */
        CMSetStatus (rc, CMPI_RC_ERR_FAILED);
        PEG_METHOD_EXIT();
        return NULL;
    }

    PEGASUS_STATIC CMPISelectCond *selxGetCOD (
        const CMPISelectExp * eSx,
        CMPIStatus * rc)
    {
        CMSetStatus (rc, CMPI_RC_ERR_NOT_SUPPORTED);
        return NULL;
    }


}

static CMPISelectExpFT selx_FT =
{
    CMPICurrentVersion,
    selxRelease,
    selxClone,
    selxEvaluate,
    selxGetString,
    selxGetDOC,
    selxGetCOD,
    selxEvaluateUsingAccessor
};

CMPISelectExpFT *CMPI_SelectExp_Ftab = &selx_FT;

CMPI_SelectExp::~CMPI_SelectExp()
{
    delete wql_stmt;
    delete wql_dnf;
#ifdef PEGASUS_ENABLE_CQL
    delete cql_dnf;
    delete cql_stmt;
#endif
    delete _context;
}
CMPI_SelectExp::CMPI_SelectExp (
    const OperationContext & ct,
    QueryContext * context,
    String cond_,
    String lang_):ctx (ct),cond (cond_),lang (lang_),
                  _context (context->clone()),persistent(true)
{
    /**
      We do NOT add ourselves to the CMPI_Object as this is a persitent object.
       Look at the other construtors.
     */
    props = NULL;
    ft = CMPI_SelectExp_Ftab;
    wql_dnf = NULL;
    wql_stmt = NULL;
#ifdef PEGASUS_ENABLE_CQL
    cql_stmt = NULL;
    cql_dnf = NULL;
#endif
    tableau = NULL;
}

CMPI_SelectExp::CMPI_SelectExp (
    WQLSelectStatement * st,
    Boolean persistent_):ctx (OperationContext ()),
                         wql_stmt (st), persistent (persistent_)
{
    /**
        Adding the object to the garbage collector.
    */
    if( !persistent_ )
    {
        CMPI_ThreadContext::addObject (reinterpret_cast<CMPI_Object *>(this));
    }
    hdl = NULL;
    ft = CMPI_SelectExp_Ftab;
    props = NULL;
    wql_dnf = NULL;
#ifdef PEGASUS_ENABLE_CQL
    cql_dnf = NULL;
    cql_stmt = NULL;
#endif
    tableau = NULL;
    _context = NULL;
    cond = st->getQuery ();
    lang = CALL_SIGN_WQL;
}

#ifdef PEGASUS_ENABLE_CQL
CMPI_SelectExp::CMPI_SelectExp (CQLSelectStatement * st, Boolean persistent_,
    QueryContext *context):ctx (OperationContext ()),cql_stmt (st),
    _context(context->clone()), persistent (persistent_)
{
    /** Adding the object to the garbage collector.
    */
    if( !persistent_ )
    {
        CMPI_ThreadContext::addObject (reinterpret_cast<CMPI_Object *>(this));
    }
    hdl = NULL;
    ft = CMPI_SelectExp_Ftab;
    props = NULL;
    wql_dnf = NULL;
    cql_dnf = NULL;
    wql_stmt = NULL;
    tableau = NULL;
    cond = st->getQuery ();
    lang = CALL_SIGN_CQL;
    classNames = st->getClassPathList ();
}
#endif
PEGASUS_NAMESPACE_END

