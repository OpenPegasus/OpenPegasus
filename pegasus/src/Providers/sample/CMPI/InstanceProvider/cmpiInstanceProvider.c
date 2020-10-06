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

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static const CMPIBroker *_broker;
static CMPIBoolean valid[10];
static CMPICount numOfInst;
static CMPIArray* clone_arr_ptr;
static CMPICount initArraySize = 10;

#define _ClassName "CMPISample_InstanceProviderClass"

/**
    initialize function will create three instances and add them to array.
    "numOfInst" will represent the number of instances in the array.
*/
static void initialize()
{
    CMPIStatus rc = { CMPI_RC_OK, NULL};
    CMPIArray *arr_ptr;
    CMPIValue value1, value2, value_inst1, value_inst2;
    CMPIInstance *instance1, *instance2, *instance3;
    CMPIObjectPath *cop1, *cop2, *cop3;
    CMPIObjectPath *embeddedObjPath;
    CMPIInstance *emInst1;
    CMPIValue valueEmInst;

    value1.uint8 = 1;
    value2.string = CMNewString(_broker, "\\x0C9C\\x0CA8\\x0CCD\\x0CAE"
        "\\x0CA6\\x0CBF\\x0CA8\\x0CA6 \\x0CB6\\x0CC1\\x0CAD\\x0CBE"
        "\\x0CB6\\x0CAF\\x0C97\\x0CB3\\x0CC1", &rc);

    /* Create an embedded instance to be returned as embedded object
       This is an instance of a class that does not exist which
       is legal in DMTF specs.
    */
    embeddedObjPath = CMNewObjectPath (
    _broker,
    "test/TestProvider",
    "TestCMPI_EmbeddedNoClass",
    &rc);

    assert(rc.rc == CMPI_RC_OK);
    valueEmInst.uint32 = 1;

    rc = CMAddKey(embeddedObjPath,"id",&valueEmInst, CMPI_uint32);
    assert(rc.rc == CMPI_RC_OK);
    emInst1 = CMNewInstance(_broker, embeddedObjPath, &rc);

    assert(rc.rc == CMPI_RC_OK);
    valueEmInst.uint32 = 1999;
    rc = CMSetProperty(emInst1, "id", &valueEmInst, CMPI_uint32);

    assert(rc.rc == CMPI_RC_OK);

    // set the new instance into a value container
    valueEmInst.inst = emInst1;

    /* create a new array to hold the instances created */
    arr_ptr = CMNewArray(_broker, initArraySize, CMPI_instance, &rc);

    /* create object path for instance1 */
    cop1 = CMNewObjectPath(
                _broker,
                "root/SampleProvider",
                _ClassName,
                &rc);
    /* add key (with its value) to object path to recognize instance uniquely */
    CMAddKey(cop1, "Identifier", (CMPIValue *)&value1, CMPI_uint8);
    /* if created object path is not null then create new instance */
    if(!CMIsNullObject(cop1))
    {
        instance1 = CMNewInstance(_broker,
                        cop1,
                        &rc);
        /* set the properties for newly created instance */
        CMSetProperty(instance1, "Identifier",&value1, CMPI_uint8);
        CMSetProperty(instance1, "Message", &value2, CMPI_string);
        CMSetProperty(instance1, "emObject", &valueEmInst, CMPI_instance);
        value_inst1.inst = instance1;

        /* assign the created instance to array created */
        rc = CMSetArrayElementAt(
            arr_ptr,
            numOfInst,
            &value_inst1,
            CMPI_instance);
        /* set the validity of instance to be true.
        Validity gets false if instance gets deleted using deleteInstance(). */
        valid[numOfInst] = 1;
        numOfInst++;
    }

    /* create instance 2 and add to array */
    value1.uint8 = 2;
    value2.string = CMNewString(_broker, "\\x0CA8\\x0CBE\\x0CB3"
        "\\x0CC6\\x0CA6\\x0CBF\\x0CA8", &rc);
    cop2 = CMNewObjectPath(
                _broker,
                "root/SampleProvider",
                _ClassName,
                &rc);
    CMAddKey(cop2, "Identifier", (CMPIValue *)&value1, CMPI_uint8);
    if(!CMIsNullObject(cop2))
    {

        instance2 = CMNewInstance(_broker,
                        cop2,
                        &rc);
         CMSetProperty(instance2, "Identifier", &value1, CMPI_uint8);
         CMSetProperty(instance2, "Message", &value2, CMPI_string);
         value_inst2.inst = instance2;
         rc = CMSetArrayElementAt(arr_ptr,
            numOfInst,
            &value_inst2,
            CMPI_instance);
         valid[numOfInst] = 1;
         numOfInst++;
    }

    /* create instance 3 and add to array */
    value1.uint8 = 3;
    value2.string = CMNewString(_broker, "\\x0C9C", &rc);
    cop3 = CMNewObjectPath(
                _broker,
                "root/SampleProvider",
                _ClassName,
                &rc);
    CMAddKey(cop3, "Identifier", (CMPIValue *)&value1, CMPI_uint8);
    if(!CMIsNullObject(cop3))
    {

        instance3 = CMNewInstance(_broker,
                        cop3,
                        &rc);
         CMSetProperty(instance3, "Identifier", &value1, CMPI_uint8);
         CMSetProperty(instance3, "Message", &value2, CMPI_string);
         CMSetProperty(instance3, "emObject", &valueEmInst, CMPI_instance);
         value_inst2.inst = instance3;
         rc = CMSetArrayElementAt(arr_ptr,
            numOfInst,
            &value_inst2,
            CMPI_instance);
         valid[numOfInst] = 1;
         numOfInst++;
    }
    /* clone the array which contains instances. */
    clone_arr_ptr = arr_ptr->ft->clone(arr_ptr, &rc);
}

/**
    The CMPIInstanceMIFT.cleanup() function shall perform any necessary
    cleanup operation prior to the unloading of the library of which
    this MI group is part. This function is called prior to the
    unloading of the provider.
*/

CMPIStatus InstProvCleanup (
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    CMPIBoolean  term)
{
    CMRelease(clone_arr_ptr);
    CMReturn (CMPI_RC_OK);
}

/**
    Enumerate ObjectPaths of Instances serviced by this provider.
*/
CMPIStatus InstProvEnumInstanceNames (
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref)
{
    CMPIStatus rc = {CMPI_RC_OK, NULL};
    unsigned int j = 0;
    CMPIObjectPath *local;
    CMPIData data;
    /* get the element from array containing created instances. */
    for (j = 0; j < numOfInst; j++)
    {
        /*check for validity of Instance, that its not deleted */
        if(valid[j] == 1)
        {
            /* get element(instance) from array */
            data = CMGetArrayElementAt(clone_arr_ptr, j, &rc);
            /* get object-path of that instance */
            local = CMGetObjectPath(data.value.inst, &rc);
            /* return object-path */
            CMReturnObjectPath(rslt,local);
        }
    }
    CMReturnDone (rslt);
    CMReturn (CMPI_RC_OK);
}

/**
    Enumerate the Instances serviced by this provider.
*/
CMPIStatus InstProvEnumInstances (
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * op,
    const char **properties)
{
    CMPIStatus rc = {CMPI_RC_OK, NULL};
    unsigned int j = 0;
    CMPIData data;
    /* get the element from array containing created instances */
    for (j = 0; j < numOfInst; j++)
    {
        /* check for validity of Instance, that its not deleted */
        if(valid[j] == 1)
        {
            /* get element(instance) from array */
            data = CMGetArrayElementAt(clone_arr_ptr, j, &rc);
            /* return the instance */
            CMReturnInstance(rslt, data.value.inst);
        }
    }
    CMReturnDone (rslt);
    CMReturn (CMPI_RC_OK);
}

/**
    Get the Instances defined by object-path op.
*/
CMPIStatus InstProvGetInstance (
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * op,
    const char **properties)
{
    CMPIData data,key1,key2;
    CMPIObjectPath *local;
    unsigned int j = 0;
    CMPIBoolean flag = 0;
    CMPIStatus rc = {CMPI_RC_OK, NULL};
    /* get the kay from object-path */
    key1 = CMGetKey(op, "Identifier", &rc);
    /* get the element from array containing created instances */
    for(j = 0; j < numOfInst; j++)
    {
        /* check for validity of Instance, that its not deleted */
        if (valid[j] == 1)
        {
            /* get element(instance) from array */
            data = CMGetArrayElementAt(clone_arr_ptr, j, &rc);

            /* get object-path of instance */
            local = CMGetObjectPath(data.value.inst, &rc);

            /* get key from this object-path */
            key2 = CMGetKey(local, "Identifier", &rc);

            /* compare key values.
               If they match return instance */
            if (key1.value.uint8 == key2.value.uint8)
            {
                CMReturnInstance(rslt, data.value.inst);
                flag =1;
            }
        }
    }
    /* key values did not match so throw exception */
    if(!flag)
    {
        CMReturn (CMPI_RC_ERR_NOT_FOUND);
    }
    CMReturnDone (rslt);
    CMReturn (CMPI_RC_OK);
}

/**
    Create Instance from inst, using object-path op as reference.
*/
CMPIStatus InstProvCreateInstance (
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const CMPIInstance * ci)
{
    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIInstance * inst;
    CMPIValue value_inst;
    CMPIData key1,key2, retInst;
    CMPIObjectPath *obp;
    unsigned int j = 0;
    if(ci)
    {
        /* clone the instance to be added to the array */
        inst = CMClone(ci, &rc);
        key1 = CMGetProperty(inst, "Identifier", &rc);
        for (j=0; j < numOfInst ; j++)
        {
            /* check for validity of Instance, that its not deleted */
            if (valid[j] == 1)
            {
                /* get element(instance) from array */
                retInst = CMGetArrayElementAt(clone_arr_ptr, j, &rc);
                /* get object-path of instance */
                obp = CMGetObjectPath(retInst.value.inst, &rc);
                /* get key from this object-path */
                key2 = CMGetKey(obp, "Identifier", &rc);
                /*compare key values.
                  If they match throw exception as two instance with same key
                  properties cannot exists. */
                if(key1.value.uint8 == key2.value.uint8)
                {
                    CMReturn (CMPI_RC_ERR_ALREADY_EXISTS);
                }
            }
        }
        value_inst.inst = inst;
        /* If instance doesnot exists in array add it */
        rc = CMSetArrayElementAt(
            clone_arr_ptr,
            numOfInst,
            &value_inst,
            CMPI_instance);
        valid[numOfInst]=1;
        numOfInst++;
        /* return object-path of instance */
        CMReturnObjectPath(rslt, cop);
        CMReturnDone(rslt);
    }
    else
    {
        CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
    }
    CMReturn (CMPI_RC_OK);
}

/**
    Replace an existing Instance from inst, using object-path op as reference.
*/
CMPIStatus InstProvModifyInstance  (
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * cop,
    const CMPIInstance * ci,
    const char **properties)
{
    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIInstance * inst;
    CMPIValue val1, val2;
    CMPIData key1, key2, retData1, retInst;
    CMPIObjectPath *obp;
    unsigned int j = 0, flag = 0;
    if(ci)
    {
        inst = CMClone(ci, &rc);
        /* get key from the object-path */
        key1 = CMGetKey(cop, "Identifier", &rc);
        val1.uint8 = key1.value.uint8;
        /* get the value of Message property */
        retData1 = CMGetProperty(inst, "Message", &rc);
        val2.string = retData1.value.string;
        for (j=0; j < numOfInst; j++)
        {
            /* check for validity of Instance, that its not deleted */
            if (valid[j] == 1)
            {
                /* get element(instance) from array */
                retInst = CMGetArrayElementAt(clone_arr_ptr, j, &rc);
                /* get object-path of instance */
                obp = CMGetObjectPath(retInst.value.inst, &rc);
                /* get key from this object-path */
                key2 = CMGetKey(obp, "Identifier", &rc);
                /*compare key values.
                  If they match then set the properties received from client */
                if(key1.value.uint8 == key2.value.uint8)
                {
                    CMSetProperty(
                        retInst.value.inst,
                        "Message",
                        &val2,
                        CMPI_string);
                    flag = 1;
                }
            }
        }
        CMRelease(inst);
        /*If match fails, throw exception, as instance to be mmodified is not
          found */
        if(!flag)
        {
            CMReturn (CMPI_RC_ERR_NOT_FOUND);
        }
    }
    CMReturnDone (rslt);
    CMReturn (CMPI_RC_OK);
}

/**
    Delete an existing Instance defined by using object-path op as reference.
*/
CMPIStatus InstProvDeleteInstance (
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * result,
    const CMPIObjectPath * cop)
{

    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIData key1, key2, retInst;
    CMPIObjectPath *obp;
    unsigned int j = 0, flag = 0;
    /* get key from the object-path */
    key1 = CMGetKey(cop, "Identifier", &rc);
    for (j=0; j < numOfInst ; j++)
    {
        /*check for validity of Instance, that its not deleted */
        if(valid[j] == 1)
        {
            /*get element(instance) from array */
            retInst = CMGetArrayElementAt(clone_arr_ptr, j, &rc);

            /*get object-path of instance */
            obp = CMGetObjectPath(retInst.value.inst, &rc);

            /*get key from this object-path */
            key2 = CMGetKey(obp, "Identifier" , &rc);

            /*compare key values.
              If they match, release the object(instance)
              Also set its validity to zero, marking it as deleted */
            if (key1.value.uint8 == key2.value.uint8)
            {
                if(retInst.value.inst)
                {
                    flag =1;
                    CMRelease(retInst.value.inst);
                    CMSetArrayElementAt(
                        clone_arr_ptr,
                        j,
                        &retInst.value.inst,
                        CMPI_null);
                    valid[j] = 0;
                }
            }
        }
    }
    if(!flag)
    {
        CMReturn (CMPI_RC_ERR_NOT_FOUND);
    }
    CMReturn (CMPI_RC_OK);
}

/**
    Query the enumeration of instances of the class (and subclasses)
    defined by using object-path op using query expression.
*/
CMPIStatus InstProvExecQuery (
    CMPIInstanceMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * referencePath,
    const char *lang,
    const char *query)
{
    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIStatus rc_Eval = { CMPI_RC_OK, NULL };
    CMPIStatus rc_Clone = { CMPI_RC_OK, NULL };
    CMPIStatus rc_Array = { CMPI_RC_OK, NULL };
    CMPISelectExp *se_def = NULL;
    CMPICount cnt = 0;
    CMPIArray *projection = NULL;
    unsigned int j = 0;
    CMPIBoolean evalRes;
    CMPIData arr_data;
    CMPIInstance *instance1;
    CMPIObjectPath *cop1;
    CMPIData data;
    CMPIValue value1;
    const char* prop_name;
    CMPIData retProp;

    /*create the select expression */
    se_def = CMNewSelectExp (_broker, query, lang, &projection, &rc_Clone);
    if (se_def)
    {
        /*loop over instances in array to evaluate for requested properties */
        for (j = 0; j < numOfInst ; j++)
        {
            /*check for validity of Instance,that its not deleted */
            if(valid[j] == 1)
            {
                /*get the element from array */
                arr_data = CMGetArrayElementAt(
                    clone_arr_ptr,
                    j,
                    &rc);
                /*Evaluate the instance using this select expression */
               evalRes = CMEvaluateSelExp(se_def,arr_data.value.inst, &rc_Eval);
                if (evalRes)
                {
                    /*check if any of properties are requested */
                    if (projection)
                    {
                        /*get number of properties requested */
                        cnt = CMGetArrayCount (projection, &rc_Array);
                        /*if count is not equal to number of properties of
                          instance */
                        if(cnt == 1)
                        {
                            /*check for the properties, requested */
                            data = CMGetArrayElementAt(
                                projection,
                                0,
                                &rc_Array);
                            prop_name = CMGetCharsPtr (
                                data.value.string,
                                &rc);
                            /*create the new instance that has to be returned */
                            cop1 = CMNewObjectPath(
                                _broker,
                                "root/SampleProvider",
                                _ClassName,
                                &rc);

                            instance1 = CMNewInstance(_broker,
                                cop1,
                                &rc);

                            /*if property name is "Identifier",
                               gets its value from instance */
                            if(!strcmp(prop_name, "Identifier"))
                            {
                                retProp = CMGetProperty(
                                    arr_data.value.inst,
                                        "Identifier",
                                        &rc);
                                value1.uint8 = retProp.value.uint8;
                                CMSetProperty(
                                    instance1,
                                    "Identifier",
                                    (CMPIValue *)&value1,
                                    CMPI_uint8);
                            }
                            /*if property name is "Message",
                              gets its value from instance */
                            if(!strcmp(prop_name, "Message"))
                            {
                                retProp = CMGetProperty(
                                    arr_data.value.inst,
                                    "Message",
                                    &rc);
                                    value1.string = retProp.value.string;
                                    CMSetProperty(
                                        instance1,
                                        "Message",
                                        (CMPIValue *)&value1,
                                        CMPI_string);
                            }
                            /*if the query is evaluated return instance */
                            CMReturnInstance (rslt, instance1);
                        }
                    }
                    else
                    {
                        CMReturnInstance(rslt, arr_data.value.inst);
                    }
                }
            }
        }
    }

    CMReturnDone (rslt);
    CMReturn (CMPI_RC_OK);
}


/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/


CMInstanceMIStub(
    InstProv,
    CMPIInstanceProvider,
    _broker,
    initialize())

/* ---------------------------------------------------------------------------*/
/*             end of SampleCMPIProvider                      */
/* ---------------------------------------------------------------------------*/

