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

#include <string.h>
#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>

static const CMPIBroker *_broker;


#define _ClassName "CMPISample_MethodProviderClass"



/**
    The CMPIMethodMIFT.cleanup() function shall perform any necessary
    cleanup operation prior to the unloading of the library of which
    this MI group is part. This function is called prior to the
    unloading of the provider.
*/
CMPIStatus SampleCMPIMethodProviderMethodCleanup (
    CMPIMethodMI * mi,
    const CMPIContext * ctx,
    CMPIBoolean  term)
{
    CMReturn (CMPI_RC_OK);
}

/**
    Invoke a named, extrinsic method (returnSuccess()) of an Instance
    defined by the &lt;op&gt; parameter.
*/
CMPIStatus SampleCMPIMethodProviderInvokeMethod (
    CMPIMethodMI * mi,
    const CMPIContext * ctx,
    const CMPIResult * rslt,
    const CMPIObjectPath * ref,
    const char *methodName,
    const CMPIArgs * in,
    CMPIArgs * out)
{
    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIString *className;
    CMPIData data;
    char result[80] = "Hello,";
    const char *strCat;
    const char * name;
    char *argName = "Place";
    CMPIString * str1;
    CMPIString * str2;
    CMPIValue val1, val2;
    /* get the class name from object-path */
    className = CMGetClassName(ref, &rc);
    /* get a pointer to a C char* representation of this String. */
    name = CMGetCharsPtr(className, &rc);

    if(!strcmp(name, _ClassName))
    {
       if(!strcmp ("SayHello", methodName))
       {
            /* gets the number of arguments contained in "in" Args. */
            if(CMGetArgCount(in, &rc) > 0)
            {
                /* gets a Name argument value */
                data = CMGetArg(in, "Name", &rc);
                /*check for data type and not null value of argument value
                  recieved */
                if(data.type == CMPI_string &&  !(CMIsNullValue(data)))
                {
                    strCat = CMGetCharsPtr(data.value.string, &rc);
                    strcat(result, strCat);
                    strcat(result, "!");
                    /* create the new string to return to client */
                    str1 = CMNewString(_broker, result, &rc);
                    val1.string = str1;
                }
            }
            else
            {
                str1 = CMNewString(_broker, result, &rc);
                val1.string = str1;
            }
            /* create string to add to an array */
            str2 = CMNewString(_broker, "From Neverland", &rc);
            val2.string = str2;
            /* Adds a value of str2 string to out array argument */
            rc = CMAddArg(out, argName, &val2, CMPI_string);
        }
    }
    CMReturnData (rslt, (CMPIValue *) &val1, CMPI_string);
    CMReturnDone (rslt);
    return rc;
}

/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/


CMMethodMIStub (SampleCMPIMethodProvider,
    CMPISampleMethodProvider,
    _broker,
    CMNoHook)

/* ---------------------------------------------------------------------------*/
/*             end of SampleCMPIProvider                      */
/* ---------------------------------------------------------------------------*/

