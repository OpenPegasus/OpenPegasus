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
// Author: Thilo Boehm (tboehm@de.ibm.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <errno.h>
#define _POSIX_SOURCE
#include <sys/utsname.h>
#include <leawi.h>
#include <ceeedcct.h>
#include <unistd.h>

#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Logger.h>
#if defined(PEGASUS_PLATFORM_ZOS_ZSERIES_IBM)
#include "AutoRestartMgr_ZOS_ZSERIES_IBM.h"
#elif defined(PEGASUS_PLATFORM_ZOS_ZSERIES64_IBM)
#include "AutoRestartMgr_ZOS_ZSERIES64_IBM.h"
#endif
#include "ARM_zOS.h"


// ARM status for CIM Server Status
// __arm_status_tags ARM_zOS_Status = NOT_REGISTERED;

#define ARM_ELEMNAME_SIZE 17          // 16 Bytes for ARM element name + '\0'

PEGASUS_NAMESPACE_BEGIN


//******************************************************************************
//
// Methode: void ARM_zOS::Register(void)
//
// Description:
// Registration and make element READY for z/OS ARM (Automatic Restart Manager).
//
// Function:
//            a) do the registration with ARM
//            b) make the element READY for ARM
//
//    All errors are handled inside this routine, a failure to register or
//    a failure to make READY the element does not impact the further function
//    of the CIM Server. The success and failures are written to the
//    z/OS console and the appropriate CIM Server logs.
//
//    The registration status is saved in the global variable ARM_zOS_Status
//    for further use ( e.g. actions after a restart) within the CIM Server.
//
//******************************************************************************
void ARM_zOS::Register(void)
{

    int i;

    // ARM element name init with base.
    char arm_elemname[ARM_ELEMNAME_SIZE] = "CFZ_SRV_";
    int full_elemname_size;

    char arm_buffer[256];               // ARM buffer
    int arm_ret=0;                      // ARM return code
    int arm_res=0;                      // ARM reason code
    struct utsname uts;                 // structure to get the SYSNAME for
                                        // the element name
    int rc;

#ifndef PEGASUS_PLATFORM_ZOS_ZSERIES64_IBM
    rc = __osname(&uts);
    if(rc < 0)
    {
        PEG_TRACE((TRC_SERVER, Tracer::LEVEL1,
                   "Failed to issue __osname() command:"
                   " rc=%d, errno=%d, reason=%08X",
                   rc,
                   errno,
                   __errno2()));
        ARM_zOS_Status = NOT_REGISTERED;
        return;
    }

    // concatenate the element name base and the nodename to the full element
    // name
    strcat(arm_elemname,uts.nodename);
    full_elemname_size = strlen(arm_elemname);

    // pad the elementname with spaces
    memset(arm_elemname + full_elemname_size, ' ',
        ARM_ELEMNAME_SIZE - full_elemname_size);


    // put a Null-termination at Pos 17 to make string printable
    arm_elemname[ARM_ELEMNAME_SIZE-1] = '\0';


    PEG_TRACE((TRC_SERVER, Tracer::LEVEL3,
        "About to register the CIM server with element name \'%s\' with ARM.",
        arm_elemname));

    // CIM server is running in ASCII and the assembler module needs the
    // module in EBCDIC
    __atoe(arm_elemname);

    __register_arm(arm_elemname, arm_buffer, &arm_ret, &arm_res);

    // convert back to ascii for further processing.
    __etoa(arm_elemname);

    if (arm_ret <= 0x04)
    {
       Logger::put_l(Logger::STANDARD_LOG, "CIM Server", Logger::INFORMATION,
           MessageLoaderParms(
               "Common.ARM_zOS.ARM_READY",
               "The CIM server successfully registered to ARM using element "
                   "name $0.",
               arm_elemname));

        if (arm_ret == 0x00)
        {
            // ARM registration at normal startup
            ARM_zOS_Status = REGISTERED;
        }
        else
        {
            // ret = 0x04 and reason code 0x104 & 0x108 indicates server restart
            if(arm_res <= 0x108)
            {
                // ARM registration at restart
                ARM_zOS_Status = RESTARTED;
            }
            else
            {
                // ret = 0x04 and reason code 0x204 & 0x304 indicates dependent
                // component not started.  This can only be happen if somebody
                // define dependencies in the restart policy.
                // CIM Server does not depend on other elements; This is a
                // normal start up.
                ARM_zOS_Status = REGISTERED;
            }

        } // end evaluate return codes.

        // set CIM server ready with ARM
        __ready_arm(arm_buffer, &arm_ret, &arm_res);
        if(arm_ret != 0)
        {
            PEG_TRACE((TRC_SERVER, Tracer::LEVEL2,
                "Failed to set the CIM server ready with ARM: "
                    "ret=%02X reason=%04X.",
                arm_ret,
                arm_res));
            ARM_zOS_Status = NOT_REGISTERED;
        }
    }
    else
    {
        // ARM registration fails
        char str_arm_ret[4];
        sprintf(str_arm_ret,"%02X",arm_ret);
        char str_arm_res[8];
        sprintf(str_arm_res,"%04X",arm_res);
        Logger::put_l(Logger::STANDARD_LOG, "CIM Server", Logger::INFORMATION,
            MessageLoaderParms(
                "Common.ARM_zOS.ARM_FAIL",
                "The CIM server failed to register with ARM using element "
                    "name $0: return code 0x$1, reason code 0x$2.",
                arm_elemname,
                str_arm_ret,
                str_arm_res));

        ARM_zOS_Status = NOT_REGISTERED;
    }
#endif
    return;
}


//******************************************************************************
//
// Method: void ARM_zOS::DeRegister(void)
//
// Description:
// De-Register CIM Server from z/OS ARM (Automatic Restart Mangager).
//
// Function:
//            de-registration with ARM
//
//    All errors are handled inside this routine.
//
//
//******************************************************************************

void ARM_zOS::DeRegister(void)
{
    char arm_buffer[256];
    int arm_ret=0;                     // ARM return code
    int arm_res=0;                     // ARM reason code

#ifndef PEGASUS_PLATFORM_ZOS_ZSERIES64_IBM
    // If CIM Server is not not registeres -> if it is REGISTERED or RESTARTED.
    if(ARM_zOS_Status != NOT_REGISTERED)
    {
        __deregister_arm(arm_buffer, &arm_ret, &arm_res);
        if(arm_ret != 0)
        {
            // write out errormessage from de-register with ARM
            PEG_TRACE((TRC_SERVER, Tracer::LEVEL2,
                "Failed to deregister CIM Server with ARM: "
                    "ret=%02X reason=%04X.",
                arm_ret,
                arm_res));
        }
        else
        {
            PEG_TRACE_CSTRING(TRC_SERVER, Tracer::LEVEL3,
                "CIM Server sucessfully deregistered with ARM.");
        }
        ARM_zOS_Status = NOT_REGISTERED;
    }// End if
#endif
}

PEGASUS_NAMESPACE_END
