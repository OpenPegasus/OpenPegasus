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
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/XmlWriter.h>

#include <Pegasus/General/OptionManager.h>
#include <Pegasus/General/MofWriter.h>

#include <Pegasus/Repository/CIMRepository.h>

#include <Pegasus/Client/CIMClient.h>

#include "clientRepositoryInterface.h"

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

CIMNamespaceName nameSpace = CIMNamespaceName ("root/cimv2");
/*
class classNameList
{
public:
    classNameList(CIMNamespaceName& nameSpace, clientRepositoryInterface& cli)
    {
    };
};
*/

/* class classNameList - Provides creation and management of classNames in a
    list form.
*/
class classNameList
{
public:
    // constructor - Creates empty list
    classNameList()
    {
    //_cli = 0;
    _index = 0;
    }
    // constructor - with nameSpace and Interface
    classNameList(const CIMNamespaceName& nameSpace,
                  const clientRepositoryInterface& cli)

    {
    _nameSpace = nameSpace,
    _cli = cli,
    _index = 0;
    }
    ~classNameList()
    {

    }
    // Set Namespace
    void setNameSpace (const CIMNamespaceName& nameSpac)
    {
    _nameSpace = nameSpace;
    }
    // enumerate - Executes the enumerate
    Boolean enumerate(const CIMName& className, Boolean deepInheritance)
    {
        try
        {
            _classNameList =  _cli.enumerateClassNames(
                _nameSpace, className, deepInheritance);
        }
        catch(Exception &e)
        {
            cout << "Exception " << e.getMessage()
                 << " on enumerateClassNames open. Terminating." << endl;
            return false;
        }
    return true;
    }
    /* filter - Filters the list against a defined pattern using the
        glob type filter.
       NOTICE: This method no longer performs glob-style matching.  The
       list is now filtered using exact matching, except that it also
       recognizes the "*" pattern which matches everything.  While the new
       functionality is quite limited, this change is consistent with
       the way this method was being used at the time of the change.
        @param pattern -String defining the pattern used to filter the
        list.
        @return Result is the list with all names that do not pass the
        filter removed.

    */
    void filter(String& pattern)
    {
        // Filter the list in accordance with the pattern
        if (pattern == "*")
        {
            return;
        }
        Array<CIMName> tmp;
        for (Uint32 i = 0; i < _classNameList.size(); i++)
        {
            if (String::equalNoCase(_classNameList[i].getString(), pattern))
                tmp.append(_classNameList[i]);
        }
        _classNameList.swap(tmp);
    }

    /* getIndex - Get the current index in the list.  This is used with
        next and start functions to get entries in the list one by one.
    */
    Uint32 getIndex() {
    return _index;
    }
    /* size - returns the number of entires in the list
        @return Uint32 with number of entires in the list.
    */
    Uint32 size() {
    return _classNameList.size();
    }
    /* next - get the next entry in the list.  If there are no more entires
       returns String:EMPTY
       @return String containing next entry or String::EMPTY if at end of list
    */
    String next()
    {
    if (_index < _classNameList.size())
        return _classNameList[_index++].getString();
    else
        return String::EMPTY;
    }
    /* start - Set the index to the start of the list
    */
    void start()
    {
        _index = 0;
    }
private:
    Array<CIMName> _classNameList;
    clientRepositoryInterface _cli;
    CIMNamespaceName _nameSpace;
    Uint32 _index;
};

/** ErrorExit - Print out the error message as an
    and get out.
    @param - Text for error message
    @return - None, Terminates the program
    @execption - This function terminates the program
*/
void ErrorExit(const String& message)
{

    cerr << message << endl;
    exit(1);
}

//------------------------------------------------------------------------------
//
// _indent()
//
//------------------------------------------------------------------------------

static void _indent(PEGASUS_STD(ostream)& os, Uint32 level, Uint32 indentSize)
{
    Uint32 n = level * indentSize;
    if (n > 50)
    {
    cout << "Jumped Ship " << level << " size " << indentSize << endl;
    exit(1);
    }

    for (Uint32 i = 0; i < n; i++)
    os << ' ';
}
void mofFormat(
    PEGASUS_STD(ostream)& os,
    const char* text,
    Uint32 indentSize)
{
    char* var = new char[strlen(text)+1];
    char* tmp = strcpy(var, text);

    Uint32 count = 0;
    Uint32 indent = 0;
    Boolean quoteState = false;
    Boolean qualifierState = false;
    char c;
    char prevchar = 0;
    while ((c = *tmp++))
    {
    count++;
    // This is too simplistic and must move to a token based mini parser
    // but will do for now. One problem is tokens longer than 12 characters
    // that overrun the max line length.
    switch (c)
    {
        case '\n':
        os << Sint8(c);
        prevchar = c;
        count = 0 + (indent * indentSize);
        _indent(os, indent, indentSize);
        break;

        case '\"':   // quote
        os << Sint8(c);
        prevchar = c;
        quoteState = !quoteState;
        break;

        case ' ':
        os << Sint8(c);
        prevchar = c;
        if (count > 66)
        {
            if (quoteState)
            {
            os << "\"\n";
            _indent(os, indent + 1, indentSize);
            os <<"\"";
            }
            else
            {
            os <<"\n";
            _indent(os, indent + 1,  indentSize);
            }
            count = 0 + ((indent + 1) * indentSize);
        }
        break;
        case '[':
        if (prevchar == '\n')
        {
            indent++;
            _indent(os, indent,  indentSize);
            qualifierState = true;
        }
        os << Sint8(c);
        prevchar = c;
        break;

        case ']':
        if (qualifierState)
        {
            if (indent > 0)
            indent--;
            qualifierState = false;
        }
        os << Sint8(c);
        prevchar = c;
        break;

        default:
        os << Sint8(c);
        prevchar = c;
    }

    }
    delete [] var;
}

///////////////////////////////////////////////////////////////
//    OPTION MANAGEMENT
///////////////////////////////////////////////////////////////

/** GetOptions function - This function defines the Options Table
    and sets up the options from that table using the option manager.
    const char* optionName;
    const char* defaultValue;
    int required;
    Option::Type type;
    char** domain;
    Uint32 domainSize;
    const char* commandLineOptionName;
    const char* optionHelpMessage;

*/
void GetOptions(
    OptionManager& om,
    int& argc,
    char** argv,
    const String& pegasusHome)
{
    const char* tmpDir = getenv ("PEGASUS_HOME");
    if (tmpDir == NULL)
    {
        tmpDir = ".";
    }

    static struct OptionRow optionsTable[] =
    {
        //optionname defaultvalue rqd  type domain domainsize clname hlpmsg

         {"namespace", "root/cimv2", false, Option::STRING, 0, 0, "n",
                       "Specifies namespace to use for"},

         {"version", "false", false, Option::BOOLEAN, 0, 0, "v",
                       "Displays Program Version identification"},
         // set to true for testing
         {"verbose", "true", false, Option::BOOLEAN, 0, 0, "verbose",
                       "Displays Extra information"},

         {"help", "false", false, Option::BOOLEAN, 0, 0, "h",
                       "Prints help message with command line options"},

         {"qualifiers", "false", false, Option::BOOLEAN, 0, 0, "q",
                      "If set, show the qualifier declarations"},

         {"instances", "false", false, Option::BOOLEAN, 0, 0, "i",
                      "If set, show the instances"},

         {"noClass", "false", false, Option::BOOLEAN, 0, 0, "nc",
                      "If set, bypass the class display completely"},
         {"all", "false", false, Option::BOOLEAN, 0, 0, "a",
                      "If set, show qualifiers, classes, and instances"},

         {"summary", "false", false, Option::BOOLEAN, 0, 0, "s",
                      "Print only a summary count at end"},

         {"location", tmpDir, false, Option::STRING, 0, 0, "l",
                      "Repository directory (/run if repository directory"
                          " is /run/repository"},

         {"client", "false", false, Option::BOOLEAN, 0, 0, "c",
                      "Runs as Pegasus client using client interface"},

         {"onlynames", "false", false, Option::BOOLEAN, 0, 0, "o",
                      "Show Names only, not the MOF"},

         {"xml", "false", false, Option::BOOLEAN, 0, 0, "x",
                      "Output result in XML rather than MOF"},

    };
    const Uint32 NUM_OPTIONS = sizeof(optionsTable) / sizeof(optionsTable[0]);

    om.registerOptions(optionsTable, NUM_OPTIONS);

    String configFile = pegasusHome + "/cimserver.conf";

    if (FileSystem::exists(configFile))
    {
        om.mergeFile(configFile);
        cout << "Config file from " << configFile << endl;
    }

    om.mergeCommandLine(argc, argv);

    om.checkRequiredOptions();
}

void printHelp(char* name, OptionManager om)
{
    String header = "Usage ";
    header.append(name);
    header.append(" -parameters [class]");
    header.append("  - Generate MOF output from the repository or client\n");

    String trailer =
        "\nAssumes  using repository interface and repository at\n";
    trailer.append("PEGASUS_HOME for repository unless -r specified");
    trailer.append("\nIf [class] exists only that class mof is output.");

    trailer.append("\nIf no parameters set, it outputs nothing");
    trailer.append("\nExamples:");
    trailer.append("\n  tomof - Returns only help/usage information");
    trailer.append("\n  tomof * - Returns information on all classes, etc.");
    trailer.append("\n  tomof CIM_DOOR - Shows mof for CIM_Door from default"
            " namespace");
    //trailer.append("\n  tomof *door* - Shows mof for classes with 'door'"
    //" in name.");
    //trailer.append("\n  tomof -o *software* - Lists Class names with"
    //" 'door' in name.");
    trailer.append("\n  tomof -a - outputs mof for all classes");
    trailer.append("\n  tomof -c - outputs mof for all classes using client"
            " interface.");
    trailer.append("\n  tomof -q - Outputs mof for qualifiers and classes");

    trailer.append("\n  tomof -s - Outputs summary count of classes and"
            " optionally instances. Does not deliver lists");
    om.printOptionsHelpTxt(header, trailer);
}




///////////////////////////////////////////////////////////////
//    MAIN
///////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{

    String pegasusHome;
    pegasusHome = "/";

    // GetEnvironmentVariables(argv[0], pegasusHome);

    // Get options (from command line and from configuration file); this
    // removes corresponding options and their arguments fromt he command
    // line.

    // Get options (from command line and from configuration file); this
    // removes corresponding options and their arguments fromt he command
    // line.

    OptionManager om;

    Boolean argsFound = false;
    if (argc > 1)
    {
        argsFound = true;
    }
    try
    {
        GetOptions(om, argc, argv, pegasusHome);

    }
    catch(Exception& e)
    {
        cout << "Error Qualifier Enumeration:" << endl;
        cerr << argv[0] << ": " << e.getMessage() << endl;
        exit(1);
    }

    // Check to see if user asked for help (-h otpion):
    if(om.isTrue("help"))
    {
        printHelp(argv[0], om);
        exit (0);
    }
    // Check for Version flag and print version message
    if(om.isTrue("version"))
    {
        cout << "Pegasus Version " << PEGASUS_PRODUCT_VERSION
                << " " << argv[0] << " version 1.0 " << endl;
        return 0;
    }

    Boolean showOnlyNames = om.isTrue("onlynames");


    Boolean doNotShowClasses = om.isTrue("noClass");

    // Check for namespace flag and set namespace
    String localNameSpace;
    if(om.lookupValue("namespace", localNameSpace))
    {
        cout << "Namespace = " << localNameSpace << endl;
    }

    // Check for the show qualifiers flag
    Boolean showQualifiers = om.isTrue("qualifiers");

    // Check for the summary count flag
    Boolean summary = om.isTrue("summary");

    Uint32 qualifierCount = 0;
    Uint32 classCount = 0;
    Uint32 classCountDisplayed = 0;
    Uint32 instanceCount = 0;

    // Check for the instances
    Boolean showInstances = om.isTrue("instances");

    Boolean showAll = om.isTrue("all");

    Boolean verbose = om.isTrue("verbose");

    Boolean singleClass = true;

    // Set the flags to determine whether we show all or simply some classes
    if(showAll)
        singleClass = false;
    else    // if no classes in list.
        if(argc < 2)
        singleClass = false;

    // Build the inputClassList from the remaining input parameters
    Array<String> inputClassList;

    if(argc > 1)
    {
        // while there are entries in the argv list collect them
        for(int i = 1; i < argc; i++)
            inputClassList.append(argv[i]);
    }
    else
    {
        // Do output if any arguments but only print help if there is no args.
        if (argsFound)
        {
            inputClassList.append("*");
        }
        else
        {
            printHelp(argv[0], om);
            exit (0);
        }
    }

    if(verbose)
    {
        if(verbose) cout << "Class list is ";
        for(Uint32 i = 0 ; i < inputClassList.size(); i++)
        {
            if(verbose) cout << inputClassList[i] << " ";
        }
        cout << endl;
    }

    // Test to determine if we have a filter pattern to limit classes displayed
    //String pattern;
    //Boolean doWildCard = false;

    // if client request set in options, set isClient variable accordingly
    Boolean isClient = om.isTrue("client");

    // Determine if output is XML or MOF
    Boolean isXMLOutput = om.isTrue("xml");

    // Check for repository path flag and set repository directory
    String location = "";

    if(om.lookupValue("location", location))
    {
        if(verbose) cout << "location Path = " << location << endl;

    }

    clientRepositoryInterface clRepository; // the repository interface object
    clientRepositoryInterface::_repositoryType rt;

    if(isClient)
    {
        location = "localhost:5988";
        rt = clientRepositoryInterface::REPOSITORY_INTERFACE_CLIENT;
    }
    else
    {
        rt = clientRepositoryInterface::REPOSITORY_INTERFACE_LOCAL;

        const char* tmp = getenv("PEGASUS_HOME");

        if (strlen(tmp) == 0 && location == "." )
            ErrorExit("Error, PEGASUS_HOME not set and repository option not"
                    " used");

        location.append("/repository");

        if (!FileSystem::exists(location)
                || FileSystem::isDirectoryEmpty(location))
        {
            cout << "Error. " << location << " does not exist or is empty."
                 << endl;
            exit(1);
        }
    }

    // Set a context for either client or repository access.
    // Test to determine if we have proper location, etc. for opening

    if(verbose)
    {
        cout << "Get from " << ((isClient) ? "client" : "repository ")
        << " at " << location << endl;
    }

    // Create the repository and client objects
    CIMRepository repository(location);
    CIMClient client;

    // if client mode, do client connection, else do repository connection
    try
    {
        clRepository.init(rt, location);
    }
    catch(Exception &e)
    {
        cout << "Exception " << e.getMessage()
             << " on repository open. Terminating."
             << endl;
        return false;
    }
    // Get the complete class name list before we start anything else

    if(showQualifiers || showAll || summary)
    {
        try
        {
            // Enumerate the qualifiers:

            Array<CIMQualifierDecl> qualifierDecls
                    = clRepository.enumerateQualifiers(nameSpace);
            qualifierCount = qualifierDecls.size();

            if(showOnlyNames)
            {
                for(Uint32 i = 0; i < qualifierDecls.size(); i++)
                    cout << "Qualifier "
                         << qualifierDecls[i].getName().getString() << endl;
            }
            if(showQualifiers || showAll)
            {
                for(Uint32 i = 0; i < qualifierDecls.size(); i++)
                {
                    CIMQualifierDecl tmp = qualifierDecls[i];

                    if(isXMLOutput)
                    {
                        XmlWriter::printQualifierDeclElement(tmp, cout);
                    }
                    else
                    {
                        Buffer x;
                        MofWriter::appendQualifierDeclElement(x, tmp);
                        mofFormat(cout, x.getData(), 4);
                    }
                }
            }
        }
        catch(Exception& e)
        {
            ErrorExit(e.getMessage());
        }
    }

    // Setup clasname list object
    classNameList list(nameSpace, clRepository);

    // Enumerate all classnames from namespace

    if (!list.enumerate(CIMName(), true))
        ErrorExit("Class Enumeration failed");

    //Filter list for input patterns
    for(Uint32 i = 0; i < inputClassList.size(); i++)
        list.filter(inputClassList[i]);

    // get size for summary
    classCount = list.size();

    if (!summary || !doNotShowClasses)
    {
        if(showOnlyNames)
        {
            for(Uint32 j = 0; j < list.size(); j++)
                cout << "Class " << list.next() << endl;
        }
        else
        {
            // Print out the MOF for those found
            for(Uint32 j = 0; j < list.size(); j++)
            {
                CIMName nextClass = list.next();
                CIMClass cimClass;
                try
                {
                    Boolean localOnly = true;
                    Boolean includeQualifiers = true;
                    Boolean includeClassOrigin = true;

                    cimClass = clRepository.getClass(nameSpace,
                                                     nextClass,
                                                     localOnly,
                                                     includeQualifiers,
                                                     includeClassOrigin);
                }
                catch(Exception& e)
                {
                    // ErrorExit(e.getMessage());
                    cout << "Class get error " << e.getMessage()
                         << " Class " << nextClass.getString() << endl;
                }
                // Note we get and print ourselves rather than use the generic
                // printMof
                if(isXMLOutput)
                    XmlWriter::printClassElement(cimClass, cout);
                else
                {
                    Buffer x;
                    MofWriter::appendClassElement(x, cimClass);
                    mofFormat(cout, x.getData(), 4);
                }
            }
        }
    // Note that we can do this so we get all instances or just the given class
    }
    list.start();
    if (showInstances || showAll)
    {
        // try Block around basic instance processing
        for(Uint32 j = 0; j < list.size(); j++)
        {
            CIMName className = list.next();
            //cout << "Instances for " << className.getString() << endl;
            try
            {
                Boolean deepInheritance = true;
                Boolean includeClassOrigin = false;
                Boolean includeQualifiers = false;

                if(showOnlyNames)
                {
                    Array<CIMObjectPath> instanceNames;
                    instanceNames =
                        clRepository.enumerateInstanceNames(nameSpace,
                                                            className);
                    for (Uint32 k = 0; k < instanceNames.size(); k++)
                    {
                        cout << "Instance " << instanceNames[k].toString();
                    }
                }
                else    // Process complete instances
                {
                // Process inputClasslist to enumerate and print instances
                    Array<CIMInstance> namedInstances;
                    namedInstances =
                        clRepository.enumerateInstances(nameSpace,
                                                        className,
                                                        deepInheritance,
                                                        includeQualifiers,
                                                        includeClassOrigin);

                    // Process and output each instance
                    for(Uint32 k = 0; k < namedInstances.size(); k++)
                    {
                        CIMInstance instance = namedInstances[k];
                        if(isXMLOutput)
                            XmlWriter::printInstanceElement(instance, cout);
                        else
                        {
                            Buffer x;
                            MofWriter::appendInstanceElement(x, instance);
                            mofFormat(cout, x.getData(), 4);
                        }
                    }
                }
            }
            catch(Exception& e)
            {
                cout << "Error Instance Enumeration:" << e.getMessage()
                     << endl;
            }
        }
    }

    if(summary)
    {
        if(qualifierCount != 0)
            cout << "Qualifiers - " << qualifierCount << endl;
        if(classCount != 0)
            cout << "Classes - " << classCount << " found and "
                 << classCountDisplayed
                 << " output" << endl;
        if(instanceCount != 0)
            cout << "Instances - " << instanceCount << endl;
    }
    exit(0);
}
//PEGASUS_NAMESPACE_END


