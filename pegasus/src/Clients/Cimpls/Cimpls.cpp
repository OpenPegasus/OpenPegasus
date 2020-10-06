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

#include "Cimpls.h"
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <Pegasus/Common/Constants.h>

static CIMClient _c;
static String _nameSpace = PEGASUS_NAMESPACENAME_INTEROP.getString();
static String _providerType[] = { "Unknown", "Other", "Instance",
  "Association", "Indication", "Method" };
static String _providerState[] = {"Unknown", "Other", "OK",
  "Degraded", "Stressed", "Predictive Failure", "Error",
  "Non-Recoverable Error", "Starting", "Stopping", "Stopped",
  "In Service", "No Contact", "Lost Communication"};

// ===============================================================
// main
// ===============================================================

int main(const int argc, const char **argv)
{
  // output looks like:
  // PG_OperatingSystem
  ///  Namespaces: root/cimv2
  //   Provider:   PG_OperatingSystemProvider
  //   Type:       Instance
  //   Module:     OperatingSystemModule
  //   File:       libOSProvider.sl

  _c.setTimeout(PEGASUS_DEFAULT_CLIENT_TIMEOUT_MILLISECONDS);

  // everything in big try/catch to display errors
  try
  {
    _c.connectLocal();

    // Start by enumerating PG_ProviderCapabilities
    Array<CIMObjectPath> capRef =
      _c.enumerateInstanceNames(PEGASUS_NAMESPACENAME_INTEROP,
              "PG_ProviderCapabilities");
    for (int i=0; i<capRef.size(); i++)
    {
      // get the instance
      CIMInstance cap = _c.getInstance(PEGASUS_NAMESPACENAME_INTEROP,capRef[i]);

      // get referenced instance of PG_ProviderModule for later use
      String pMod;
      cap.getProperty(
              cap.findProperty("ProviderModuleName")).getValue().get(pMod);
      CIMObjectPath modRef(String("PG_ProviderModule.Name=\"") + pMod + "\"");
      CIMInstance mod = _c.getInstance(PEGASUS_NAMESPACENAME_INTEROP,modRef);

      // display name of class instrumented
      String className;
      cap.getProperty(cap.findProperty("ClassName")).getValue().get(className);
      cout << endl << className << endl;

      // display namespaces
      Array<String> nameSpaces;
      cap.getProperty(
              cap.findProperty("Namespaces")).getValue().get(nameSpaces);
      cout << "  Namespaces:";
      for (int j=0; j<nameSpaces.size(); j++) cout << " " << nameSpaces[j];
      cout << endl;

      // display name of provider
      String pName;
      cap.getProperty(cap.findProperty("ProviderName")).getValue().get(pName);
      cout << "  Provider:   " << pName << endl;

      // display type of provider
      Array<Uint16> pType;
      cap.getProperty(cap.findProperty("ProviderType")).getValue().get(pType);
      cout << "  Type:      ";
      for (int j=0; j<pType.size(); j++)
        cout << " " << _providerType[ pType[j] ];
      cout << endl;

      // display state
      Array<Uint16> state;
      mod.getProperty(
              mod.findProperty("OperationalStatus")).getValue().get(state);
      cout << "  State:     ";
      for (int j=0; j<state.size(); j++)
        cout << " " << _providerState[ state[j] ];
      cout << endl;

      // display module
      cout << "  Module:     " << pMod << endl;

      // display file (PG_ProviderModule.Location)
      String loc;
      mod.getProperty(mod.findProperty("Location")).getValue().get(loc);
      cout << "  File:       lib" << loc << ".sl" << endl;
    }
  }

  catch (Exception &e)
  {
    cerr << e.getMessage() << endl;
    return 1;
  }

  return 0;
}

// ===============================================================
// getClass
// ===============================================================

int _getClass(const int argc, const char **argv)
{
  CIMClass cldef;
  try
  {
    cldef = _c.getClass( PEGASUS_NAMESPACENAME_INTEROP, argv[0] );
  }
  catch (Exception& e)
  {
    cerr << /* "getClass: " << */ e.getMessage() << endl;
    return 1;
  }

  // Display the class definition
  // without qualifiers, for the moment

  // First the class name and superclass
  cout << "class " << cldef.getClassName().getString() << " : "
    << cldef.getSuperClassName().getString() << endl;
  cout << "{" << endl;

  // Now the properties
  // No qualifiers except [key], but specify type, array
  for (int i=0; i<cldef.getPropertyCount(); i++)
  {
    CIMProperty p = cldef.getProperty(i);
    cout << "  ";
    // output key, if required
    if (_isKey(p)) cout << "[ Key ] ";
    // prepare to output type, but
    // first, if type is "reference", find target class
    if (p.getType() == CIMTYPE_REFERENCE)
      cout << p.getReferenceClassName().getString() << " REF ";
    // output type
    else cout << cimTypeToString(p.getType()) << " ";
    // output name
    cout << p.getName().getString();
    // output array, if required
    if (p.isArray()) cout << "[]";
    // final eol
    cout << ";" << endl;
  }

  // need to do methods
  for (int i=0; i<cldef.getMethodCount(); i++)
  {
    CIMMethod m = cldef.getMethod(i);
    // output type
    cout << "  " << cimTypeToString(m.getType()) << " ";
    // output name
    cout << m.getName().getString() << "(";
    // output parameters
    // new line if there are any parameters
    for (int j=0; j<m.getParameterCount(); j++)
    {
      CIMParameter p = m.getParameter(j);
      // output IN/OUT qualifiers on a fresh line
      cout << endl << "    [ ";
      // loop through qualifiers looking for IN, OUT
      for (int k=0; k<p.getQualifierCount(); k++)
      {
        // when one found, output its value
        CIMQualifier q = p.getQualifier(k);
        if (q.getName().equal("in") ||
            q.getName().equal("out"))
        {
          cout << q.getName().getString() << " ";
        }
      }
      // Now the type
      cout << "] " << cimTypeToString(p.getType()) << " ";
      // finally the name
      cout << p.getName().getString();
      // array brackets
      if (p.isArray()) cout << "[]";
      // closing , on parameter if not last
      if (j != m.getParameterCount()-1) cout << ",";
    }
    // after last param, indent before closing paren

    // close paren
    cout << ")";
    // if (m.isArray()) cout << "[]";
    // finish output
    cout << ";" << endl;
  }

  // final brace and done
  cout << "};" << endl;

  return 0;
}

// ===============================================================
// enumerateClassNames
// ===============================================================

int _enumerateClassNames(const int argc, const char **argv)
{
  return 0;
}

// ===============================================================
// getInstance
// ===============================================================

int _getInstance(const int argc, const char **argv)
{
  // need to get class definition to find keys
  // first arg is name of class
  CIMClass cldef;
  try
  {
    cldef = _c.getClass( PEGASUS_NAMESPACENAME_INTEROP, argv[0] );
  }
  catch(Exception& e)
  {
    cerr << /* "getInstance: " << */ e.getMessage() << endl;
    return 1;
  }

  CIMObjectPath ref;
  CIMInstance inst;

  // If there are no more args, prompt user for keys
  if (argv[1] == 0) ref = CIMObjectPath(String::EMPTY, // hostname left blank
                                       PEGASUS_NAMESPACENAME_INTEROP,
                                       argv[0],
                                       _inputInstanceKeys(cldef));

  // else if there's another arg and it's "list", enumInstNames and print
  // a list from which user will select (return if none)
  else if (String::equalNoCase("list",argv[1]))
  {
    ref = _selectInstance(argv[0]);
    // An empty ObjectPath means nothing was selected
    if (ref.identical(CIMObjectPath())) return 0;
  }

  // else there's another arg but it's invalid
  else
  {
    return 1;
  }

  // get the specified instance
  try
  {
    inst = _c.getInstance(PEGASUS_NAMESPACENAME_INTEROP,ref);
  }
  catch(Exception& e)
  {
    cerr << /* "getInstance: " << */ e.getMessage() << endl;
    return 1;
  }
  _displayInstance(inst);
  return 0;
}

// ===============================================================
// enumerateInstances
// ===============================================================

int _enumerateInstances(const int argc, const char **argv)
{
  Array<CIMInstance> ia;
  try
  {
    ia = _c.enumerateInstances( PEGASUS_NAMESPACENAME_INTEROP, argv[0] );
  }
  catch(Exception& e)
  {
    cerr << /* "enumerateInstances: " << */ e.getMessage() << endl;
    return 1;
  }

  cerr << ia.size() << " instances" << endl;

  for (int i=0; i<ia.size(); i++)
  {
    cout << endl;
    // display property names and values
    _displayInstance(ia[i]);
  }
  return 0;
}

// ===============================================================
// enumerateInstanceNames
// ===============================================================

int _enumerateInstanceNames(const int argc, const char **argv)
{
  Array<CIMObjectPath> iNames;
  try
  {
    iNames = _c.enumerateInstanceNames( PEGASUS_NAMESPACENAME_INTEROP,
                                        argv[0] );
  }
  catch(Exception& e)
  {
    cerr << /* "enumerateInstanceNames: " << */ e.getMessage() << endl;
    return 1;
  }

  cerr << iNames.size() << " instance(s)" << endl;

  for (int i=0; i<iNames.size(); i++)
    cout << "  " << iNames[i].toString() << endl;
  return 0;
}

// ===============================================================
// getProperty
// ===============================================================

int _getProperty(const int argc, const char **argv)
{
  // need to get class definition to find keys
  // first arg is name of class
  CIMClass cldef;
  try
  {
    cldef = _c.getClass( PEGASUS_NAMESPACENAME_INTEROP, argv[0] );
  }
  catch(Exception& e)
  {
    cerr << /* "getProperty: " << */ e.getMessage() << endl;
    return 1;
  }

  CIMObjectPath ref;
  CIMInstance inst;

  // If next arg is "ask", prompt user for keys
  if (String::equalNoCase("ask",argv[1])) ref =
            CIMObjectPath(String::EMPTY,
                          PEGASUS_NAMESPACENAME_INTEROP,
                          argv[0],
                          _inputInstanceKeys(cldef) );

  // else if the next arg and is "list", enumInstNames and print
  // a list from which user will select
  else if (String::equalNoCase("list",argv[1]))
  {
    ref = _selectInstance( argv[0] );
    if (ref.identical(CIMObjectPath())) return 0;
  }

  // else there's another arg but it's invalid
  else
  {
    return 1;
  }

  CIMProperty pDef;
  // if no more args, display property names and ask which
  if (argc < 3)
  {
    int n;
    for (n=0; n<cldef.getPropertyCount(); n++)
      cerr << n+1 << ": " << cldef.getProperty(n).getName().getString() << endl;
    cerr << "Property (1.." << cldef.getPropertyCount() << ")? ";
    cin >> n;
    pDef = cldef.getProperty(n-1);
  }

  // else use last arg as property to get
  else
  {
    int pos = cldef.findProperty(argv[2]);
    if (pos == PEG_NOT_FOUND)
    {
      cerr << argv[2] << ": not found." << endl;
      return 1;
    }
    pDef = cldef.getProperty(pos);
  }

  // Now we can call getProperty() and display result
  CIMValue v;
  try
  {
    v = _c.getProperty( PEGASUS_NAMESPACENAME_INTEROP, ref, pDef.getName() );
  }
  catch (Exception &e)
  {
    cerr << /* "getProperty: " << */ e.getMessage() << endl;
    return 1;
  }

  cout << "  " << pDef.getName().getString();

  if (v.isArray()) cout << "[" << v.getArraySize() << "]";

  if (v.isNull()) cout << "=NULL";
  else cout << "=" << "\"" << v.toString() << "\"";

  cout << endl;
  return 0;
}

// ===============================================================
// deleteClass
// ===============================================================

int _deleteClass(const int argc, const char **argv)
{
  CIMClass cldef;
  try
  {
    _c.deleteClass( PEGASUS_NAMESPACENAME_INTEROP, argv[0] );
  }
  catch (Exception& e)
  {
    cerr << /* "deleteClass: " << */ e.getMessage() << endl;
    return 1;
  }
  return 0;
}

// ===============================================================
// deleteInstance
// ===============================================================

int _deleteInstance(const int argc, const char **argv)
{
  // need to get class definition to find keys
  // first arg is name of class
  CIMClass cldef;
  try
  {
    cldef = _c.getClass( PEGASUS_NAMESPACENAME_INTEROP, argv[0] );
  }
  catch(Exception& e)
  {
    cerr << /* "deleteInstance: " << */ e.getMessage() << endl;
    return 1;
  }

  CIMObjectPath ref;
  CIMInstance inst;

  // If there are no more args, prompt user for keys
  if (argv[1] == 0) ref = CIMObjectPath(String::EMPTY, // hostname left blank
                                       PEGASUS_NAMESPACENAME_INTEROP,
                                       argv[0],
                                       _inputInstanceKeys(cldef));

  // else if there's another arg and it's "list", enumInstNames and print
  // a list from which user will select (return if none)
  else if (String::equalNoCase("list",argv[1]))
  {
    ref = _selectInstance(argv[0]);
    // An empty ObjectPath means nothing was selected
    if (ref.identical(CIMObjectPath())) return 0;
  }

  // else there's another arg but it's invalid
  else
  {
    return 1;
  }

  // delete the specified instance
  try
  {
    _c.deleteInstance(PEGASUS_NAMESPACENAME_INTEROP,ref);
  }
  catch(Exception& e)
  {
    cerr << /* "deleteInstance: " << */ e.getMessage() << endl;
    return 1;
  }

  return 0;
}

// ===============================================================
// Auxiliary function _inputInstanceKeys()
//
// Prompt user for values of key properties of specified class
// ===============================================================

Array<CIMKeyBinding> _inputInstanceKeys(const CIMClass &cldef)
{
  Array<CIMKeyBinding> kb;
  for (int i=0; i<cldef.getPropertyCount(); i++)
  {
    CIMProperty prop = cldef.getProperty(i).clone();
    if (_isKey(prop))
    {
      char s[256];
      cout << prop.getName().getString() << " ("
          << cimTypeToString(prop.getType()) << "): ";
      cin.getline(s,sizeof(s));
      enum CIMKeyBinding::Type t;
      switch (prop.getType())
      {
      case CIMTYPE_BOOLEAN:
        t = CIMKeyBinding::BOOLEAN; break;
      case CIMTYPE_UINT8:
      case CIMTYPE_SINT8:
      case CIMTYPE_UINT16:
      case CIMTYPE_SINT16:
      case CIMTYPE_UINT32:
      case CIMTYPE_SINT32:
      case CIMTYPE_UINT64:
      case CIMTYPE_SINT64:
      case CIMTYPE_REAL32:
      case CIMTYPE_REAL64:
        t = CIMKeyBinding::NUMERIC; break;
      case CIMTYPE_STRING:
      case CIMTYPE_DATETIME:
      default:
        t = CIMKeyBinding::STRING; break;
      case CIMTYPE_REFERENCE:
        t = CIMKeyBinding::REFERENCE; break;
      }
      kb.append(CIMKeyBinding(prop.getName(), s, t));
    }
  }
  return kb;
}

// ===============================================================
// Auxiliary function _selectInstance()
//
// Display list of instance names and ask which
// ===============================================================

CIMObjectPath _selectInstance(const String &clnam)
{
  // Get all the instance handles
  Array<CIMObjectPath> iNames;
  try
  {
    iNames = _c.enumerateInstanceNames(PEGASUS_NAMESPACENAME_INTEROP,clnam);
  }
  catch (Exception& e)
  {
    cerr << /* "enumerateInstanceNames: " << */ e.getMessage() << endl;
    return CIMObjectPath();
  }

  if (iNames.size() == 0)
  {
    cerr << "No instances found." << endl;
    return CIMObjectPath();
  }

  // Display them, numbered starting at 1
  int i;
  for (i=0; i<iNames.size(); i++)
    cerr << i+1 << ": " << iNames[i].toString() << endl;
  i = 0;

  // Ask user to pick one
  while (i < 1 || i > iNames.size())
  {
    cerr << "Instance (1.." << iNames.size() << ")? ";
    cin >> i;
  }
  return iNames[i-1];
}


// ===============================================================
// Auxiliary function _displayInstance()
//
// Print property names and values
// ===============================================================

void _displayInstance(CIMInstance &inst)
{
  for (int j=0; j<inst.getPropertyCount(); j++)
    _displayProperty(inst.getProperty(j));
}


// ===============================================================
// Auxiliary function _displayProperty()
// ===============================================================

void _displayProperty(const CIMProperty &p)
{
  cout << "  " << p.getName().getString();

  CIMValue v = p.getValue();

  if (v.isArray())
    cout << "[" << v.getArraySize() << "]";

  cout << " = ";

  if (v.isNull())
    cout << "NULL";
  else if (v.getType() == CIMTYPE_STRING ||
           v.getType() == CIMTYPE_CHAR16 ||
           v.getType() == CIMTYPE_DATETIME)
    cout << "\"" << v.toString() << "\"";
  else
    cout << v.toString();

  cout << endl;
}

// ===============================================================
// Auxiliary function _isKey()
// ===============================================================

const Boolean _isKey(const CIMProperty &p)
{
  int i;
  if ( (i=p.findQualifier("key")) == PEG_NOT_FOUND )
    return false;

  CIMValue v = p.getQualifier(i).getValue();
  if (v.isNull())
    return false;

  Boolean isKey;
  v.get(isKey);
  if (isKey)
    return true;
  else
    return false;
}
