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

//
//
// process the command line, getting the repository location and
// the include path list and the other options
//
//
// bug 4573 - cimmof include file search path processing is inadequate
//
// Bug 4573 changed the behavior of the processing for locating specified
//  include files. The new procssing is based on the include file processing
//  behaviour used by the C compiler.
//
//      The search path for included files previously was:
//          1. try to open the file in the current working directory.
//          2. process the include path array from the cimof(l) cmdline
//             processing which always include "dot" as a default search
//             path and then any paths specified on the command line
//             with the -I option.
//
//      The search path for included files now is:
//          1. try to open the file in the same directory as the current
//             file being processed.
//          2. process the include path array from the cimof(l) cmdline
//             processing which only includes paths specified on the
//             command line with the -I option.
//
//
//
//
//
//
//


#include <Pegasus/Common/Config.h>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/PegasusVersion.h>

#ifdef PEGASUS_ENABLE_PRIVILEGE_SEPARATION
#include <Pegasus/Common/Constants.h>
#include <pwd.h>
#endif

#include <Pegasus/Compiler/mofCompilerOptions.h>
#include "cmdlineExceptions.h"
#include "cmdline.h"
#include <Pegasus/getoopt/getoopt.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

#define DEFAULT_SERVER_AND_PORT "localhost:5988"

ostream & cimmofl_warning(ostream &os)
{

  String warn =
      "Warning: Use of cimmofl can corrupt the CIM Server Repository.\n";
  warn.append (
      "         cimmofl should only be used under very controlled"
            " situations.\n");
  warn.append (
      "         cimmof is the recommended OpenPegasus MOF compiler.\n\n");

  MessageLoaderParms parms(
      "Compiler.cmdline.cimmof.cmdline.CIMMOFL_USAGE_WARNING",warn);

  os << MessageLoader::getMessage(parms);

  return os;
}

ostream & help(ostream &os, int progtype)
{

    //l10n menu
    //PEP167 change
    String help;
#ifdef PEGASUS_OS_HPUX
    if(progtype == 1)
    {
        help.append("Usage: ").append("cimmofl");
    }
    else
    {
        help.append("Usage: ").append("cimmof ");
    }
    help.append( " -h | --help\n" );

    if(progtype == 1)
        help.append("       cimmofl");
    else
        help.append("       cimmof ");
    help.append(" --version\n");

    if(progtype == 1)
        help.append("       cimmofl");
    else
        help.append("       cimmof ");
    help.append(" [ -w ] [ -uc ] [ -aE | -aV | -aEV ] [ -I path ]\n");
    help.append("               [ -n namespace | --namespace namespace ]");
    if(progtype == 1)
    {
        help.append(
            "\n               [ -R repositorydir ] [ --CIMRepository"
                                " repositorydir ]");
        help.append(
            "\n               [ -N repositoryname ] [ -M repositorymode]");
    }

    help.append( " [ mof_file ... ]\n");
    help.append("Options : \n");
    help.append( "    -h, --help      - Display this help message \n");
    help.append( "    --version       - Display CIM Server version\n");
    help.append( "    -w              - Suppress warning messages\n");
    help.append( "    -I              - Specify an include path\n");
    help.append(
       "    -n, --namespace - Override the default CIM Repository namespace\n");
    help.append(
        "    -uc             - Allow update of an existing class definition\n");
    help.append( "    -aE             - Allow Experimental Schema changes\n");
    help.append( "    -aV             - Allow any Version Schema change\n");
    help.append( "    -aEV            - Allow both Experimental and Version"
                                        " Schema changes\n");
    if(progtype == 1) {
        help.append( "    -R, --CIMRepository - Specify the repository"
                                        " directory\n");
        help.append( "    -N                  - Specify the repository name -"
                                        " defaults to \"repository\"\n");
        help.append( "    -M                  - Repository mode [XML, BIN] -"
                                        " defaults to \"XML\"\n");
    }
#else
    if(progtype == 1)
        help.append("Usage: ").append("cimmofl");
    else
        help.append("Usage: ").append("cimmof ");
    help.append( " -h | --help\n");

    if(progtype == 1)
        help.append("       cimmofl");
    else
        help.append("       cimmof ");
    help.append(" --version\n");

    if(progtype == 1)
        help.append("       cimmofl");
    else
        help.append("       cimmof ");
    help.append( " [ -w ] [-E ] [ -uc ] [ -aE | -aV | -aEV ] [ -I path ]\n");
    help.append( "               [ -n namespace | --namespace namespace ]"
                                " [ --xml ]\n");
    help.append( "               [ --trace ]");
#ifdef PEGASUS_OS_PASE
  if(progtype == 1)
      help.append("  [ -q ]");
#endif
    if(progtype == 1)
    {
        help.append("\n               [ -R repositorydir ] [ --CIMRepository"
                                    " repositorydir ]");
        help.append("\n               [ -N repositoryname ] [ -M repository"
                                    "mode]");
    }

    help.append( " [ mof_file... ]\n");
    help.append("Options :\n");
    help.append( "    -h, --help          - Display this help message\n");
    help.append( "    --version           - Display CIM Server version\n");
    help.append( "    -E                  - Syntax check only\n");
    help.append( "    -w                  - Suppress warning messages\n");
    help.append( "    -I                  - Specify an include path\n");
    help.append( "    -n, --namespace     - Override default CIM Repository"
                                            " namespace (root/cimv2)\n");
    help.append( "    -uc                 - Allow update of an existing class"
                                                " definition\n");
    help.append( "    -aE                 - Allow Experimental Schema"
                                                " changes\n");
    help.append("    -aV                 - Allow any Version Schema change\n");
    help.append( "    -aEV                - Allow both Experimental and"
                                                " Version Schema changes\n");
#ifdef PEGASUS_OS_PASE
    help.append( "    -q                  - Suppress all messages except"
                                            " command line usage errors\n");
#endif
    help.append( "    --xml               - Output XML only, to stdout."
                                                " Do not update repository\n");
    help.append( "    --trace             - Trace to file (default to stdout)"
                                                "\n");
    // PEP167 - '-R', '--CIMRepository' disabled for cimmof ONLY.
    if(progtype == 1) {
        help.append( "    -R, --CIMRepository - Specify the repository"
                                                " directory\n");
        help.append( "    -N                  - Specify the repository name"
                                            " - defaults to \"repository\"\n");
        help.append( "    -M                  - Repository mode [XML, BIN]"
                                                " - defaults to \"XML\"\n");
    }
#endif

    if (progtype == 1)
    {
        help.append(
        "    -W                  - Suppress the usage warning (i.e., Warning:\n"
        "                          Use of cimmofl can corrupt the CIM Server\n"
        "                          Repository. cimmofl should only be used\n"
        "                          under very controlled situations. cimmof\n"
        "                          is the recommended OpenPegasus MOF\n"
        "                          compiler.)\n");
    }

#ifdef PEGASUS_ENABLE_MRR_GENERATION
    if (progtype == 1)
    {
        help.append(
        "    -m                  - Create an MRR (memory-resident repository)\n"
        "                          rather than a disk-resident repository.\n"
        "                          Classes and qualifier declarations are\n"
        "                          placed into source files and instances are\n"
        "                          placed into a flat binary file.\n");
        help.append(
        "    -d                  - Discard description qualifiers when\n"
        "                          creating an MRR (memory-resident\n"
        "                          repository).\n");
    }
#endif

    if(progtype == 1)
    {
        help.append("\n\nWarning: Use of cimmofl can corrupt the CIM Server"
                    " Repository.\n");
        help.append("         cimmofl should only be used under very"
                    " controlled situations.\n");
        help.append("         cimmof is the recommended OpenPegasus MOF"
                    " compiler.\n");
    }

    // now localize the menu based on platform, use help as the default menu
    // which has been appropriately built above for the specific platform
    MessageLoaderParms parms ;
    if(progtype == 1)
    {
        parms = MessageLoaderParms(
            "Compiler.cmdline.cimmofl.cmdline.MENU.STANDARD",help);

#ifdef PEGASUS_OS_HPUX
        parms = MessageLoaderParms(
            "Compiler.cmdline.cimmofl.cmdline.MENU.PEGASUS_OS_HPUX",help);
#endif
    }
    else
    {
        parms = MessageLoaderParms(
            "Compiler.cmdline.cimmof.cmdline.MENU.STANDARD",help);

#ifdef PEGASUS_OS_HPUX
        parms = MessageLoaderParms(
            "Compiler.cmdline.cimmof.cmdline.MENU.PEGASUS_OS_HPUX",help);
#endif
    }
    os << MessageLoader::getMessage(parms);
    return os;
}

/* flag value, type, islong?, needsValue? */
static struct optspec optspecs[] =
{
    {(char*)"", FILESPEC, false, getoopt::NOARG},
    {(char*)"h", HELPFLAG, false, getoopt::NOARG},
    {(char*)"help", HELPFLAG, true, getoopt::NOARG},
    {(char*)"version", VERSIONFLAG, true, getoopt::NOARG},
    {(char*)"n", NAMESPACE, false, getoopt::MUSTHAVEARG},
    {(char*)"namespace", NAMESPACE, true, getoopt::MUSTHAVEARG},
    {(char*)"I", INCLUDEPATH, false, getoopt::MUSTHAVEARG},
    //PEP167 - not required
    //{(char*)"Include", INCLUDEPATH, true, getoopt::MUSTHAVEARG},
    {(char*)"w", SUPPRESSFLAG, false, getoopt::NOARG},
    {(char*)"u", UPDATEFLAG, false, getoopt::MUSTHAVEARG},
    {(char*)"a", ALLOWFLAG, false, getoopt::MUSTHAVEARG},
#ifndef PEGASUS_OS_HPUX
    {(char*)"E", SYNTAXFLAG, false, getoopt::NOARG},
    {(char*)"trace", TRACEFLAG, true, getoopt::OPTIONALARG},
    {(char*)"xml", XMLFLAG, true, getoopt::NOARG},
#endif
#ifdef PEGASUS_OS_PASE
    //PASE env ship q option
    {(char*)"q", QUIETFLAG, false, getoopt::NOARG},
#endif
#ifdef PEGASUS_ENABLE_MRR_GENERATION
    {(char*)"m", MRRFLAG, false, getoopt::NOARG},
    {(char*)"d", DISCARDFLAG, false, getoopt::NOARG},
#endif
    {(char*)"", OPTEND_CIMMOF, false, getoopt::NOARG},
    {(char*)"R", REPOSITORYDIR, false, getoopt::MUSTHAVEARG},
    {(char*)"CIMRepository", REPOSITORYDIR, true, getoopt::MUSTHAVEARG},
    {(char*)"N", REPOSITORYNAME, false, getoopt::MUSTHAVEARG},
    {(char*)"M", REPOSITORYMODE, false, getoopt::MUSTHAVEARG},
    {(char*)"W", NO_USAGE_WARNING, false, getoopt::NOARG},
    {(char*)"", OPTEND_CIMMOFL, false, getoopt::NOARG}
};

//PEP167 change - 2nd argument char* added
static void setCmdLineOpts(getoopt &cmdline, int progtype)
{
    for (unsigned int i = 0; ; i++)
    {
        const optspec &o = optspecs[i];
        //PEP167 change
        if(progtype == 1 && o.catagory == OPTEND_CIMMOF)
            continue;
        else if(progtype == 1 && o.catagory == OPTEND_CIMMOFL)
            break;
        else if(progtype == 0 && o.catagory == OPTEND_CIMMOF)
            break;

        if (o.flag[0] == '\0')
            continue;
        if (o.islong)
            cmdline.addLongFlagspec(o.flag, (getoopt::argtype)o.needsvalue);
        else
            cmdline.addFlagspec(o.flag[0], o.needsvalue ==
                getoopt::MUSTHAVEARG ? true : false);
    }
}

//PEP167 change - 2nd argument char* added
static opttypes catagorize(const Optarg &arg, int progtype)
{
    for (unsigned int i = 0; ; i++)
    {
        const optspec &o = optspecs[i];
        //PEP167 change
        if(progtype == 1 && o.catagory == OPTEND_CIMMOF)
            continue;
        else if(progtype == 1 && o.catagory == OPTEND_CIMMOFL)
            break;
        else if(progtype == 0 && o.catagory == OPTEND_CIMMOF)
            break;
        if (arg.getName() == o.flag)
            return o.catagory;
    }
    if(progtype == 0)
        return OPTEND_CIMMOF;
    else
        return OPTEND_CIMMOFL;
}

static void applyDefaults(mofCompilerOptions &cmdlinedata)
{
    if (cmdlinedata.is_local())
    {
#if defined(PEGASUS_USE_RELEASE_DIRS) && \
        defined(PEGASUS_OS_ZOS)
        cmdlinedata.set_repository_name(ZOS_DEFAULT_PEGASUS_REPOSITORY);
#elif defined(PEGASUS_OS_PASE)
        cmdlinedata.set_repository(PASE_DEFAULT_PEGASUS_HOME);
#else
        char *peghome = getenv(PEGASUS_HOME);
        if (peghome)
        {
            cmdlinedata.set_repository(peghome);
        } else
        {
        }
#endif // end of #if defined(PEGASUS_USE_RELEASE_DIRS) &&
       // defined(PEGASUS_OS_ZOS)
    }
    else
    {
        cmdlinedata.set_repository(DEFAULT_SERVER_AND_PORT);
    }
    cmdlinedata.reset_syntax_only();
    cmdlinedata.reset_suppress_warnings();
    cmdlinedata.reset_suppress_all_messages();
    cmdlinedata.reset_trace();
    cmdlinedata.set_namespacePath(ROOTCIMV2);
    cmdlinedata.set_repository_name(REPOSITORY_NAME_DEFAULT);
    cmdlinedata.set_repository_mode(REPOSITORY_MODE_DEFAULT);
    cmdlinedata.set_erroros(PEGASUS_STD(cerr));
    cmdlinedata.set_warningos(PEGASUS_STD(cerr));
    cmdlinedata.reset_operationType();
    cmdlinedata.reset_xmloutput();
    cmdlinedata.reset_update_class();
    cmdlinedata.reset_allow_experimental();
    cmdlinedata.reset_allow_version();
#ifdef PEGASUS_OS_PASE
    cmdlinedata.reset_quiet();
#endif
}

#if defined(PEGASUS_OS_TYPE_WINDOWS)
# define SEPCHAR '\\'
# define SEPCHAR2 '/'
#else
# define SEPCHAR '/'
# define SEPCHAR2 '/'
#endif

// This function looks at a string (which we suppose to be argv[0]
// and decides if it represents cimmof or cimmofl
Boolean isCimmoflCommandName(const char *name)
{
    const char *pos;
    const char *pos2;
    pos = strrchr(name, SEPCHAR);
    pos2 = strrchr(name, SEPCHAR2);
    pos = (pos2 > pos) ? pos2 : pos;
    if (!pos)
        pos = name;
    else
        pos++;
    // returns true in case of cimmofl
    // returns false in case first 6 letter are "cimmof" and no "l" follows
    // returns true in case anything else but "cimmof" or "cimmofl"
    if (*pos != 'c' && *pos != 'C') return true;
    pos++;
    if (*pos != 'i' && *pos != 'I') return true;
    pos++;
    if (*pos != 'm' && *pos != 'M') return true;
    pos++;
    if (*pos != 'm' && *pos != 'M') return true;
    pos++;
    if (*pos != 'o' && *pos != 'O') return true;
    pos++;
    if (*pos != 'f' && *pos != 'F') return true;
    pos++;
    if (*pos != 'l' && *pos != 'L') return false;
    return true;
}

extern "C++" int processCmdline(int, char **, mofCompilerOptions &, ostream&);
int processCmdLine(int argc, char **argv, mofCompilerOptions &cmdlinedata,
   ostream &helpos = cerr)
{
    getoopt cmdline;
    int type = -1;

    Boolean isCimmoflCommand = isCimmoflCommandName(argv[0]);
    setCmdLineOpts(cmdline, isCimmoflCommand);
    cmdline.parse(argc, argv);
    if (isCimmoflCommand)
    {
        cmdlinedata.set_is_local();

#ifdef PEGASUS_ENABLE_PRIVILEGE_SEPARATION
        // Create the repository files with the correct owner and permissions

# define PWD_BUFF_SIZE 4096

        struct passwd pwd;
        char buffer[PWD_BUFF_SIZE];
        struct passwd* ptr = 0;

        if ((getpwnam_r(PEGASUS_CIMSERVERMAIN_USER,
                        &pwd,
                        buffer,
                        PWD_BUFF_SIZE,
                        &ptr) == 0)
                &&
                    (ptr != 0))

        {
            setgid(ptr->pw_gid);
            setuid(ptr->pw_uid);
        }

        umask(S_IRWXG|S_IRWXO);
#endif

    }
    else
    {
        cmdlinedata.reset_is_local();
    }

    applyDefaults(cmdlinedata);
    if (cmdline.hasErrors())
    {

        // Throw an exception and handle it in the caller
        MessageLoaderParms parms(
            "Compiler.cmdline.cimmof.cmdline.CMDLINE_ERRORS",
            "Command line errors:\n");
        String msg = MessageLoader::getMessage(parms);
        cmdline.printErrors(msg);

        throw ArgumentErrorsException(msg);
    }

    MessageLoaderParms tooManyOptionsMsgParms(
        "Compiler.cmdline.cimmof.cmdline.CMDLINE_ERRORS",
        "Too many options specified.\n");

    for (unsigned int i = cmdline.first(); i < cmdline.last(); i++)
    {
        const Optarg &arg = cmdline[i];
        opttypes c = catagorize(arg, isCimmoflCommand);
        if(type == HELPFLAG || type == VERSIONFLAG)
            throw ArgumentErrorsException(tooManyOptionsMsgParms);
        switch (c)
        {
            case VERSIONFLAG:
                if(type != -1)
                    throw ArgumentErrorsException(tooManyOptionsMsgParms);
                break;
            case HELPFLAG:
                if(type != -1)
                    throw ArgumentErrorsException(tooManyOptionsMsgParms);
                break;
            case INCLUDEPATH:
                cmdlinedata.add_include_path(arg.optarg());
                break;
            case SUPPRESSFLAG:
                cmdlinedata.set_suppress_warnings();
                break;
            case NAMESPACE:
                cmdlinedata.set_namespacePath(arg.optarg());
                break;
            case REPOSITORYDIR:
                cmdlinedata.set_repository(arg.optarg());
                break;
            case REPOSITORYNAME:
                cmdlinedata.set_repository_name(arg.optarg());
                break;
            case REPOSITORYMODE:
                {
                    cmdlinedata.set_repository_mode(arg.optarg());
                   if (!String::equalNoCase(arg.optarg(), "XML") &&
                       !String::equalNoCase(arg.optarg(), "BIN"))
                   {
                      MessageLoaderParms parms(
                          "Compiler.cmdline.cimmof.cmdline."
                              "UNKNOWN_VALUE_OPTION_M",
                          "Unknown value specified for option -M.");
                      throw ArgumentErrorsException(parms);
                   }

                }
                break;
            case UPDATEFLAG:
                {
                  if (arg.optarg().size() == 1)
                  {
                      for (unsigned int j = 0; j < arg.optarg().size(); j++)
                      {
                          if (arg.optarg()[j] == 'c')
                              cmdlinedata.set_update_class();
                          else
                          {
                              MessageLoaderParms parms(
                                  "Compiler.cmdline.cimmof.cmdline."
                                        "UNKNOWN_VALUE_OPTION_U",
                                  "Unknown value specified for option -u.");
                              throw ArgumentErrorsException(parms);
                          }
                      }
                  }
                  else
                  {
                      MessageLoaderParms parms(
                          "Compiler.cmdline.cimmof.cmdline."
                              "TOO_MANY_VALUES_OPTION_U",
                          "Too many values specified for option -u.");
                      throw ArgumentErrorsException(parms);
                  }
                }
                break;
            case ALLOWFLAG:
                {
                  if ((arg.optarg().size() <= 2) && (arg.optarg().size() != 0))
                  {
                      for (unsigned int j = 0; j < arg.optarg().size(); j++)
                      {
                          if (arg.optarg()[j] == 'E')
                              cmdlinedata.set_allow_experimental();
                          else
                              if (arg.optarg()[j] == 'V')
                                  cmdlinedata.set_allow_version();
                              else
                              {
                                  MessageLoaderParms parms(
                                      "Compiler.cmdline.cimmof.cmdline."
                                            "UNKNOWN_VALUE_OPTION_A",
                                      "Unknown value specified for option -a.");
                                  throw ArgumentErrorsException(parms);
                              }
                      }
                  }
                  else
                  {
                      MessageLoaderParms parms(
                          "Compiler.cmdline.cimmof.cmdline."
                              "TOO_MANY_VALUES_OPTION_A",
                          "Too many values specified for option -a.");
                      throw ArgumentErrorsException(parms);
                  }
                }
                break;
#ifndef PEGASUS_OS_HPUX
            case SYNTAXFLAG: cmdlinedata.set_syntax_only();
                cmdlinedata.set_operationType(
                compilerCommonDefs::DO_NOT_ADD_TO_REPOSITORY);
                break;
            case TRACEFLAG:
                {
                     cmdlinedata.set_trace();
                     const String &s = arg.optarg();
                     if (s != "")
                     {
                         // ATTN: P3 BB Mar 2001  No way to close the trace
                         // stream  or to delete the ostream object.
                         // It's OK for now because the program terminates
                         // when we're done with the stream.
                         ofstream *tracefile = new ofstream(s.getCString());
                         if (tracefile && *tracefile)
                             cmdlinedata.set_traceos(*tracefile);
                     }
                }
                break;
            case XMLFLAG: cmdlinedata.set_xmloutput();
                break;
#endif
#ifdef PEGASUS_OS_PASE
            // If quiet mode is chosen then shut down stdout and stderr.
            // This is used during product installation and PTF application.
            // We must be absolutely quiet to avoid a terminal being
            // activated in native mode.
            case QUIETFLAG:
            {
                cmdlinedata.set_quiet();
                // Redirect to /dev/null.
                // Works for both qshell and native modes.
                freopen("/dev/null","w",stdout);
                freopen("/dev/null","w",stderr);
                break;
            }
#endif
            case FILESPEC: cmdlinedata.add_filespecs(arg.optarg());
                break;
            case OPTEND_CIMMOFL:
            case OPTEND_CIMMOF:
               return -1;  // shouldn't happen
                   break;

            case NO_USAGE_WARNING:
               cmdlinedata.set_no_usage_warning();
#ifdef PEGASUS_ENABLE_MRR_GENERATION
            case MRRFLAG:
                cmdlinedata.set_mrr();
                break;
            case DISCARDFLAG:
                cmdlinedata.set_discard();
                break;
#endif
        }
        type = c;
    }

    if(type == VERSIONFLAG)
    {
        cerr << "Version " << PEGASUS_PRODUCT_VERSION << endl;
        return(-1);
    }
    else if(type == HELPFLAG)
    {
        help(helpos, isCimmoflCommand);
        return(-1);
    }

    if (String::equal(cmdlinedata.get_repository(), String::EMPTY)) {

        MessageLoaderParms parms(
            "Compiler.cmdline.cimmof.cmdline.MUST_SPECIFY_R_OR_HOME",
            "Specify -R or set the PEGASUS_HOME environment variable.");
        throw CmdlineNoRepository(parms);
    }

    return 0;
}
