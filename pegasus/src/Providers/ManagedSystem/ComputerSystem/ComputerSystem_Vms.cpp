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
//%////////////////////////////////////////////////////////////////////////////

//  12-Feb-08
// PTR 73-51-124,Made  changes to display the primary owner name and contact
// number in getPrimaryOwnerName() & getPrimaryOwnerContact()  .
// PTR 73-51-123,Made changes to UUID display format in
// getIdentificationNumber().
// PTR 73-51-122 ,Made changes to system model display format in
// getOtherIdentifyingInfo().
//  22-Aug-06
//  PTR 73-51-30 , Made changes to replace sys$specific:[000000]
//  with "wbem_tmp:" logical in getInstallDate().
//
//  11-July-06
//  PTR 73-51-24 , Made changes to getInstallDate() and convertToCIMDateString()
//  functions to display InstallDate()
//
//  11-July-06
//  PTR 73-51-23, Changes made to get the correct string values to be displayed
//  for Caption , Description, PrimaryOwner, PrimaryOwnerContact,
//  PrimaryOwnerPager, InitialLoadInfo, SecondaryOwnerName,
//  SecondaryOwnerContact and SecondaryOwnerPager properties
//
//  28-Jun-06
//  PTR 73-51-4, 73-51-6 and PTR 73-51-8, changes made to use temp file
//  instead of hsitory.out in GetInstallDate()
//
//  28-Jun-06
//  PTR 73-51-16, changes made to obtaine correct serial number on I64
//
///////////////////////////////////////////////////////////////////////////////

#include "ComputerSystemProvider.h"
#include "ComputerSystem.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <descrip.h>
#include <syidef.h>
#include <jpidef.h>
#include <pscandef.h>
#include <lib$routines.h>
#include <starlet.h>
#include <stsdef.h>
#include <ssdef.h>
#include <libdtdef.h>
#include <lnmdef.h>
#include <netdb.h>
#include <opcdef.h>
#include <psldef.h>
#include <stddef.h>
#include <time.h>
#include <tis.h>
#include <pthread.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/System.h>

// Verify that /main=posix_exit is set. This changes the return status values
// of RTL calls such as system().
#ifndef _POSIX_EXIT
#error "The compile switch /main=posix_exit is required."
#endif

// Changed to Fix PTR -73-51-16

#ifdef  PEGASUS_PLATFORM_VMS_IA64_DECCXX
#define hwrpb$b_sys_serialnum 72
#else
#define hwrpb$b_sys_serialnum 64
#endif

#define MAXHOSTNAMELEN 256

extern "C" {
extern const long EXE$GPQ_HWRPB;
}

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

// Defines used in the file
#define TIMEOUT 360000
// The default OperationalStatus
#define OPENVMS_DEFAULT_OPERATIONAL_STATUS 2
// The default status description
#define OPENVMS_DEFAULT_STATUS_DESCRIPTIONS \
    "One or more components that make up this computer system have an " \
    "OperationalStatus value of OK or Completed."

typedef struct {
    uint32_t  time_low;
    uint16_t  time_mid;
    uint16_t  time_hi_and_version;
    uint8_t   clock_seq_hi_and_reserved;
    uint8_t   clock_seq_low;
    uint8_t   node[6];
}uuid_t;

void translateLogical(char *logical,
                      char *translatedName,
                      int translatedNameSize)
{
    typedef struct descrip
    {
        unsigned short dsc$w_length;   // specific to descriptor class
        unsigned char  dsc$b_dtype;    // data type code
        unsigned char  dsc$b_class;    // descriptor class code
        const char    *dsc$a_pointer; // address of first byte of data element
    } DESCRIP;

    typedef struct itmlst
    {
        unsigned short wItmLen;
        unsigned short wItmCod;
        const void *pItmBuf;
        unsigned short *pItmRetLen;
    } ITMLST;

    DESCRIP dsName = {0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0};
    DESCRIP dsTable = {0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0};

    char szResponse[LNM$C_NAMLENGTH+1];
    unsigned short int wRespLen = 0;

    ITMLST itmGetLnm [2] = {sizeof(szResponse) -1,
        LNM$_STRING, &szResponse, &wRespLen, 0, 0, NULL, NULL};

    if (NULL == logical || NULL == translatedName || 0 == translatedNameSize)
    {
        return;
    }

    // point the local descriptor to the logical name and translate it
    dsName.dsc$w_length  = strlen(logical);
    dsName.dsc$a_pointer = logical;
    dsTable.dsc$a_pointer = "LNM$SYSTEM_TABLE";
    dsTable.dsc$w_length  = strlen(dsTable.dsc$a_pointer);

    unsigned char byAcMode = PSL$C_EXEC;
    unsigned long ulStatus = sys$trnlnm (0,
                                 &dsTable,
                                 &dsName,
                                 &byAcMode,
                                 itmGetLnm);

    if( ulStatus != SS$_NORMAL )
    {
       if( ulStatus == SS$_NOLOGNAM )
       {
          translatedName[0]='\0';
          return;
       }
       else if( ulStatus == SS$_NOPRIV )
       {
          translatedName[0]='\0';
          return;
       }
       else
       {
          translatedName[0]='\0';
          return;
       }
    }


    // "If an equivalence name does not exist... $TRNLNM returns the value 0
    // in the return length address field of the item descriptor."

    szResponse[wRespLen] = '\0';    // append null terminator
    strncpy(translatedName, szResponse, translatedNameSize);
}

static void updateStatusAndDescription();

static String _hostName;
static String _otherInfo;
static String _serialNumber;
static String _status;
static Array<Uint16> _operationalStatus;
static Array<String> _statusDescriptions;

ComputerSystem::ComputerSystem()
{
}

ComputerSystem::~ComputerSystem()
{
}


Boolean ComputerSystem::getCaption(CIMProperty& p)
{
    // hardcoded
    // Changed to address PTR 73-51-23
    p = CIMProperty(PROPERTY_CAPTION,String(CAPTION));
    return true;
}

Boolean ComputerSystem::getDescription(CIMProperty& p)
{
    // hardcoded
    // Changed to address PTR 73-51-23
    p = CIMProperty(PROPERTY_DESCRIPTION,String(DESCRIPTION));
    return true;
}

/*
   convertToCIMDateString converts a tm struct to a CIMDateTime formatted
   char *. -1 is retuned on  error, 1 otherwise.
*/

int convertToCIMDateString(struct tm *t, char **time)
{
    // Format the date.
    // Changed to address PTR 73-51-24
    (*time) = (char *)malloc(80);
    sprintf(*time,"%04d%02d%02d%02d%02d%02d.000000%c%03d",
            t->tm_year + 1900,
            t->tm_mon + 1,
            t->tm_mday,
            t->tm_hour,
            t->tm_min,
            t->tm_sec,
            (timezone>0)?'-':'+',
            abs(timezone/60 - ( t->tm_isdst? 60:0 )));

    return 1;
}

Boolean ComputerSystem::getInstallDate(CIMProperty& p)
{
    Boolean bStatus;
    int status, istr;
    char record1[512], *rptr1=0;
    // Added to address PTR 73-51-4, 73-51-6 and 73-51-8
    char  HistFileBuffer[512];
    pthread_t thread_num;
    char  cmd[512];
    // end of addition

    FILE *fptr1=0;
    unsigned __int64 bintime=0;
    unsigned short int timbuf[7], val=0;

    // Changed to address PTR 73-51-24
    char *cimtime;
    struct tm timetm;
    struct tm *ptimetm=&timetm;
    time_t tme=0, tme1=0;
    char t_string[24]="", libdst;
    unsigned int retlen;
    unsigned long libop, libdayweek, libdayear;
    long dst_desc[2];
    char log_string[]="SYS$TIMEZONE_DAYLIGHT_SAVING";
    struct dsc$descriptor_s sysinfo;
    static $DESCRIPTOR(lnm_tbl,"LNM$SYSTEM");
    struct {
        unsigned short wLength;
        unsigned short wCode;
        void*    pBuffer;
        unsigned int* pRetLen;
        int term;
    } item_list;

    thread_num = tis_self();
    sprintf(HistFileBuffer, "%s%s%x%s", "wbem_tmp:", "cshist_",
            thread_num->sequence,
            ".out");

    sysinfo.dsc$b_dtype=DSC$K_DTYPE_T;
    sysinfo.dsc$b_class=DSC$K_CLASS_S;
    sysinfo.dsc$w_length=sizeof(t_string);
    sysinfo.dsc$a_pointer=t_string;

    strcpy(cmd, "pipe product show history openvms | search/nolog/nowarn/out=");
    strcat(cmd, HistFileBuffer);
    strcat(cmd, " sys$input install");

    status = system(cmd);
    if (!WIFEXITED(status))
    {
        bStatus = false;
        perror("system()");
        goto done;
    }

    fptr1 = fopen(HistFileBuffer, "r");
    if (fptr1)
    {
        while (fgets(record1, sizeof(record1), fptr1))
        {
            for (istr=0; istr<=(sizeof(record1)-4); istr++)
            {
                if ((rptr1 = strstr(record1+istr,"-")) &&
                    !strncmp(rptr1+4,"-",1))
                {
                    break;
                }
                rptr1 = 0;
            }

            //Changed to address PTR 73-51-24
            if (rptr1)
            {
                time(&tme);
                tme1 = mktime(ptimetm); /* get timezone */
                strcpy(t_string, rptr1 - 2);

                if (t_string[11] == 10)
                {
                    // a <cr>.
                    // When the date; but not the time is provided,
                    // fill in zeros.
                    t_string[11] = ' ';
                    t_string[12] = '0';
                    t_string[13] = '0';
                    t_string[14] = ':';
                    t_string[15] = '0';
                    t_string[16] = '0';
                    t_string[17] = ':';
                    t_string[18] = '0';
                    t_string[19] = '0';
                }
                t_string[20] = '.';
                t_string[21] = '0';
                t_string[22] = '0';
                t_string[23] = '0';

                status = sys$bintim (&sysinfo, &bintime);
                if (!$VMS_STATUS_SUCCESS(status))
                {
                    bStatus = false;
                    goto done;
                }


                libop=LIB$K_DAY_OF_WEEK;
                status=lib$cvt_from_internal_time (&libop,&libdayweek,&bintime);
                if (!$VMS_STATUS_SUCCESS(status))
                {
                    bStatus = false;
                    goto done;
                }


                libop=LIB$K_DAY_OF_YEAR;
                status=lib$cvt_from_internal_time (&libop,&libdayear,&bintime);
                if (!$VMS_STATUS_SUCCESS(status))
                {
                    bStatus = false;
                    goto done;
                }


                dst_desc[0]  = strlen(log_string);
                dst_desc[1]  = (long) log_string;
                item_list.wLength = 1;
                item_list.wCode = LNM$_STRING;
                item_list.pBuffer = &libdst;
                item_list.pRetLen = &retlen;
                item_list.term =0;

                status = sys$trnlnm (0,&lnm_tbl,&dst_desc,0,&item_list);
                if (!$VMS_STATUS_SUCCESS(status))
                {
                    bStatus = false;
                    goto done;
                }

                status = sys$numtim(timbuf,&bintime);
                if (!$VMS_STATUS_SUCCESS(status))
                {
                    bStatus = false;
                    goto done;
                }


                timetm.tm_sec = timbuf[5];
                timetm.tm_min = timbuf[4];
                timetm.tm_hour = timbuf[3];
                timetm.tm_mday = timbuf[2];
                timetm.tm_mon = timbuf[1]-1;
                timetm.tm_year = timbuf[0]-1900;
                timetm.tm_wday = libdayweek-1;
                timetm.tm_yday = libdayear-1;
                timetm.tm_isdst = 0;
                if (libdst != 48)
                {
                    timetm.tm_isdst = 1;
                }

                //Changed to address PTR 73-51-24
                status = convertToCIMDateString(ptimetm,&cimtime);
                if (!$VMS_STATUS_SUCCESS(status))
                {
                    bStatus = false;
                    goto done;
                }

                CIMDateTime _installDate(cimtime);
                p = CIMProperty(PROPERTY_INSTALL_DATE, _installDate);

                bStatus = true;
                goto done;

            } // end if (rptr1 = strstr(record1,"Install"))
        }
        bStatus = false;
        goto done;

    } // end if (fptr1 = fopen(history.out, "r"))
    else
    {
        perror("fopen cshist_");
        bStatus = false;
        goto done;
    }

// Added to address PTR 73-51-4, 73-51-6 and 73-51-8
done:

    if (fptr1)
    {
        fclose(fptr1);
        fptr1 = 0;
    }

    remove(HistFileBuffer);

    return bStatus;
}

Boolean ComputerSystem::getCreationClassName(CIMProperty& p)
{
    // can vary, depending on class
    p = CIMProperty(PROPERTY_CREATION_CLASS_NAME,
                    String(CLASS_CIM_UNITARY_COMPUTER_SYSTEM));
    return true;
}

Boolean ComputerSystem::getName(CIMProperty& p)
{
    p = CIMProperty(PROPERTY_NAME,_hostName);
    return true;
}

Boolean ComputerSystem::getStatus(CIMProperty& p)
{
    updateStatusAndDescription();
    // return status string for this property
    p = CIMProperty(PROPERTY_STATUS, _status);
    return true;
}

Boolean ComputerSystem::getOperationalStatus(CIMProperty& p)
{
    updateStatusAndDescription();
    // return operational status array for this property
    p = CIMProperty(PROPERTY_OPERATIONAL_STATUS, _operationalStatus);
    return true;
}

Boolean ComputerSystem::getStatusDescriptions(CIMProperty& p)
{
    updateStatusAndDescription();
    // return status descriptions array for this property
    p = CIMProperty(PROPERTY_STATUS_DESCRIPTIONS, _statusDescriptions);
    return true;
}

Boolean ComputerSystem::getNameFormat(CIMProperty& p)
{
    // hardcoded
    p = CIMProperty(PROPERTY_NAME_FORMAT,String(NAME_FORMAT));
    return true;
}

Boolean ComputerSystem::getPrimaryOwnerName(CIMProperty& p)
{
    // Changed to Adress PTR 73-51-124
    char translated_name[260]="\0";
    Array<String> s;

    // PRIMARY OWNER  details getting from  logical "WBEM_PRIMARY_OWNER"
    // Calling the translateLogical() function to tranlate logical name
    // to exact value

    translateLogical("WBEM_PRIMARY_OWNER", translated_name,256);
    if(translated_name[0] != '\0')
    {
        s.append(translated_name);
        p = CIMProperty(PROPERTY_PRIMARY_OWNER_NAME,s);
    }
    else
    {
        // hardcoded
        // Changed to Adress PTR 73-51-23
        p = CIMProperty(PROPERTY_PRIMARY_OWNER_NAME,String(""));
    }

    return true;
}

Boolean ComputerSystem::setPrimaryOwnerName(const String&)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getPrimaryOwnerContact(CIMProperty& p)
{
    // Changed to Adress PTR 73-51-124
    char translated_value[260]="\0";
    Array<String> s;

    // PRIMARY OWNER CONTACT details getting from the
    // logical "WBEM_PRIMARY_OWNER_CONTACT"
    // Calling the translateLogical() function to tranlate logical name
    // to exact value
    translateLogical("WBEM_PRIMARY_OWNER_CONTACT", translated_value,256);
    if(translated_value[0] != '\0')
    {
        s.append(translated_value);
        p = CIMProperty(PROPERTY_PRIMARY_OWNER_CONTACT,s);
    }
    else
    {
        // "WBEM_PRIMARY_OWNER_CONTACT" is not defined
        // hardcoded
        // Changed to Adress PTR 73-51-23
        p = CIMProperty(PROPERTY_PRIMARY_OWNER_CONTACT,String(""));
    }

    return true;
}

Boolean ComputerSystem::setPrimaryOwnerContact(const String&)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getRoles(CIMProperty& p)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getOtherIdentifyingInfo(CIMProperty& p)
{
    // return model for this property

    int status;
    char *temp_hwname =NULL,*temp =NULL;
    Array<String> s;
    char hwname[32];
    typedef struct {
        unsigned short wlength;
        unsigned short wcode;
        void *pbuffer;
        void *pretlen;
    } item_list;
    item_list itmlst3[2];

    itmlst3[0].wlength = sizeof(hwname);
    itmlst3[0].wcode = SYI$_HW_NAME;
    itmlst3[0].pbuffer = hwname;
    itmlst3[0].pretlen = NULL;
    itmlst3[1].wlength = 0;
    itmlst3[1].wcode = 0;
    itmlst3[1].pbuffer = NULL;
    itmlst3[1].pretlen = NULL;

    // Hardware name we will get like this "HP rx3600  (1.40GHz/6.0MB)"
    status = sys$getsyiw (0, 0, 0, itmlst3, 0, 0, 0);
    if ($VMS_STATUS_SUCCESS(status))
    {
        // Changed to Adress PTR 73-51-122
#ifdef  PEGASUS_PLATFORM_VMS_IA64_DECCXX
        _otherInfo.assign(hwname);
        //fixing the PTR 73-51-122
        //experted hardware name is like this "HP rx3600"
        temp_hwname=&hwname[3];    //pointing the 4th charecter of string
        //Searching for space from 4th charecter of the string and putting '\0'
        temp=strchr(temp_hwname,' ');
        if (NULL != temp)
        {
            *temp = '\0';
        }
        temp=strchr(temp_hwname,'\t');
        if (NULL != temp)
        {
            *temp = '\0';
        }
        _otherInfo.assign(temp_hwname);
        _otherInfo = String("ia64 hp server ") + _otherInfo;
#else
        _otherInfo.assign(hwname);
#endif
        s.append(_otherInfo);
        p = CIMProperty(PROPERTY_OTHER_IDENTIFYING_INFO,s);
        return true;
    }
    else
    {
        return false;
    }
}

Boolean ComputerSystem::getIdentifyingDescriptions(CIMProperty& p)
{
    // hardcoded
    Array<String> s;
    s.append("Model");
    p = CIMProperty(PROPERTY_IDENTIFYING_DESCRIPTIONS,s);
    return true;
}

Boolean ComputerSystem::getDedicated(CIMProperty& p)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getResetCapability(CIMProperty& p)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getPowerManagementCapabilities(CIMProperty& p)
{
    // hardcoded
    Array<Uint16> s;
    s.append(1);
    p = CIMProperty(PROPERTY_POWER_MANAGEMENT_CAPABILITIES, s);
    return true;
}

Boolean ComputerSystem::getInitialLoadInfo(CIMProperty& p)
{
    long status, dst_desc[2];
    char log_string[]="SYS$SYSDEVICE";
    char res_string[256]="", *ptr1=0, *ptr2=0;
    unsigned int retlen;
    static $DESCRIPTOR(lnm_tbl,"LNM$SYSTEM");
    struct {
        unsigned short wLength;
        unsigned short wCode;
        void*    pBuffer;
        unsigned int* pRetLen;
        int term;
    } item_list;

    dst_desc[0]  = strlen(log_string);
    dst_desc[1]  = (long) log_string;
    item_list.wLength = sizeof(res_string);
    item_list.wCode = LNM$_STRING;
    item_list.pBuffer = res_string;
    item_list.pRetLen = &retlen;
    item_list.term =0;

    status = sys$trnlnm (0,&lnm_tbl,&dst_desc,0,&item_list);
    if ($VMS_STATUS_SUCCESS(status))
    {
        ptr1 = res_string;
        ptr2 = strchr(ptr1,':');
        if (ptr2) res_string[ptr2-ptr1] = '\0';
        // Changed to Adress PTR 73-51-23
        p = CIMProperty(PROPERTY_INITIAL_LOAD_INFO, String(res_string));
        return true;
    }
    else
    {
        // Changed to Adress PTR 73-51-23
        p = CIMProperty(PROPERTY_INITIAL_LOAD_INFO, String("Unknown"));
        return false;
    }
}

Boolean ComputerSystem::getLastLoadInfo(CIMProperty& p)
{
    // ATTN: not sure yet
    return false;
}

Boolean ComputerSystem::getPowerManagementSupported(CIMProperty& p)
{
    // hardcoded
    p = CIMProperty(PROPERTY_POWER_MANAGEMENT_SUPPORTED,false); // on PA-RISC!!
    return true;
}

Boolean ComputerSystem::getPowerState(CIMProperty& p)
{
// hardcoded
/*
    ValueMap {"1", "2", "3", "4", "5", "6", "7", "8"},
    Values {"Full Power", "Power Save - Low Power Mode",
            "Power Save - Standby", "Power Save - Other",
            "Power Cycle", "Power Off", "Hibernate", "Soft Off"}
*/
    p = CIMProperty(PROPERTY_POWER_STATE,Uint16(1));  // Full Power on PA-RISC!!
    return true;
}

Boolean ComputerSystem::getWakeUpType(CIMProperty& p)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getPrimaryOwnerPager(CIMProperty& p)
{
    // hardcoded
    // Changed to Adress PTR 73-51-23
    p = CIMProperty(PROPERTY_PRIMARY_OWNER_PAGER,String(""));
    return true;
}

Boolean ComputerSystem::setPrimaryOwnerPager(const String&)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getSecondaryOwnerName(CIMProperty& p)
{
    // hardcoded
    // Changed to Adress PTR 73-51-23
    p = CIMProperty(PROPERTY_SECONDARY_OWNER_NAME,String(""));
    return true;
}

Boolean ComputerSystem::setSecondaryOwnerName(const String&)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getSecondaryOwnerContact(CIMProperty& p)
{
    // hardcoded
    // Changed to Adress PTR 73-51-23
    p = CIMProperty(PROPERTY_SECONDARY_OWNER_CONTACT,String(""));
    return true;
}

Boolean ComputerSystem::setSecondaryOwnerContact(const String&)
{
    // not supported
    return false;
}

Boolean ComputerSystem::getSecondaryOwnerPager(CIMProperty& p)
{
    // hardcoded
    // Changed to Adress PTR 73-51-23
    p = CIMProperty(PROPERTY_SECONDARY_OWNER_PAGER,String(""));
    return true;
}

Boolean ComputerSystem::setSecondaryOwnerPager(const String&)
{
    // not supported
    return false;
}

int GetSerNum (char *pSerNum)
{
    void *phwrpb;

    phwrpb = (void *) ((int)EXE$GPQ_HWRPB+hwrpb$b_sys_serialnum);
    strncpy(pSerNum,(char*)phwrpb,16);
    return(SS$_NORMAL);
}

Boolean ComputerSystem::getSerialNumber(CIMProperty& p)
{
    long status = SS$_NORMAL, i;
    char lrSerNum[16]="";
    char lSerNum[16]="";
    struct k1_arglist {                 // kernel call arguments
        long lCount;                    // number of arguments
        char *pSerNum;
    } getsernumkargs = {1};             // init 1 argument

    getsernumkargs.pSerNum = lrSerNum;

    status = sys$cmkrnl(GetSerNum,&getsernumkargs);
    if ($VMS_STATUS_SUCCESS(status))
    {
        for (i=0;i<strlen(lrSerNum);i++)
        {
// Changed to Fix PTR 73-51-16
#ifdef  PEGASUS_PLATFORM_VMS_IA64_DECCXX
            lSerNum[i] = lrSerNum[i];
#else
            lSerNum[strlen(lrSerNum)-i-1] = lrSerNum[i];
#endif
        }
        _serialNumber.assign(lSerNum);
        p = CIMProperty(PROPERTY_SERIAL_NUMBER, _serialNumber);
        return true;
    }

    return false;
}

/*
* Function:
*      system_uuid- for getting the system UUID
* Inputs:
*       structure uuid_t for filling the system UUID
* Outputs:
*       system UUID in uuid_t structure format .
*
* Returns:
*       Success 1 or failure 0
* Notes:
*       none
*/

int system_uuid(uuid_t *uuid_system)
{
    int status;
    int i;

    typedef struct {
        unsigned short wlength;
        unsigned short wcode;
        void *pbuffer;
        void *pretlen; } item_list;

    item_list itmlst3[2];

    itmlst3[0].wlength = sizeof(uuid_t);
    itmlst3[0].wcode = SYI$_SYSTEM_UUID;
    itmlst3[0].pbuffer = (char*) uuid_system;
    itmlst3[0].pretlen = NULL;
    itmlst3[1].wlength = 0;
    itmlst3[1].wcode = 0;
    itmlst3[1].pbuffer = NULL;
    itmlst3[1].pretlen = NULL;

    status = sys$getsyiw (0, 0, 0, itmlst3, 0, 0, 0);

    if ($VMS_STATUS_SUCCESS(status))
    {
         //Putting the required bytes only
        uuid_system->time_low=uuid_system->time_low & 0XFFFFFFFF;
        uuid_system->time_mid =uuid_system->time_mid& 0XFFFF;
        uuid_system->time_hi_and_version =
            uuid_system->time_hi_and_version & 0XFFFF;
        uuid_system->clock_seq_hi_and_reserved =
            uuid_system->clock_seq_hi_and_reserved & 0XFF;
        uuid_system->clock_seq_low= uuid_system->clock_seq_low & 0XFF;
        for(i=0; i < 6; i++)uuid_system->node[i]=uuid_system->node[i] & 0XFF;
        return 1;
    }

    return 0;
}

Boolean ComputerSystem::getIdentificationNumber(CIMProperty& p)
{

    int status;
    String uidStr;
    char uidBuffer[36];
    uuid_t *systemUUID;
    systemUUID =(uuid_t *) calloc(1,sizeof(uuid_t));

    if (systemUUID ==NULL )
    {
        return false;
    }

    // calling  system_uuid function with passing pointer of stucture
    // "uuid_t" as a argument
    // getting the system UUID in the format of stucture "uuid_t"
    status = system_uuid(systemUUID);
    if(status ==1)        //checking the status for success
    {
       // Putting the UUID in uidBuffer with expected
       // format "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"

        sprintf(uidBuffer,"%.8X-%.4X-%.4X-%.2X%.2X-%.2X%.2X%.2X%.2X%.2X%.2X",
            systemUUID->time_low,
            systemUUID->time_mid,
            systemUUID->time_hi_and_version,
            systemUUID->clock_seq_hi_and_reserved,
            systemUUID->clock_seq_low,
            systemUUID->node[0],
            systemUUID->node[1],
            systemUUID->node[2],
            systemUUID->node[3],
            systemUUID->node[4],
            systemUUID->node[5]);

        uidStr.assign(uidBuffer);
        p = CIMProperty(PROPERTY_IDENTIFICATION_NUMBER, uidStr);
        free(systemUUID);
        return true;
    }
    else
    {
        //faild the system_uuid() function to fill the UUID in the structer
        free(systemUUID);
        return false;
    }
}

Boolean ComputerSystem::getElementName(CIMProperty& p)
{
    // We're just going to re-use the caption
    p = CIMProperty(PROPERTY_ELEMENTNAME, String(CAPTION));
    return true;
}

void ComputerSystem::initialize(void)
{
    char hostName[PEGASUS_MAXHOSTNAMELEN + 1];
    struct addrinfo *info, hints;
    int rc;


    if (gethostname(hostName, sizeof(hostName)) != 0)
    {
        _hostName.assign("Unknown");
        return;
    }
    hostName[sizeof(hostName)-1] = 0;

    // Now get the official hostname.  If this call fails then return
    // the value from gethostname().
    // Note: gethostbyname() is not reentrant and VMS does not
    // have gethostbyname_r() so use getaddrinfo().

    info = 0;
    memset (&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_CANONNAME;
    hints.ai_family = AF_INET;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;

    rc = System::getAddrInfo(hostName, 0, &hints, &info);
    if ((!rc) && (info) && (info->ai_canonname))
    {
        // Note: if assign throws an exception, freeaddrinfo is not called.
        _hostName.assign(info->ai_canonname);
    }
    else
    {
        _hostName.assign(hostName);
    }

    if (info)
    {
        freeaddrinfo(info);
    }

    return;
}

String ComputerSystem::getHostName(void)
{
    return _hostName;
}
// Function to get the _status, _operationalStatus, _statusDescriptions
// by doing an enumerate instance on HP_HealthState
static void updateStatusAndDescription()
{
    Boolean useDefaultStatus = true;
    static time_t previousTime = 0;
    static time_t currentTime = 0;
    int diffTime = 0;

    // If request has come within 1 minute interval, do not refresh
    time (&currentTime);
    diffTime = currentTime - previousTime;
    if (diffTime >= 0 && diffTime <= 60)
    {
        return;
    }
    else
    {
        previousTime = currentTime;
    }

    try {
        Array<CIMObjectPath> instanceNames;

        CIMInstance returnedInstance;
        Array <CIMName> propertyNames;
        propertyNames.append (PROPERTY_STATUS);
        propertyNames.append (PROPERTY_OPERATIONAL_STATUS);
        propertyNames.append (PROPERTY_STATUS_DESCRIPTIONS);
        CIMPropertyList propertyList;
        propertyList.set (propertyNames);

        _status.clear();
        _operationalStatus.clear();
        _statusDescriptions.clear();

        CIMValue status;
        CIMValue operationalStatus;
        CIMValue statusDescriptions;

        CIMClient hpcsclient;
        hpcsclient.setTimeout(TIMEOUT);
        hpcsclient.connectLocal();


        instanceNames = hpcsclient.enumerateInstanceNames("root/cimv2",
                                             CIMName("HP_HealthState"));

        if ( instanceNames.size() > 0 )
        {
            for (Uint32 i = 0; i < instanceNames.size(); i++)
            {
                if (instanceNames[i].getClassName().
                    equal(CIMName("HP_HealthState")))
                {
                    returnedInstance = hpcsclient.getInstance("root/cimv2",
                        instanceNames[i],
                        true,
                        false,
                        false,
                        propertyList);

                    Uint32 statusIndex =
                        returnedInstance.findProperty(PROPERTY_STATUS);
                    Uint32 operationalStatusIndex =
                        returnedInstance.findProperty(
                            PROPERTY_OPERATIONAL_STATUS);
                    Uint32 statusDescriptionsIndex =
                        returnedInstance.findProperty(
                            PROPERTY_STATUS_DESCRIPTIONS);

                    if (statusIndex != PEG_NOT_FOUND &&
                        operationalStatusIndex != PEG_NOT_FOUND &&
                        statusDescriptionsIndex != PEG_NOT_FOUND)
                    {
                        status.assign(returnedInstance.
                            getProperty(statusIndex).getValue());
                        operationalStatus.assign(returnedInstance.
                            getProperty(operationalStatusIndex).getValue());
                        statusDescriptions.assign(returnedInstance.
                            getProperty(statusDescriptionsIndex).getValue());
                        if (!status.isNull() && !operationalStatus.isNull() &&
                            !statusDescriptions.isNull())
                        {
                            // Set the properties, read from HP_HealhState.
                            status.get(_status);
                            operationalStatus.get(_operationalStatus);
                            statusDescriptions.get(_statusDescriptions);
                            useDefaultStatus = false;
                        }
                    }
                    break;
                }
            }
        }
    }
    catch (...)
    {
        // Use the default values if any exception is encountered.
    }

    if (useDefaultStatus)
    {
        _status.clear();
        _operationalStatus.clear();
        _statusDescriptions.clear();

        // hardcoded.
        _status = String(STATUS);
        _operationalStatus.append(Uint16(OPENVMS_DEFAULT_OPERATIONAL_STATUS));
        _statusDescriptions.append(String(OPENVMS_DEFAULT_STATUS_DESCRIPTIONS));
    }
}
