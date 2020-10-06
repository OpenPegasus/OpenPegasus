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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/Threads.h>
#include <Pegasus/Common/AuditLogger.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Audit_zOS_SMF.h>

#include <sys/utsname.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

static const char* CFZ_EBCDIC = "\xC3\xC6\xE9\x40";  // "CFZ " in EBCDIC


// Mutex Audit_zOS_SMF::_protectProlog;

Audit_zOS_SMF::Audit_zOS_SMF()
{
    if (_recordProlog == NULL)
    {
        AutoMutex autoMut(_protectProlog);
        if (_recordProlog == NULL)
        {
            _recordProlog = (_smf86_record_prolog*)calloc(1,
                sizeof(_smf86_record_prolog));

            _preInitRecordHeaderSection(&(_recordProlog->header));
            _preInitRecordProductSection(&(_recordProlog->product));

        }
    }

}

Audit_zOS_SMF::~Audit_zOS_SMF()
{
    if (_recordProlog != NULL)
    {
        AutoMutex huddel(_protectProlog);
        free(_recordProlog);
        _recordProlog = NULL;
    }
}

void Audit_zOS_SMF::initMyProlog( _smf86_record_prolog * myRecProlog,
                                _smf_record_suptype subtype,
                                int subTypeSize)
{

    // copy the common prolog to the current record
    memcpy(myRecProlog,_recordProlog, sizeof(_smf86_record_prolog));

    // set header subtye specific values.

    // Set total record length
    myRecProlog->header.SMF86LEN=(sizeof(_smf86_record_prolog) + subTypeSize );
    // We do no segmentaion in general
    myRecProlog->header.SMF86SEG=0;
    // Set subtype
    myRecProlog->header.SMF86STY=subtype;
    // Set size of subtype section
    myRecProlog->header.SMF86STL=subTypeSize;

    // Set thread ID
    setEBCDICRecordField( myRecProlog->product.SMF86THID,
                          Threads::id().buffer,
                          sizeof(myRecProlog->product.SMF86THID),true);

    return;
}

Boolean Audit_zOS_SMF::isRecording(_smf_record_suptype subtype)
{
    if (__smf_record(86,subtype,0,NULL) < 0)
    {
        PEG_TRACE((TRC_SERVER,Tracer::LEVEL4,
                   "SMF recording not enabled: \'%s\' "
                       "(errno(%d) reason code(0x%08X))",
                   strerror(errno),errno,__errno2()));
        return false;
    }

    return true;
}

void Audit_zOS_SMF::writeRecord(int subtype, char* record )
{
    // the size of the whole record is stored as short in the first bytes
    // of the record.
    if (__smf_record(86,subtype,((_smf86_header *)record)->SMF86LEN,record) < 0)
    {
        PEG_TRACE((TRC_SERVER,Tracer::LEVEL2,
                   "Cannot write SMF records: \'%s\' "
                       "(errno(%d) reason code(0x%08X))",
                   strerror(errno),errno,__errno2()));
    }

    return;
}


void Audit_zOS_SMF::setEBCDICRecordField(
    unsigned char * field,
    const char * value,
    int size, bool nullTerminated )
{
    int length = strlen(value);

    // if the value size is larger then the filed size
    // cut the value to the size fitting in the field.
    if (length > size)
    {
        length = size;
    }


    memcpy((char *)field,value,length);

    if (nullTerminated)
    {
        field[length] = 0x00;
        length++;
    }

    while (length < size)
    {
        field[length] = ' ';
        length++;
    }

    // convert the ascii field to ebcdic
    __a2e_l((char *)field,length);


}


void Audit_zOS_SMF::_preInitRecordHeaderSection( _smf86_header* recordHeader )
{

    recordHeader->SMF86FLG=0xFF;         // Set bit mask to 11111111;
    recordHeader->SMF86RTY=86;           // Set SMF reckord type
    recordHeader->SMF86TRN=2;            // Both triplets are used
                                         // set system identifyter
    memcpy(recordHeader->SMF86SSI,CFZ_EBCDIC,
           strlen(CFZ_EBCDIC));

                                         // start of product section
    recordHeader->SMF86PRO=sizeof(_smf86_header);
                                         // size of product section
    recordHeader->SMF86PRL=sizeof(_smf86_product);
    recordHeader->SMF86PRN=1;            // one product section
                                         // start of subtype section
    recordHeader->SMF86STO=sizeof(_smf86_header)+
        sizeof(_smf86_product);
    recordHeader->SMF86STN=1;            // one subtype section
}

void Audit_zOS_SMF::_preInitRecordProductSection (
    _smf86_product* recordProductSec )
{

    struct utsname uts;
    char zOS_VRM[sizeof(uts.version) +
                 sizeof(uts.release) + 2];

    // set to recrod version 1
    recordProductSec->SMF86PRRVN = 1;

    // set system identifyter
    memcpy(recordProductSec->SMF86SSI,CFZ_EBCDIC,
           strlen(CFZ_EBCDIC));

    // set pegasus VRM
    setEBCDICRecordField( recordProductSec->SMF86VRM,
                        PEGASUS_PRODUCT_VERSION,
                        sizeof(recordProductSec->SMF86VRM),false);
     if (__osname(&uts) > -1 )
     {

         zOS_VRM[0] = 0;
         strcat(zOS_VRM,uts.version);
         strcat(zOS_VRM,".");
         strcat(zOS_VRM,uts.release);

         // set OS VRM
         setEBCDICRecordField( recordProductSec->SMF86OSL,
                             zOS_VRM,
                             sizeof(recordProductSec->SMF86OSL),false);

         // set system name
         setEBCDICRecordField( recordProductSec->SMF86SYN,
                             uts.nodename,
                             sizeof(recordProductSec->SMF86SYN),false);
     } else
     {
         PEG_TRACE((TRC_SERVER,Tracer::LEVEL1,
             "Cannot get OS version and system name: \'%s\' (errno(%d) "
             "reason code(%08X))",
              strerror(errno),errno,__errno2()));
     }

                                 // set sysplex name
     _setSysplexName( recordProductSec->SMF86SYP);

     recordProductSec->SMF86OPI = System::getPID();

}


void Audit_zOS_SMF::_setSysplexName(unsigned char * field)
{
    // Get pointer to the CVT control block
    int * p = (int *)16;
    int * cvt_ptr = (int *) *(p);
    // Get pointer to the eCVT control block
    int * ecvt_ptr = (int *) *(cvt_ptr+35); // 4*35 = 140 Byte
    // Get pointer to the sysplex name
    char * ECVTSPLX = (char *)(ecvt_ptr+2); // 4*2 = 8 Byte

    memcpy((char*)field, ECVTSPLX, 8);
}

void Audit_zOS_SMF::printRecord (int subtype, char* record )
{
    char printLine[3][80];
    int p;
    int len;
    char item;
    int total = ((_smf86_header *)record)->SMF86LEN;

    printf("\n Char/HEX output(%08X)\n\n",System::getPID());

    for (int i = 0; i <= total;i=i+1)
    {
        p = i%80;

        if (p == 0 && i > 0 ||
            i == total )
        {
            for (int y = 0; y < 3; y=y+1)
            {
                if (p == 0)
                {
                    len = 80;
                } else
                {
                    len = p;
                }

                for (int x = 0; x < len; x=x+1)
                {
                    if (y == 0)
                    {
                        printf("%c",printLine[y][x]);
                    }
                    else
                    {
                        printf("%1X",printLine[y][x]);
                    }
                }
                printf("\n");
            }
            printf("\n");
        }

        item = record[i];
        __e2a_l(&item,1);

        if (item < 32 || item > 126)
        {
            printLine[0][p] = '.';
        } else
        {
            printLine[0][p] = item;
        }

        printLine[1][p] = record[i]/16;
        printLine[2][p] = record[i]%16;


    }
}

PEGASUS_NAMESPACE_END
