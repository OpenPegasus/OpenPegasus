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

#include <cstdlib>
#include <cctype>
#include <fstream>
#include <cstdio>
#include <Pegasus/Common/FileSystem.h>
#include "OptionManager.h"


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// TODO: expand variables in the configuration file. For example:
//
//     provider_dir = "${home}/providers"
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// Option
//
////////////////////////////////////////////////////////////////////////////////

const Array<String>& Option::getDomain() const
{
    return _domain;
}

void Option::setDomain(const Array<String>& domain)
{
    _domain = domain;
}

////////////////////////////////////////////////////////////////////////////////
//
// OptionManager
//
////////////////////////////////////////////////////////////////////////////////

OptionManager::OptionManager()
    :_msgPath("")
{

}

OptionManager::~OptionManager()
{
    // Delete all options in the list:

    for (Uint32 i = 0; i < _options.size(); i++)
        delete _options[i];
}

void OptionManager::registerOption(Option* option)
{
    if (!option)
        throw NullPointer();

    if (lookupOption(option->getOptionName()))
        throw OMDuplicateOption(option->getOptionName());

    _options.append(option);
}


void OptionManager::registerOptions(OptionRow* optionRow, Uint32 numOptions)
{
    for (Uint32 i = 0; i < numOptions; i++)
    {
        // Get option name:

        if (!optionRow[i].optionName)
            throw NullPointer();

        String optionName = optionRow[i].optionName;

        // Get default value:

        String defaultValue;

        if (optionRow[i].defaultValue)
            defaultValue = optionRow[i].defaultValue;

        // Get the required flag:

        Boolean required = optionRow[i].required != 0;

        // Get the type:

        Option::Type type = optionRow[i].type;

        // Get the domain:

        Array<String> domain;

        if (optionRow[i].domain)
        {
            Uint32 domainSize = optionRow[i].domainSize;

            for (Uint32 j = 0; j < domainSize; j++)
                domain.append(optionRow[i].domain[j]);
        }

        // Get commandLineOptionName:

        String commandLineOptionName;

        if (optionRow[i].commandLineOptionName)
            commandLineOptionName = optionRow[i].commandLineOptionName;

        // get optionHelp Message String

        String optionHelpMessage;

        if (optionRow[i].optionHelpMessage)
            optionHelpMessage = optionRow[i].optionHelpMessage;

        // Add the option:

        Option* option = new Option(
            optionName,
            defaultValue,
            required,
            type,
            domain,
            commandLineOptionName,
            optionHelpMessage);

        registerOption(option);
    }
}

void OptionManager::registerOptions(
    OptionRowWithMsg* optionRow,
    Uint32 numOptions)
{
    for (Uint32 i = 0; i < numOptions; i++)
    {
        // Get option name:

        if (!optionRow[i].optionName)
        {
            throw NullPointer();
        }

        String optionName = optionRow[i].optionName;

        // Get default value:

        String defaultValue;

        if (optionRow[i].defaultValue)
        {
            defaultValue = optionRow[i].defaultValue;
        }
        // Get the required flag:

        Boolean required = optionRow[i].required != 0;

        // Get the type:

        Option::Type type = optionRow[i].type;

        // Get the domain:

        Array<String> domain;

        if (optionRow[i].domain)
        {
            Uint32 domainSize = optionRow[i].domainSize;

            for (Uint32 j = 0; j < domainSize; j++)
            {
                domain.append(optionRow[i].domain[j]);
            }
        }

        // Get commandLineOptionName:

        String commandLineOptionName;

        if (optionRow[i].commandLineOptionName)
        {
            commandLineOptionName = optionRow[i].commandLineOptionName;
        }
        // get optionHelp Message String

        String optionHelpMessage;

        if (optionRow[i].optionHelpMessage)
        {
            optionHelpMessage = optionRow[i].optionHelpMessage;
        }

        String messageKey = String();
        if (optionRow[i].messageKey)
        {
            messageKey = optionRow[i].messageKey;
        }

        // Add the option:

        Option* option = new Option(
            optionName,
            defaultValue,
            required,
            type,
            domain,
            commandLineOptionName,
            optionHelpMessage,
            messageKey);

        registerOption(option);
    }
}
void OptionManager::mergeCommandLine(
    int& argc,
    char**& argv,
    Boolean abortOnErr)
{
    for (int i = 0; i < argc; )
    {
        // Check for -option:

        const char* arg = argv[i];

        if (*arg == '-')
        {
            // Look for the option:

            Option* option = _lookupOptionByCommandLineOptionName(arg + 1);

            // a Null option here could mean either 
            // 1) option is missing
            // 2) option is invalid
            String optn = arg+1;
            if (!option)
            {
                if (abortOnErr)
                {
                    //option is invalid
                    if(optn.size() != 0)
                    {
                        throw OMMBadCmdLineOption(arg);
                    }
                    //option is missing 
                    else
                    {
                        throw OMMissingCommandLineOptionArgument(arg);
                    }
                }
                else
                {
                    i++;
                    continue;
                }
            }

            // Get the option argument if any:

            const char* optionArgument = "true";

            if (option->getType() != Option::BOOLEAN)
            {
                if (i + 1 == argc)
                    throw OMMissingCommandLineOptionArgument(arg);

                optionArgument = argv[i + 1];
            }

            // Validate the value:

            if (!option->isValid(optionArgument))
                throw OMInvalidOptionValue(arg, optionArgument);

            // Set the value:

            option->setValue(optionArgument);

            // Remove the option and its argument from the command line:

            if (option->getType() == Option::BOOLEAN)
            {
                memmove(&argv[i], &argv[i + 1], (argc-i) * sizeof(char*));
                argc--;
            }
            else
            {
                memmove(&argv[i], &argv[i + 2], (argc-i-1) * sizeof(char*));
                argc -= 2;
            }
        }
        else
            i++;
    }
}

void OptionManager::mergeFile(const String& fileName)
{
   // Open the input file:
    ifstream is(fileName.getCString());

    if (!is)
        throw NoSuchFile(fileName);

    // For each line of the file:

    String line;

    for (Uint32 lineNumber = 1; GetLine(is, line); lineNumber++)
    {
        // -- Get the identifier and value:

        if (line[0] == '#')
            continue;

        // Skip leading whitespace:

        const Char16* p = line.getChar16Data();

        while (*p && isspace(*p))
            p++;

        if (!*p)
            continue;

        if (*p == '#')
            continue;

        // Get the identifier:

        String ident;

        if (!(isalpha(*p) || *p == '_'))
            throw OMConfigFileSyntaxError(fileName, lineNumber);

        ident.append(*p++);

        while (isalnum(*p) || *p == '_')
            ident.append(*p++);

        // Skip whitespace after identifier:

        while (*p && isspace(*p))
            p++;

        // Expect an equal sign:

        if (*p != '=')
            throw OMConfigFileSyntaxError(fileName, lineNumber);
        p++;

        // Skip whitespace after equal sign:

        while (*p && isspace(*p))
            p++;

        // Expect open quote:

        if (*p != '"')
            throw OMConfigFileSyntaxError(fileName, lineNumber);
        p++;

        // Get the value:

        String value;

        while (*p && *p != '"')
        {
            if (*p == '\\')
            {
                p++;

                switch (*p)
                {
                    case 'n':
                        value.append('\n');
                        break;

                    case 'r':
                        value.append('\r');
                        break;

                    case 't':
                        value.append('\t');
                        break;

                    case 'f':
                        value.append('\f');
                        break;

                    case '"':
                        value.append('"');
                        break;

                    case '\0':
                        throw OMConfigFileSyntaxError(fileName, lineNumber);

                    default:
                        value.append(*p);
                }
                p++;
            }
            else
                value.append(*p++);
        }


        // Expect close quote:

        if (*p != '"')
            throw OMConfigFileSyntaxError(fileName, lineNumber);
        p++;

        // Skip whitespace through end of line:

        while (*p && isspace(*p))
            p++;

        if (*p)
            throw OMConfigFileSyntaxError(fileName, lineNumber);

        // Now that we have the identifier and value, merge it:

        Option* option = (Option*)lookupOption(ident);

        if (!option)
            throw OMUnrecognizedConfigFileOption(ident);

        if (!option->isValid(value))
            throw OMInvalidOptionValue(ident, value);

        option->setValue(value);
    }
}

void OptionManager::checkRequiredOptions() const
{
    for (Uint32 i = 0; i < _options.size(); i++)
    {
        const Option* option = _options[i];

        if (option->getRequired() && !option->isResolved())
            throw OMMissingRequiredOptionValue(option->getOptionName());
    }
}

const Option* OptionManager::lookupOption(const String& name) const
{
    for (Uint32 i = 0; i < _options.size(); i++)
    {
        if (_options[i]->getOptionName() == name)
            return _options[i];
    }

    return 0;
}

Boolean OptionManager::lookupValue(const String& name, String& value) const
{
    const Option* option = lookupOption(name);

    if (!option)
        return false;

    value = option->getValue();
    return true;
}

Boolean OptionManager::lookupIntegerValue(
    const String& name,
    Uint32& value) const
{
    //ATTN: KS P1 7 May 2002 - Add test for Integer type in om table.
    String valueString;
    if (lookupValue(name, valueString))
    {
        value = atol(valueString.getCString());
        return true;
    }
    else
    {
        return false;
    }

}

Boolean OptionManager::valueEquals(
    const String& name,
    const String& value) const
{
    String optionString;

    return (lookupValue(name, optionString) && optionString == value);
}

Boolean OptionManager::isTrue(const String& name) const
{
    //ATTN: KS 7 May 2002 P3 Add test to confirm boolean type
    return valueEquals(name, "true") ? true: false;
}

/*  ATTN: P3 MB 2001 Buried this one for the moment to think about it.
Uint32 OptionManager::isStringInOptionMask(
    const String& option,
    const String& entry)
{
    String optionString;

    if (lookupValue(name, optionString) && optionString == value)
        if (optionString.find(entry)
            return 1;
    else
        return PEG_NOT_FOUND;
}
*/

Option* OptionManager::_lookupOptionByCommandLineOptionName(const String& name)
{
    for (Uint32 i = 0; i < _options.size(); i++)
    {
        if (_options[i]->getCommandLineOptionName() == name)
            return _options[i];
    }

    return 0;
}

void OptionManager::print() const
{
    for (Uint32 i = 0; i < _options.size(); i++)
    {
        Option* option = _options[i];
        cout << option->getOptionName() << "=\"";
        cout << option->getValue() << "\" ";
        cout << option->getOptionHelpMessage() << "\n";
    }
    cout << endl;
}

void OptionManager::setMessagePath(String messagePath)
{
    _msgPath = messagePath;
}
/* This is utility function to create help string for the option*/
String createOptionHelpString(Option* option)
{
    String defMsg = " -";
    defMsg.append(option->getCommandLineOptionName());
    defMsg.append("  ");
    defMsg.append(option->getOptionName());
    defMsg.append(". ");
    defMsg.append(option->getOptionHelpMessage());
    defMsg.append(". Default(");
    defMsg.append(option->getDefaultValue());
    defMsg.append(")\n");
    return defMsg;
}

void OptionManager::printOptionsHelp() const
{
    for (Uint32 i = 0; i < _options.size(); i++)
    {
        String str;
        Option* option = _options[i];
        String defMsg = createOptionHelpString(option);
        CString cstr = defMsg.getCString();
        const String messageKey = option->getMessageKey();
        CString msgKeyCString = messageKey.getCString();
        if (String::compare(messageKey, String::EMPTY))
        {
            MessageLoaderParms parms(
                (const char*)msgKeyCString,
                (const char*)cstr);
            parms.msg_src_path = _msgPath;
            str = MessageLoader::getMessage(parms);
        }
        else
        {
            str = defMsg;
        }
        cout << str;
    }
    cout << endl;
}

void OptionManager::printOptionsHelpTxt(
    const String& header,
    const String& trailer) const
{
    cout << "\n" << header << "\n";
    printOptionsHelp();
    cout << trailer << "\n";
}

////////////////////////////////////////////////////////////////////////////////
//
// Option
//
////////////////////////////////////////////////////////////////////////////////

Option::Option(
    const String& optionName,
    const String& defaultValue,
    Boolean required,
    Type type,
    const Array<String>& domain,
    const String& commandLineOptionName,
    const String& optionHelpMessage,
    const String& messageKey)
    :
    _optionName(optionName),
    _defaultValue(defaultValue),
    _value(defaultValue),
    _required(required),
    _type(type),
    _domain(domain),
    _commandLineOptionName(commandLineOptionName),
    _optionHelpMessage(optionHelpMessage),
    _messageKey(messageKey),
    _resolved(false)
{
    if (!isValid(_value))
        throw OMInvalidOptionValue(_optionName, _value);
}

Option::Option(const Option& x)
    :
    _optionName(x._optionName),
    _defaultValue(x._defaultValue),
    _value(x._value),
    _required(x._required),
    _type(x._type),
    _domain(x._domain),
    _commandLineOptionName(x._commandLineOptionName),
    _optionHelpMessage(x._optionHelpMessage),
    _messageKey(x._messageKey)
{
}

Option::~Option()
{

}

Option& Option::operator=(const Option& x)
{
    if (this != &x)
    {
        _optionName = x._optionName;
        _defaultValue = x._defaultValue;
        _value = x._value;
        _required = x._required;
        _type = x._type;
        _domain = x._domain;
        _commandLineOptionName = x._commandLineOptionName;
        _optionHelpMessage = x._optionHelpMessage;
        _messageKey = x._messageKey;
    }
    return *this;
}

Boolean Option::isValid(const String& value) const
{
    // Check to see that the value is in the domain (if a domain was given)

    Uint32 domainSize = _domain.size();

    if (domainSize)
    {
        Boolean found = false;

        for (Uint32 i = 0; i < domainSize; i++)
        {
            if (value == _domain[i])
                found = true;
        }

        if (!found)
            return false;
    }

    // Check the type:

    switch (_type)
    {
        case BOOLEAN:
        {
            if (value == "true" || value == "false")
                return true;
            else
                return false;
        }

        case STRING:
            return true;

        case INTEGER:
        case NATURAL_NUMBER:
        case WHOLE_NUMBER:
        {
            CString tmp = value.getCString();
            char* end = 0;
            long x = strtol(tmp, &end, 10);

            if (!end || *end != '\0')
                return false;

            switch (_type)
            {
                case INTEGER:
                    return true;

                case NATURAL_NUMBER:
                    return x >= 1;

                case WHOLE_NUMBER:
                    return x >= 0;

                default:
                    break;
            }
        }
    }

    // Unreachable!
    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
// ConfigFileSyntaxError
//
////////////////////////////////////////////////////////////////////////////////

String OMConfigFileSyntaxError::_formatMessage(
    const String& file, Uint32 line)
{
    char buffer[32];
    sprintf(buffer, "%u", line);
    MessageLoaderParms parms(
        "Common.OptionManager.SYNTAX_ERR_CONFIG_FILE",
        "Syntax error in configuration file: ");
    String result = MessageLoader::getMessage(parms);
    result.append(file);
    result.append("(");
    result.append(buffer);
    result.append(")");
    return result;
}

PEGASUS_NAMESPACE_END
