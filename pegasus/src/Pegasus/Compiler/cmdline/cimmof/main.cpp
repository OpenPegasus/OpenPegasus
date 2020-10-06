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
//%/////////////////////////////////////////////////////////////////////////////


// A main for the cimmof_parser.  It can be embedded elsewhere, too.

#include <Pegasus/Common/Config.h>
#include <iostream>
#include <Pegasus/Compiler/mofCompilerOptions.h>
#include "cmdlineExceptions.h"
#include "cmdline.h"
#include <Pegasus/Compiler/cimmofParser.h>
#include <Pegasus/Compiler/parserExceptions.h>

#ifdef PEGASUS_OS_PASE
# include <ILEWrapper/ILEUtilities.h>
#endif

PEGASUS_USING_STD;

#ifdef PEGASUS_HAVE_NAMESPACES
using namespace ParserExceptions;
#endif

// This is used by the parsing routines to control flow
// through include files
#ifdef PEGASUS_OS_ZOS
#include <Pegasus/General/SetFileDescriptorToEBCDICEncoding.h>
#endif

static mofCompilerOptions cmdline;

extern "C++" int processCmdLine(int, char **, mofCompilerOptions &, ostream &);
extern "C++" ostream& help(ostream& os, int progtype);
extern "C++" ostream& cimmofl_warning(ostream& os);

#define NAMESPACE_ROOT "root/cimv2"

static const char MSG_PATH [] = "pegasus/pegasusServer";

int main(int argc, char ** argv)
{
#ifdef PEGASUS_OS_PASE
    // Allow user group name larger than 8 chars in PASE environemnt
    setenv("PASE_USRGRP_LIMITED","N",1);
#endif

#ifdef PEGASUS_OS_ZOS
    // for z/OS set stdout and stderr to EBCDIC
    setEBCDICEncoding(STDOUT_FILENO);
    setEBCDICEncoding(STDERR_FILENO);
#endif

    int ret = 0;
    String msg_;
    MessageLoaderParms parms;
    // set message loading to use process locale
    MessageLoader::_useProcessLocale = true;
    MessageLoader::setPegasusMsgHomeRelative(argv[0]);

    try
    {
        ret = processCmdLine(argc, argv, cmdline, cerr);
    }
    catch (ArgumentErrorsException &e)
    {
        cerr << argv[0] << ": " << e.getMessage() << endl;

        parms.msg_id = "Compiler.cimmofMessages.ERR_USAGE";
        parms.default_msg =
            "Use '--help' to obtain command syntax.";
        parms.msg_src_path = MSG_PATH;
        cerr << argv[0] << ": " << MessageLoader::getMessage(parms) << endl;

        ret =  PEGASUS_CIMMOF_CIM_EXCEPTION;
    }
    catch (CmdlineNoRepository &e)
    {
        cerr << e.getMessage() << endl;
        ret = PEGASUS_CIMMOF_CMDLINE_NOREPOSITORY;
    }
    catch (CIMException &e)
    {
        parms.msg_id = "Compiler.cmdline.cimmof.main.UNEXPECTED_CONDITION";
        parms.default_msg = "Unexpected condition: ";
        cerr << MessageLoader::getMessage(parms) << e.getMessage() << endl;
        ret = PEGASUS_CIMMOF_UNEXPECTED_CONDITION;
    }

#ifdef PEGASUS_OS_PASE
    if (cmdline.is_local())
    {
        // Check special authorities in PASE environment
        if (!umeCheckCmdAuthorities(cmdline.quiet()))
            return 1;
    }
#endif

    if (ret)
    {
        if (ret > 0)
        {
            parms.msg_id = "Compiler.cmdline.cimmof.main.UNEXPECTED_RESULT";
            parms.default_msg =
                "Unexpected result from processing command line: $0";
            parms.arg0 = ret;
            cerr << MessageLoader::getMessage(parms) << endl;
            parms.msg_id = "Compiler.cmdline.cimmof.main.COMPILE_TERMINATING";
            parms.default_msg = "Compilation terminating.";
            cerr << MessageLoader::getMessage(parms) << endl;
        }

        return ret;
    }

    if (cmdline.is_local() && !cmdline.get_no_usage_warning())
    {
        cimmofl_warning(cerr);
    }

    const Array<String>& filespecs = cmdline.get_filespec_list();

    // For most options, a real repository is required.  If we can't
    // create one and we need to, bail.
    cimmofParser *p = cimmofParser::Instance();
    p->setCompilerOptions(&cmdline);
    try
    {
        if ( p->setRepository() )
        {
            p->setDefaultNamespacePath(NAMESPACE_ROOT);
        }
        else
        {
            parms.msg_id = "Compiler.cmdline.cimmof.main.FAILED_TO_SET";
            parms.default_msg = "Failed to set DefaultNamespacePath.";
            cerr << MessageLoader::getMessage(parms) << endl;
            // ATTN: P3 BB 2001 Did not set namespace.
            // We may need to log an error here.
            ret = PEGASUS_CIMMOF_NO_DEFAULTNAMESPACEPATH;
            return ret;
        }
    }
    catch (const CannotConnectException &)
    {
        parms.msg_id =
            "Compiler.cmdline.cimmof.cmdline.CANNOT_CONNECT_EXCEPTION";
        parms.default_msg = "Cannot connect to CIM Server."
            " The CIM Server may not be running.";
        cerr << MessageLoader::getMessage(parms) << endl;
        return PEGASUS_CIMMOF_CANNOT_CONNECT_EXCEPTION;
    }
    if (filespecs.size())    // user specified command line args
    {
        for (unsigned int i = 0; i < filespecs.size(); i++)
        {
            if (p->setInputBufferFromName((const String &)filespecs[i]) == 0)
            {
                try
                {
                    ret = p->parse();
                } catch(ParserLexException &e)
                {
                    parms.msg_id = "Compiler.cmdline.cimmof.main.LEXER_ERROR";
                    parms.default_msg = "Lexer error: ";
                    msg_ = MessageLoader::getMessage(parms).append(
                            e.getMessage());
                    ret = PEGASUS_CIMMOF_PARSER_LEXER_ERROR ;
                }
                catch(Exception &e)
                {
                    parms.msg_id = "Compiler.cmdline.cimmof.main.PARSING_ERROR";
                    parms.default_msg = "Parsing error: ";
                    //msg_ = String("Parsing error: ").append(e.getMessage());
                    msg_ = MessageLoader::getMessage(parms).append(
                            e.getMessage());
                    ret = PEGASUS_CIMMOF_PARSING_ERROR;
                }
            }
            else
            {
                //l10n
                // ATTN: l10n TODO - this path was not localized by the msg
                // freeze for R2.3.  So, use an
                // internal exception msg for now.
                // But, need to replace this with
                // a new cimmof msg in release 2.4.
                parms.msg_id = "Common.InternalException.CANNOT_OPEN_FILE";
                parms.default_msg = "Can't open file $0";
                parms.arg0 = filespecs[i];
                //msg_ = String("Can't open file ").append(filespecs[i]);
                msg_ = MessageLoader::getMessage(parms);
                ret = PEGASUS_CIMMOF_BAD_FILENAME;
            }
        }
    }
    else
    {
        try
        {
            ret = p->parse();
        }
        catch(ParserLexException &e)
        {
            parms.msg_id = "Compiler.cmdline.cimmof.main.LEXER_ERROR";
            parms.default_msg = "Lexer error: ";
            msg_ = MessageLoader::getMessage(parms).append(e.getMessage());
            ret = PEGASUS_CIMMOF_PARSER_LEXER_ERROR ;
        }
        catch(Exception &e)
        {
            parms.msg_id = "Compiler.cmdline.cimmof.main.GENERAL_EXCEPTION";
            parms.default_msg = "Compiler general exception: ";
            msg_ = MessageLoader::getMessage(parms).append(e.getMessage());
            ret = PEGASUS_CIMMOF_COMPILER_GENERAL_EXCEPTION;
        }
    }

    p->destroy();

    if (msg_.size() > 0)
    {
        cerr << msg_ << endl;
    }

    return ret;
}
