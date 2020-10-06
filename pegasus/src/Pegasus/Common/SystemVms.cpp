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

#include <descrip.h>           //  $DESCRIPTOR
#include <iodef.h>             // IO$_SENSEMODE
#include <ttdef.h>             // TT$M_NOBRDCST
#include <tt2def.h>            // TT2$M_PASTHRU
#include <starlet.h>
#include <ssdef.h>             // SS$_NORMAL
#include <stsdef.h>            // VMS_STATUS_SUCCESS
#include <chpdef.h>            // CHP$_
#include <armdef.h>            // ARM$_
#include <prvdef.h>            // PRV$M_SETPRV
#include <unixlib.h>
#include <rms.h>

PEGASUS_NAMESPACE_BEGIN

//==============================================================================
//
// System
//
//==============================================================================

static String vmsPath;
static Mutex vmsPathMutex(Mutex::NON_RECURSIVE);

static char* toVmsDir(const char* path)
{
    char* dirend;
    char* dirstart;
    char* dir_path;
    dirend = strrchr(path, '.');
    if (dirend)
    {
        dir_path = new char[strlen(path) + sizeof("DIR")];
        strcpy(dir_path, path);
        dirend = strrchr(dir_path, '.');
        *dirend = ']';
        dirend = strrchr(dir_path, ']');
        *dirend = '\0';
        strcat(dir_path, ".DIR");
    }
    else
    {
        dir_path = new char[strlen(path) + sizeof("[000000].DIR")];
        dirstart = strchr(path, '[');
        if (dirstart) // Top level directory
        {
            strncpy(dir_path, path, dirstart-path);
            dir_path[dirstart-path] = '\0';
            strcat(dir_path, "[000000]"); // Top level directory
            strcat(dir_path, ++dirstart); // Add the directory name
            dirend = strrchr(dir_path, ']');
            *dirend = '\0';
            strcat(dir_path, ".DIR");
        }
        else
        {
            delete [] dir_path;
            dir_path = 0;
        }
    }
    return dir_path;
}

static int action(char *path, int type_of_file)
{
    int ret;
    char *dir_path;

    if (type_of_file == DECC$K_DIRECTORY)
    {
        dir_path = toVmsDir(path);
        if (dir_path)
        {
            vmsPath = dir_path;
            delete [] dir_path;
        }
        ret = dir_path != 0;
    }
    else
    {
        vmsPath = path;
        ret = 1;
    }
    return ret;
}

static int toVms(const char* path, String& vms_path)
{
    char cwd[256]; // OpenVMS file can't be larger than 255 chars.
    char *result;
    char *dir_path;
    int ret;

    // Treat the current directory as a special case

    if ((strcmp(path, ".") == 0) || (strcmp(path, "./") == 0))
    {
        result = getcwd(cwd, sizeof(cwd), 1);
        if (!result) return 0;
        dir_path = toVmsDir(cwd);
        if (dir_path)
        {
            vms_path = dir_path;
            delete [] dir_path;
            ret = 1;
        }
        else
            ret = 0;
    }
    else if (strchr(path, '/') != 0)
    {
        AutoMutex pathLock(vmsPathMutex);
        ret = decc$to_vms(path, action, 0, 0);
        vms_path = vmsPath;
    }
    else
    {
        vms_path = path;
        ret = 1;
    }
    return ret;
}

static int canAccess(const char* path, int access_right)
{
    struct itm
    {
        unsigned short bufLen;
        unsigned short itmCode;
        void *bufAddr;
        void *retAddr;
    };

    int retStat;
    int len;

    union armdef armval;
    struct FAB fab;
    struct XABPRO xabpro;

    struct
    {
        struct itm item[3];
        unsigned long term;
    } itmlst;

    String vms_path;
    CString cvms_path;

    // Convert the path to OpenVMS format (if necessary)

    retStat = toVms(path, vms_path);

    cvms_path = vms_path.getCString();
    len = strlen(cvms_path);
    if (len > 255) return SS$_FILACCERR;

    // Get the file protections

    fab = cc$rms_fab;

    fab.fab$l_fna = (char *) (const char *) cvms_path;
    fab.fab$b_fns = len;
    fab.fab$b_fac = FAB$M_GET;
    fab.fab$l_xab = &xabpro;

    xabpro.xab$b_cod = XAB$C_PRO;
    xabpro.xab$b_bln = XAB$C_PROLEN;
    xabpro.xab$w_aclsiz = 0;    // Not considering ACLs at this time
    xabpro.xab$l_aclbuf = 0;
    xabpro.xab$l_nxt = 0;

    retStat = sys$open(&fab, 0, 0);
    if (!$VMS_STATUS_SUCCESS(retStat))
    {
        if (retStat != RMS$_FNF)
        {
            PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL1,
                "sys$open error: %s", strerror(EVMSERR, retStat)));
        }
        return retStat;
    }

    sys$close(&fab, 0, 0);

    armval.arm$r_fill_47_.arm$l_file_access = access_right;

    itmlst.item[0].itmCode = CHP$_ACCESS;
    itmlst.item[0].bufLen = sizeof(armval);
    itmlst.item[0].bufAddr = &armval;
    itmlst.item[0].retAddr = 0;
    itmlst.item[1].itmCode = CHP$_OWNER;
    itmlst.item[1].bufLen = sizeof(xabpro.xab$l_uic);
    itmlst.item[1].bufAddr = &xabpro.xab$l_uic;
    itmlst.item[1].retAddr = 0;
    itmlst.item[2].itmCode = CHP$_PROT;
    itmlst.item[2].bufLen = sizeof(xabpro.xab$w_pro);
    itmlst.item[2].bufAddr = &xabpro.xab$w_pro;
    itmlst.item[2].retAddr = 0;
    itmlst.term = 0;

    retStat = sys$chkpro(&itmlst, 0, 0);
    if (!$VMS_STATUS_SUCCESS(retStat))
    {
        if (retStat != SS$_NOPRIV)
        {
            PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL1,
                "sys$chkpro error: %s", strerror(EVMSERR, retStat)));
        }
    }

    return retStat;
}

Boolean System::canRead(const char* path)
{
    return canAccess(path, ARM$M_READ) == SS$_NORMAL;
}

Boolean System::canWrite(const char* path)
{
    return canAccess(path, ARM$M_WRITE) == SS$_NORMAL;
}

String System::getPassword(const char* prompt)
{
    struct
    {
        short int numbuf;
        char frst_char;
        char rsv1;
        long rsv2;
    }
    tahead;

    typedef struct
    {                           // I/O status block
        short i_cond;           // Condition value
        short i_xfer;           // Transfer count
        long i_info;            // Device information
    }
    iosb;

    typedef struct
    {                           // Terminal characteristics
        char t_class;           // Terminal class
        char t_type;            // Terminal type
        short t_width;          // Terminal width in characters
        long t_mandl;           // Terminal's mode and length
        long t_extend;          // Extended terminal characteristics
    }
    termb;

    termb otermb;
    termb ntermb;

    short int ichan;            // Gets channel number for TT:

    int errorcode;
    int kbdflgs;                // saved keyboard fd flags
    int kbdpoll;                // in O_NDELAY mode
    int kbdqp = false;          // there is a char in kbdq
    int psize;                  // size of the prompt

    const size_t MAX_PASS_LEN = 32;
    static char buf[MAX_PASS_LEN];
    char kbdq;                  // char we've already read

    iosb iostatus;

    static long termset[2] = { 0, 0 };  // No terminator

    $DESCRIPTOR(inpdev, "TT");  // Terminal to use for input

    buf[0] = 0;

    ichan = 0;

    try
    {
        // Get a channel for the terminal

        errorcode = sys$assign(&inpdev,     // Device name
                               &ichan,      // Channel assigned
                               0,   // request KERNEL mode access
                               0);  // No mailbox assigned

        if (errorcode != SS$_NORMAL)
        {
            throw Exception("sys$assign failure");
        }

        // Read current terminal settings

        errorcode = sys$qiow(0,     // Wait on event flag zero
                             ichan, // Channel to input terminal
                             IO$_SENSEMODE, // Function - Sense Mode
                             &iostatus,     // Status after operation
                             0, 0,  // No AST service
                             &otermb,       // [P1] Address of Char Buffer
                             sizeof (otermb),       // [P2] Size of Char Buffer
                             0, 0, 0, 0);   // [P3] - [P6]

        if (errorcode != SS$_NORMAL)
        {
            throw Exception("sys$qiow IO$_SENSEMODE failure");
        }

        // setup new settings

        ntermb = otermb;

        // turn on passthru and nobroadcast

        ntermb.t_extend |= TT2$M_PASTHRU;
        ntermb.t_mandl |= TT$M_NOBRDCST;

        // Write out new terminal settings

        errorcode = sys$qiow(0,     // Wait on event flag zero
                             ichan, // Channel to input terminal
                             IO$_SETMODE,   // Function - Set Mode
                             &iostatus,     // Status after operation
                             0, 0,  // No AST service
                             &ntermb,       // [P1] Address of Char Buffer
                             sizeof (ntermb),       // [P2] Size of Char Buffer
                             0, 0, 0, 0);   // [P3] - [P6]

        if (errorcode != SS$_NORMAL)
        {
            throw Exception("sys$qiow IO$_SETMODE failure");
        }

        // Write a prompt, read characters from the terminal, performing no
        // editing
        // and doing no echo at all.

        psize = strlen(prompt);

        errorcode = sys$qiow(0,     // Event flag
                             ichan, // Input channel
                             IO$_READPROMPT | IO$M_NOECHO | IO$M_NOFILTR |
                             IO$M_TRMNOECHO,
                             // Read with prompt, no echo, no translate, no
                             // termination character echo
                             &iostatus,     // I/O status block
                             NULL,  // AST block (none)
                             0,     // AST parameter
                             &buf,  // P1 - input buffer
                             MAX_PASS_LEN,  // P2 - buffer length
                             0,     // P3 - ignored (timeout)
                             0,     // P4 - ignored (terminator char set)
                             prompt,        // P5 - prompt buffer
                             psize);        // P6 - prompt size

        if (errorcode != SS$_NORMAL)
        {
            throw Exception("sys$qiow IO$_READPROMPT failure:");
        }

        // Write out old terminal settings
        errorcode = sys$qiow(0,     // Wait on event flag zero
                             ichan, // Channel to input terminal
                             IO$_SETMODE,   // Function - Set Mode
                             &iostatus,     // Status after operation
                             0, 0,  // No AST service
                             &otermb,       // [P1] Address of Char Buffer
                             sizeof (otermb),       // [P2] Size of Char Buffer
                             0, 0, 0, 0);   // [P3] - [P6]

        if (errorcode != SS$_NORMAL)
        {
            throw Exception("sys$qiow IO$_SETMODE failure");
        }

        // Start new line

        const int CR = 0x0d;
        const int LF = 0x0a;
        fputc(CR, stdout);
        fputc(LF, stdout);

        // Remove the termination character
        psize = strlen(buf);
        buf[psize - 1] = 0;
    }

    catch (Exception &e)
    {
        PEG_TRACE((TRC_OS_ABSTRACTION, Tracer::LEVEL1, "%s: %s",
            (const char *) (e.getMessage()).getCString(),
            strerror(EVMSERR, errorcode)));
    }

    // Deassign the channel

    if (ichan) sys$dassgn(ichan);

    return buf;
}

String System::encryptPassword(const char* password, const char* salt)
{
    const size_t MAX_PASS_LEN = 1024;
    char pbBuffer[MAX_PASS_LEN] = {0};
    Uint32 dwByteCount;
    char pcSalt[3] = {0};

    strncpy(pcSalt, salt, 2);
    dwByteCount = strlen(password);
    memcpy(pbBuffer, password, dwByteCount);

    for (Uint32 i=0; (i<dwByteCount) || (i>=MAX_PASS_LEN); i++)
    {
        (i%2 == 0) ? pbBuffer[i] ^= pcSalt[1] : pbBuffer[i] ^= pcSalt[0];
    }

    return String(pcSalt) + String((char *)pbBuffer);
}

Boolean System::isPrivilegedUser(const String& userName)
{
    unsigned int audsts;
    union prvdef priv_mask;

    int retStat;

    priv_mask.prv$l_l1_bits =
    priv_mask.prv$l_l2_bits = 0;
    priv_mask.prv$v_sysprv = true;    // SYSPRV privilege.
    priv_mask.prv$v_bypass = true;    // BYPASS privilege.

    retStat = sys$check_privilegew(0, &priv_mask, 0, 0, 0, &audsts, 0, 0);

    return retStat == SS$_NORMAL;
}

PEGASUS_NAMESPACE_END
