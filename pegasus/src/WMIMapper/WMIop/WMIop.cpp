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
// Author: Jair F. T. dos Santos (t.dos.santos.francisco@hp.com)
// Modified By:
//             Amit K Arora, IBM (amita@in.ibm.com) for PEP#101
//
//%////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/AutoPtr.h>
#include "WMIop.h"
#include <iostream>
#include <unistd.h>
#include <stdlib.h>

static const int TIMEOUT = 60000;
static const String DEFAULT_NAMESPACE("root/cimv2");
static CIMClient _c;
static String _hostName;
static Uint32 _portNumber = 0;
static String _nameSpace;
static String _userName;
static String _passWord;

// ===============================================================
// main
// ===============================================================

  // one day:
  // Options:
  //   -h hostname (default: localhost)
  //   -p port (default: 5988)
  //   -u username
  //   -w password (required if -u present, else prompts)

int main(const int argc, const char **argv)
{
  // error if no args
  if (argv[1] == 0)
  {
    _usage();
    return 1;
  }

  AutoPtr<SSLContext> sslContext; // initialized for unencrypted connection
  _c.setTimeout(TIMEOUT);

  // Get hostname from environment, if defined
  char *p = getenv("CIM_HOST");
  if (p) _hostName = p;

  // Get port number, if defined
  char *pn = getenv("CIM_PORT");
  if (pn) _portNumber = atol(pn);

  if (p)
  {
    // hostname was specified, we will not connect local
    // so decide whether encrypted or not
    char *s = getenv("CIM_NOSSL");
    if (s) // don't use ssl (not encrypted)
    {
      if (!pn) _portNumber = 5988; // use 5988 if no port specified
    }
    else
    {
      try
      {
        sslContext.reset(new SSLContext(PEGASUS_SSLCLIENT_CERTIFICATEFILE,
                                    verifyServerCertificate,
                                    PEGASUS_SSLCLIENT_RANDOMFILE
                                    /* "/var/opt/wbem/ssl.rnd" */));
      }
      catch (Exception &e)
      {
        cerr << e.getMessage() << endl;
        return 1;
      }
      if (!pn) _portNumber = 5989; // use 5989 if no port specified
    }
  }

  // Get namespace from environment or use default
  p = getenv("CIM_NAMESPACE");
  _nameSpace = (p==0)? DEFAULT_NAMESPACE:p;

  // Get user from environment or don't specify
  p = getenv("CIM_USER");
  _userName = (p==0)? String::EMPTY : p;

  // Get password from environment or use empty
  p = getenv("CIM_PASSWORD");
  _passWord = (p==0)? String::EMPTY : p;

  try
  {
    if (String::equal(_hostName,String::EMPTY))
    {
        _c.connectLocal();
    }
    else
    // hostname was specified; do remote connect
    {
        if (sslContext.get())
        {
            _c.connect(_hostName, _portNumber, *sslContext.get(),
                _userName, _passWord);
        }
        else
        {
            _c.connect(_hostName, _portNumber, _userName, _passWord);
        }
    }
  }
  catch(Exception& e)
  {
    cerr << e.getMessage() << endl;
    return 1;
  }

  // command is first arg
  const char *cmd = argv[1];

  if (String::equalNoCase(cmd,"getClass") ||
      String::equalNoCase(cmd,"gc"))
    return _getClass(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"enumerateClasses") ||
           String::equalNoCase(cmd,"ec"))
    return _enumerateClasses(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"enumerateClassNames") ||
           String::equalNoCase(cmd,"ecn"))
    return _enumerateClassNames(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"getInstance") ||
           String::equalNoCase(cmd,"gi"))
    return _getInstance(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"enumerateInstances") ||
           String::equalNoCase(cmd,"ei"))
    return _enumerateInstances(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"enumerateInstanceNames") ||
           String::equalNoCase(cmd,"ein"))
    return _enumerateInstanceNames(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"getProperty") ||
           String::equalNoCase(cmd,"gp"))
    return _getProperty(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"setProperty") ||
           String::equalNoCase(cmd,"sp"))
    return _setProperty(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"invokeMethod") ||
           String::equalNoCase(cmd,"im"))
    return _invokeMethod(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"createClass") ||
           String::equalNoCase(cmd,"cc"))
    return _createClass(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"modifyClass") ||
           String::equalNoCase(cmd,"mc"))
    return _modifyClass(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"deleteClass") ||
           String::equalNoCase(cmd,"dc"))
    return _deleteClass(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"createInstance") ||
           String::equalNoCase(cmd,"ci"))
    return _createInstance(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"modifyInstance") ||
           String::equalNoCase(cmd,"mi"))
    return _modifyInstance(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"deleteInstance") ||
           String::equalNoCase(cmd,"di"))
    return _deleteInstance(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"associators") ||
           String::equalNoCase(cmd,"a"))
    return _associators(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"associatorNames") ||
           String::equalNoCase(cmd,"an"))
    return _associatorNames(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"references") ||
           String::equalNoCase(cmd,"r"))
    return _references(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"referenceNames") ||
           String::equalNoCase(cmd,"rn"))
    return _referenceNames(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"execQuery") ||
           String::equalNoCase(cmd,"exq"))
    return _execQuery(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"getQualifier") ||
           String::equalNoCase(cmd,"gq"))
    return _getQualifier(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"setQualifier") ||
           String::equalNoCase(cmd,"sq"))
    return _setQualifier(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"deleteQualifier") ||
           String::equalNoCase(cmd,"dq"))
    return _deleteQualifier(argc-2,&argv[2]);
  else if (String::equalNoCase(cmd,"enumerateQualifiers") ||
           String::equalNoCase(cmd,"eq"))
    return _enumerateQualifiers(argc-2,&argv[2]);
  else
  {
    cerr << cmd << ": Invalid CIM operation."<< endl;
   _usage();
    return 1;
  }
}

// ===============================================================
// getClass
// ===============================================================
int _getClass(const int argc, const char **argv)
{
  if (argv[0] == 0)
  {
    cerr << "Usage: wmiop getClass|gc <class>" << endl;
    return 1;
  }

  CIMClass cldef;
  try
  {
    cldef = _c.getClass( _nameSpace, argv[0], false );
  }
  catch (CIMException& e)
  {
    cerr << "Error: [" << e.getCode() << "] " <<
        cimStatusCodeToString(e.getCode()) << endl;
    return 1;
  }

  // Display the class definition
  // without qualifiers, for the moment

  // First the class name and superclass
  cout << endl;
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
// enumerateClasses
// ===============================================================
int _enumerateClasses(const int argc, const char **argv)
{
  cerr << "Not yet implemented" << endl;
  return 1;
}

// ===============================================================
// enumerateClassNames
// ===============================================================
int _enumerateClassNames(const int argc, const char **argv)
{
  // This curious function calls a version of itself recursively
  // so that it can display the classes in tree form

  // We want to indent by an increasing amount at each level,
  // starting with no indentation
  String tab;

  // First time we call the client API in a try/catch to
  // catch possible bad class spec supplied by user.
  // Subsequent calls in the recursive function don't need to.
  Array<CIMName> cn;
  try
  {
    if (argc < 1)
      cn = _c.enumerateClassNames( _nameSpace);
    else
      cn = _c.enumerateClassNames( _nameSpace, argv[0] );
  }
  catch (CIMException& e)
  {
    cerr << "Error: [" << e.getCode() << "] " <<
        cimStatusCodeToString(e.getCode()) << endl;
    return 1;
  }

  // Show namespace if not default
  cout << endl ;
  if (0 != getenv("CIM_NAMESPACE"))
  {
    if (cn.size() > 0)
    {
        cout << "Classes in namespace [" <<
            _nameSpace << "]:" << endl;
    }
    else
    {
        cout << "There are no classes in namespace [" <<
            _nameSpace << "]!" << endl;
    }
  }

  for (int i = 0; i < cn.size(); i++)
  {
    // print class name after current tab amount
    cout << tab << cn[i].getString() << endl;
    // recurse to print subclasses of this class with a larger tab
    if (_recursiveClassEnum( cn[i], tab+"  " )!=0) return 1;
  }
  return 0;
}

// ===============================================================
// getInstance
// ===============================================================
int _getInstance(const int argc, const char **argv)
{
  if (argv[0] == 0)
  {
    _giUsage();
    return 1;
  }

  // need to get class definition to find keys
  // first arg is name of class
  CIMClass cldef;
  try
  {
    cldef = _c.getClass( _nameSpace, argv[0], false );
  }
  catch (CIMException& e)
  {
    cerr << "Error: [" << e.getCode() << "] " <<
        cimStatusCodeToString(e.getCode()) << endl;
    return 1;
  }

  CIMObjectPath ref;
  CIMInstance inst;

  // If there are no more args, prompt user for keys
  cout << endl;
  if (argv[1] == 0)
  {
      ref = CIMObjectPath(String::EMPTY, // hostname left blank
                          _nameSpace,
                          argv[0],
                          _inputInstanceKeys(cldef));
  }

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
    _giUsage();
    return 1;
  }

  // get the specified instance
  try
  {
    inst = _c.getInstance(_nameSpace, ref, false, true);
  }
  catch (CIMException& e)
  {
    cerr << "Error: [" << e.getCode() << "] " <<
        cimStatusCodeToString(e.getCode()) << endl;
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
  if (argv[0] == 0)
  {
    cerr << "Usage: wmiop enumerateInstances|ei <class>" << endl;
    return 1;
  }

  Array<CIMInstance> ia;
  try
  {
    ia = _c.enumerateInstances( _nameSpace, argv[0] );
  }
  catch (CIMException& e)
  {
    cerr << "Error: [" << e.getCode() << "] " <<
        cimStatusCodeToString(e.getCode()) << endl;
    return 1;
  }

  cout << endl;

  if (ia.size() > 0)
  {
      cout << "Instances of [" << argv[0] <<
        "] (" << ia.size() << " instances):" << endl;
  }
  else
  {
      cout << "There are no instances of [" << argv[0] << "]." << endl;
  }

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
  if (argv[0] == 0)
  {
    cerr << "Usage: wmiop enumerateInstanceNames|ein <class>" << endl;
    return 1;
  }

  Array<CIMObjectPath> iNames;
  try
  {
    iNames = _c.enumerateInstanceNames( _nameSpace, argv[0] );
  }
  catch (CIMException& e)
  {
    cerr << "Error: [" << e.getCode() << "] " <<
        cimStatusCodeToString(e.getCode()) << endl;
    return 1;
  }

  cout << endl;
  if (iNames.size() > 0)
  {
      cout << "Instances of [" << argv[0] << "] (" <<
        iNames.size() << " instances):" << endl;
  }
  else
  {
      cout << "There are no instances of [" << argv[0] << "]." << endl;
  }

  for (int i=0; i<iNames.size(); i++)
    cout << "  " << iNames[i].toString() << endl;
  return 0;
}

// ===============================================================
// getProperty
// ===============================================================
int _getProperty(const int argc, const char **argv)
{
  if (argc < 2)
  {
    _gpUsage();
    return 1;
  }

  // need to get class definition to find keys
  // first arg is name of class
  CIMClass cldef;
  try
  {
    cldef = _c.getClass( _nameSpace, argv[0], false );
  }
  catch (CIMException& e)
  {
    cerr << "Error: [" << e.getCode() << "] " <<
        cimStatusCodeToString(e.getCode()) << endl;
    return 1;
  }

  CIMObjectPath ref;
  CIMInstance inst;

  // If next arg is "ask", prompt user for keys
  cout << endl;
  if (String::equalNoCase("ask",argv[1]))
  {
       ref = CIMObjectPath(String::EMPTY,
                          _nameSpace,
                          argv[0],
                          _inputInstanceKeys(cldef) );
  }
  // else if the next arg and is "list", enumInstNames and print
  // a list from which user will select
  else if (String::equalNoCase("list",argv[1]))
  {
    ref = _selectInstance( argv[0] );
    if (ref.identical(CIMObjectPath())) return 0;
  }
  else
  {
    // else there's another arg but it's invalid
    _gpUsage();
    return 1;
  }

  CIMProperty pDef;
  // if no more args, display property names and ask which
  if (argc < 3)
  {
    cout << endl << "Please select the property desired:" << endl;
    int n;
    for (n=0; n<cldef.getPropertyCount(); n++)
    {
      pDef = cldef.getProperty(n);
      cout << n+1 << ": ";
      cout << cimTypeToString(pDef.getType()) << " ";
      cout << pDef.getName().getString();
      if (pDef.isArray()) cout << "[]";
      cout << endl;
    }
    cout << "Property (1.." << cldef.getPropertyCount() << ")? " << flush;
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
    v = _c.getProperty( _nameSpace, ref, pDef.getName().getString() );
  }
  catch (CIMException& e)
  {
    cerr << "Error: [" << e.getCode() << "] " <<
        cimStatusCodeToString(e.getCode()) << endl;
    return 1;
  }

  cout << endl << "Instance: " << ref.toString() << endl;
  cout << "Property: " << pDef.getName().getString();

  if (v.isArray()) cout << "[" << v.getArraySize() << "]";

  cout << endl << "Value: ";

  if (v.isNull()) cout << "NULL";
  else cout << "\"" << v.toString() << "\"";

  cout << endl;
  return 0;
}

// ===============================================================
// setProperty
// ===============================================================
int _setProperty(const int argc, const char **argv)
{
#if 0
  cerr << "Not yet implemented" << endl;
  return 1;
#endif

  if (argc < 2)
  {
    _spUsage();
    return 1;
  }

  // need to get class definition to find keys
  // first arg is name of class
  CIMClass cldef;
  try
  {
    cldef = _c.getClass( _nameSpace, argv[0], false );
  }
  catch (CIMException& e)
  {
    cerr << "Error: [" << e.getCode() << "] " <<
        cimStatusCodeToString(e.getCode()) << endl;
    return 1;
  }

  CIMObjectPath ref;
  CIMInstance inst;

  // If next arg is "ask", prompt user for keys
  if (String::equalNoCase("ask",argv[1]))
  {
      ref = CIMObjectPath(String::EMPTY,
                          _nameSpace,
                          argv[0],
                          _inputInstanceKeys(cldef) );
  }
  // else if the next arg is "list", do enumInstNames and print
  // a list from which user will select
  else if (String::equalNoCase("list",argv[1]))
  {
    ref = _selectInstance( argv[0] );
    // Exit if a null reference was returned
    if (ref.identical(CIMObjectPath())) return 0;
  }

  // else the arg is invalid
  else
  {
    _spUsage();
    return 1;
  }

  CIMProperty pDef;
  // if no more args, display property names and ask which to set
  if (argc < 3)
  {
    cout << endl << "Please select the property desired:" << endl;
    int n;
    for (n=0; n<cldef.getPropertyCount(); n++)
    {
      cout << n+1 << ": ";
      cout << cimTypeToString(cldef.getProperty(n).getType()) << " ";
      cout << cldef.getProperty(n).getName().getString();
      if (cldef.getProperty(n).isArray()) cout << "[]";
      cout << endl;
    }
    cout << "Property (1.." << cldef.getPropertyCount() << ")? " << flush;
    cin >> n;
    pDef = cldef.getProperty(n-1);
  }

  // else use next arg as name of property to set
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

  if (pDef.isArray())
  {
    cout << "setProperty for arrays not yet implemented" << endl;
    return 1;
  }

  char v[1024];
  // If value was not specified, ask for one
  if (argc < 4)
  {
    cout << endl << "New value? " << flush;
    cin >> v;
  }
  else strcpy(v, argv[3]);

  // Now we can call setProperty()
  try
  {
    _c.setProperty( _nameSpace, ref,
        pDef.getName().getString(), _makeValue(v,pDef) );
  }
  catch (CIMException& e)
  {
    cerr << "Error: [" << e.getCode() << "] " <<
        cimStatusCodeToString(e.getCode()) << endl;
    return 1;
  }

  cout << endl;
  cout << "Instance : " << ref.toString() << endl;
  cout << "Property : " << pDef.getName().getString() << endl;
  cout << "New Value: " << _makeValue(v,pDef).toString() << endl;

  return 0;
}

// ===============================================================
// invokeMethod
// ===============================================================

int _invokeMethod(const int argc, const char **argv)
{
  cerr << "Not yet implemented" << endl;
  return 1;
}

// ===============================================================
// createClass
// ===============================================================

int _createClass(const int argc, const char **argv)
{
  cerr << "Not yet implemented" << endl;
  return 1;
}

// ===============================================================
// modifyClass
// ===============================================================

int _modifyClass(const int argc, const char **argv)
{
  cerr << "Not yet implemented" << endl;
  return 1;
}

// ===============================================================
// deleteClass
// ===============================================================

int _deleteClass(const int argc, const char **argv)
{
  if (argv[0] == 0)
  {
    cerr << "Usage: wmiop deleteClass|dc <class>" << endl;
    return 1;
  }

  try
  {
    _c.deleteClass( _nameSpace, argv[0] );
  }
  catch (CIMException& e)
  {
    cerr << "Error: [" << e.getCode() << "] " <<
        cimStatusCodeToString(e.getCode()) << endl;
    return 1;
  }

  cout << "Class [" << argv[0] << "] successfully deleted!" << endl;
  return 0;
}

// ===============================================================
// createInstance
// ===============================================================
int _createInstance(const int argc, const char **argv)
{
  if (argc < 1)
  {
    _ciUsage();
    return 1;
  }

  // get class definition
  CIMClass cldef;

  // Handle special case of __Namespace
  if (String::equalNoCase(argv[0],"__namespace"))
  {
      cldef = _makeNamespaceClass();
  }
  else
  {
    try
    {
      cldef = _c.getClass( _nameSpace, argv[0], false );
    }
    catch (CIMException& e)
    {
      cerr << "Error: [" << e.getCode() << "] " <<
          cimStatusCodeToString(e.getCode()) << endl;
      return 1;
    }
  }

  CIMInstance tmpInst(cldef.getClassName());
  Array<CIMKeyBinding> keys;

  // prompt user for property values
  cout << endl << "Please fill in the properties below:" << endl;
  for (int i=0; i<cldef.getPropertyCount(); i++)
  {
    // display the property
    CIMProperty pDef(cldef.getProperty(i));

    if (_isKey(pDef)) cout << "[ key ] ";
    cout << cimTypeToString(pDef.getType()) << " " <<
        pDef.getName().getString();
    if (pDef.isArray()) cout << "[]";
    if (pDef.isArray())
    {
      cerr << ": Array properties not yet implemented" << endl;
      continue;
    }
    cout << "? " << flush;
    // ask for a value
    char v[1024];
    gets(v);

    // insert thusly specified property in instance
    pDef.setValue(_makeValue(v,pDef));
    tmpInst.addProperty(pDef);
    // And in a CIMKeyBinding array if it's a key
    if (_isKey(pDef)) keys.append(_makeKey(pDef));
  }

  // Set the object path
  try
  {
    tmpInst.setPath(CIMObjectPath(String::EMPTY,         // host
                                  _nameSpace,            // namespace
                                  cldef.getClassName(),  // classname
                                  keys));                // keybindings
  }
  catch (Exception &e)
  {
    cerr << e.getMessage() << endl;
    return 1;
  }

  // Submit the create operation to the cimserver
  try
  {
    _c.createInstance( _nameSpace, tmpInst);
  }
  catch (CIMException& e)
  {
    cerr << "Error: [" << e.getCode() << "] " <<
        cimStatusCodeToString(e.getCode()) << endl;
    return 1;
  }

  cout << endl << "Instance [" << tmpInst.getPath().toString() <<
      "] successfully created!" << endl;
  return 0;
}

// ===============================================================
// modifyInstance
// ===============================================================
int _modifyInstance(const int argc, const char **argv)
{
  if (argc < 1)
  {
    _miUsage();
    return 1;
  }

  // need to get class definition to find keys
  // first arg is name of class
  CIMClass cldef;
  try
  {
    cldef = _c.getClass( _nameSpace, argv[0], false );
  }
  catch (CIMException& e)
  {
    cerr << "Error: [" << e.getCode() << "] " <<
        cimStatusCodeToString(e.getCode()) << endl;
    return 1;
  }

  CIMObjectPath ref;
  CIMInstance inst;

  // If no more args, prompt user for keys
  if (argc < 2)
  {
      ref = CIMObjectPath(String::EMPTY,
                          _nameSpace,
                          argv[0],
                          _inputInstanceKeys(cldef) );
  }
  // if the next arg is "list", do enumInstNames and print
  // a list from which user will select
  else if (String::equalNoCase("list",argv[1]))
  {
    ref = _selectInstance( argv[0] );
    // Exit if a null reference was returned
    if (ref.identical(CIMObjectPath())) return 0;
  }

  // else the arg is invalid
  else
  {
    _miUsage();
    return 1;
  }

  // get a copy of existing instance for display
  try
  {
    inst = _c.getInstance(_nameSpace, ref, false, true);
  }
  catch (CIMException& e)
  {
    cerr << "Error: [" << e.getCode() << "] " <<
        cimStatusCodeToString(e.getCode()) << endl;
    return 1;
  }




  // display property names and ask which to modify
  cout << endl << "Select the property to be updated:" << endl;
  int n;
  for (n=0; n<cldef.getPropertyCount(); n++)
  {
    cout << n+1 << ": ";
    Uint32 p = inst.findProperty(cldef.getProperty(n).getName());
    if (p != PEG_NOT_FOUND)
      cout << _displayProperty(inst.getProperty(p)) << endl;
    else
      cout << cldef.getProperty(n).getName().getString() <<
        " <absent>" << endl;
  }




  CIMInstance tmpInst(cldef.getClassName());
  Array<CIMName> pNames;
  while (true) // break when user enters escape value
  {
    cout << "Property (1.." << cldef.getPropertyCount() <<
        ", 0=quit, 999=all)? " << flush;
    cin >> n;

    if (n==0 || n==999) break;
    if (n > cldef.getPropertyCount() || n < 0) continue;

    CIMProperty pDef(cldef.getProperty(n-1));
    if (pDef.isArray())
    {
      cerr << "Array properties not yet implemented" << endl;
      continue;
    }

    // Ask for value
    cout << "New value for [" << cimTypeToString(pDef.getType()) << " " <<
        pDef.getName().getString() << "]? " << flush;
    char v[1024];
    cin >> v;

    // remove property if one of same name exists
    int p = tmpInst.findProperty(pDef.getName().getString());
    if (p != PEG_NOT_FOUND)
        tmpInst.removeProperty(p);

    // insert a property with this value in the instance
    pDef.setValue(_makeValue(v,pDef));
    tmpInst.addProperty(pDef);

    // add to array of names if not already there
    for (p=0; p < pNames.size(); p++)
      if (pNames[p]==pDef.getName()) break;

    if (p == pNames.size())
      pNames.append(pDef.getName());
  }

  CIMPropertyList pList;

  // need to re-ask if n=999, and use null propertyList
  if (n==999)
    for (int i=0; i<cldef.getPropertyCount(); i++)
    {
      CIMProperty pDef(cldef.getProperty(i));
      cout << "New value for " << cimTypeToString(pDef.getType()) << " " <<
          pDef.getName().getString();
      if (pDef.isArray()) cout << "[]";
      if (pDef.isArray())
      {
        cerr << ": Array properties not yet implemented" << endl;
        continue;
      }
      cout << "? " << flush;
      char v[1024];
      cin >> v;

      pDef.setValue(_makeValue(v,pDef));
      int j = tmpInst.findProperty(pDef.getName());
      if (j!=PEG_NOT_FOUND) tmpInst.removeProperty(j);
      tmpInst.addProperty(pDef);
      // don't bother with pNames array
    }
  else
    pList = CIMPropertyList(pNames);

  // Don't forget to set the object path in this instance
  tmpInst.setPath(ref);

  // Now we can call modifyInstance()
  try
  {
    _c.modifyInstance(_nameSpace, tmpInst, false, pList);
  }
  catch (CIMException& e)
  {
    cerr << "Error: [" << e.getCode() << "] " <<
        cimStatusCodeToString(e.getCode()) << endl;
    return 1;
  }

  cout << endl << "Instance [" << tmpInst.getPath().toString() <<
      "] successfully updated!" << endl;
  return 0;
}

// ===============================================================
// deleteInstance
// ===============================================================

int _deleteInstance(const int argc, const char **argv)
{
  if (argv[0] == 0)
  {
    _diUsage();
    return 1;
  }

  // need to get class definition to find keys
  // first arg is name of class
  CIMClass cldef;

  // Handle special case of __Namespace
  if (String::equalNoCase(argv[0],"__namespace"))
  {
      cldef = _makeNamespaceClass();
  }
  else
  {
    try
    {
      cldef = _c.getClass( _nameSpace, argv[0], false );
    }
    catch (CIMException& e)
    {
      cerr << "Error: [" << e.getCode() << "] " <<
          cimStatusCodeToString(e.getCode()) << endl;
      return 1;
    }
  }

  CIMObjectPath ref;
  CIMInstance inst;

  // If there are no more args, prompt user for keys
  if (argv[1] == 0)
  {

      ref = CIMObjectPath(String::EMPTY, // hostname left blank
                          _nameSpace,
                          argv[0],
                          _inputInstanceKeys(cldef));
  }

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
    _diUsage();
    return 1;
  }

  // delete the specified instance
  try
  {
    _c.deleteInstance(_nameSpace, ref);
  }
  catch (CIMException& e)
  {
    cerr << "Error: [" << e.getCode() << "] " <<
        cimStatusCodeToString(e.getCode()) << endl;
    return 1;
  }

  cout << endl << "Instance [" << ref.toString() <<
      "] successfully deleted!" << endl;
  return 0;
}

// ===============================================================
// associators
// ===============================================================

int _associators(const int argc, const char **argv)
{
  cerr << "Not yet implemented" << endl;
  return 1;
}

// ===============================================================
// associatorNames
// ===============================================================

int _associatorNames(const int argc, const char **argv)
{
  cerr << "Not yet implemented" << endl;
  return 1;
}

// ===============================================================
// references
// ===============================================================

int _references(const int argc, const char **argv)
{
  cerr << "Not yet implemented" << endl;
  return 1;
}

// ===============================================================
// referenceNames
// ===============================================================

int _referenceNames(const int argc, const char **argv)
{
  cerr << "Not yet implemented" << endl;
  return 1;
}

// ===============================================================
// execQuery
// ===============================================================

int _execQuery(const int argc, const char **argv)
{
  cerr << "Not yet implemented" << endl;
  return 1;
}

// ===============================================================
// getQualifier
// ===============================================================

int _getQualifier(const int argc, const char **argv)
{
  cerr << "Not yet implemented" << endl;
  return 1;
}

// ===============================================================
// setQualifier
// ===============================================================

int _setQualifier(const int argc, const char **argv)
{
  cerr << "Not yet implemented" << endl;
  return 1;
}

// ===============================================================
// deleteQualifier
// ===============================================================

int _deleteQualifier(const int argc, const char **argv)
{
  cerr << "Not yet implemented" << endl;
  return 1;
}

// ===============================================================
// enumerateQualifiers
// ===============================================================

int _enumerateQualifiers(const int argc, const char **argv)
{
  cerr << "Not yet implemented" << endl;
  return 1;
}

// ===============================================================
// Auxiliary function _inputInstanceKeys()
//
// Prompt user for values of key properties of specified class
// ===============================================================
Array<CIMKeyBinding> _inputInstanceKeys(CIMClass &cldef)
{
  Array<CIMKeyBinding> kb;
  for (int i = 0; i < cldef.getPropertyCount(); i++)
  {

    CIMProperty prop = cldef.getProperty(i);

    if (_isKey(prop))
    {
      char s[1024];
      do
      {
        cout << "Please insert the value for the key property [";
        cout << prop.getName().getString() << " (" <<
            cimTypeToString(prop.getType()) << ")]: " << flush;
        gets(s);
      }
      while (strlen(s) == 0);

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
        // From PEP 194: EmbeddedObjects cannot be
        // keys (to be verified with DMTF via CR)
      case CIMTYPE_OBJECT:
        break;
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
    iNames = _c.enumerateInstanceNames(_nameSpace, clnam);
  }
  catch (CIMException& e)
  {
    cerr << "Error: [" << e.getCode() << "] " <<
        cimStatusCodeToString(e.getCode()) << endl;
    return CIMObjectPath();
  }

  if (iNames.size() == 0)
  {
    cerr << "No instances found." << endl;
    return CIMObjectPath();
  }

  // Display them, numbered starting at 1
  cout << "Please select the instance desired:" << endl;
  int i;
  for (i=0; i<iNames.size(); i++)
    cout << i+1 << ": " << iNames[i].toString() << endl;
  i = 0;

  // Ask user to pick one
  while (i < 1 || i > iNames.size())
  {
    cout << "Instance (1.." << iNames.size() << ")? " << flush;
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
  cout << endl;
  cout << "Instance of " << inst.getClassName().getString() << ":" << endl;
  cout << "{" << endl;

  for (int j=0; j<inst.getPropertyCount(); j++)
    cout << "  " << _displayProperty(inst.getProperty(j)) << endl;

  cout << "}" << endl;
}


// ===============================================================
// Auxiliary function _isKey()
// ===============================================================

Boolean _isKey(const CIMProperty &p)
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

// ===============================================================
// Auxiliary function _displayProperty()
// ===============================================================

String _displayProperty(const CIMProperty &p)
{
  String s;

  if (_isKey(p)) s.append("[ key ] ");

  s.append(p.getName().getString());

  CIMValue v = p.getValue();

  if (v.isArray())
  {
    s.append("[");
    //s.append(ultoa(v.getArraySize()));
    s.append(v.getArraySize());
    s.append("]");
  }

  s.append(" = ");

  if (v.isNull())
      s.append("<empty>");
  else if (v.getType() == CIMTYPE_STRING ||
           v.getType() == CIMTYPE_CHAR16 ||
           v.getType() == CIMTYPE_DATETIME)
  {
   s.append("\"");
   s.append(v.toString());
   s.append("\"");
  }
  else
    s.append(v.toString());

  return s;
}


// ===============================================================
// Auxiliary function _usage()
// ===============================================================

void _usage()
{
  cerr << "Usage:" << endl;
  cerr << "  wmiop <wmioperation> [arg, ...]" << endl;
  cerr << "Implemented operations (not case sensitive) are:" << endl;
  cerr << "  getClass|gc <class>" << endl;
  cerr << "  enumerateClassNames|ecn [ <class> ]" << endl;
  cerr << "  getInstance|gi <class> [ list ]" << endl;
  cerr << "  enumerateInstances|ei <class>" << endl;
  cerr << "  enumerateInstanceNames|ein <class>" << endl;
  cerr << "  getProperty|gp <class> { ask | list } [ <propnam> ]" << endl;
  cerr << "  setProperty|sp <class> { ask | list } "
      "[ <propnam> [ <value> ] ]" << endl;
  cerr << "  deleteClass|dc <class>" << endl;
  cerr << "  createInstance|ci <class>" << endl;
  cerr << "  modifyInstance|mi <class> [ list ]" << endl;
  cerr << "  deleteInstance|di <class> [ list ]" << endl;
  cerr << "Examples:" << endl;
  cerr << "  wmiop ecn" << endl;
  cerr << "  wmiop enumerateinstancenames pg_operatingsystem" << endl;
  cerr << "  wmiop gi pg_unixprocess list" << endl;
  cerr << "Environment variables:" << endl;
  cerr << "  CIM_NAMESPACE -- if not defined use root/cimv2" << endl;
}

// ===============================================================
// Auxiliary function _giUsage()
// ===============================================================

void _giUsage()
{
  cerr << "Usage: wmiop getInstance|gi <class> [list]" << endl;
  cerr << "  \"list\" displays list from which to choose" << endl;
  cerr << "  otherwise asks for keys (empty ok for many)" << endl;
}


// ===============================================================
// Auxiliary function _diUsage()
// ===============================================================

void _diUsage()
{
  cerr << "Usage: wmiop deleteInstance|di <class> [ list ]" << endl;
  cerr << "  \"list\" displays list from which to choose" << endl;
  cerr << "  otherwise asks for keys (empty ok for many)" << endl;
}


// ===============================================================
// Auxiliary function _gpUsage()
// ===============================================================

void _gpUsage()
{
  cerr << "Usage: wmiop getProperty|gp <class> "
      "{ ask | list } [<propnam>]" << endl;
  cerr << "  \"list\" displays list from which to choose" << endl;
  cerr << "  \"ask\" asks for keys (empty ok for many)" << endl;
}


// ===============================================================
// Auxiliary function _miUsage()
// ===============================================================

void _miUsage()
{
  cerr << "Usage: wmiop modifyInstance|mi <class> [ list ]" << endl;
  cerr << "  \"list\" displays list from which to choose" << endl;
  cerr << "  otherwise asks for keys (empty ok for many)" << endl;
}


// ===============================================================
// Auxiliary function _ciUsage()
// ===============================================================

void _ciUsage()
{
  cerr << "Usage: wmiop createInstance|ci <class>" << endl;
}


// ===============================================================
// Auxiliary function _spUsage()
// ===============================================================

void _spUsage()
{
  cerr << "Usage: wmiop setProperty|sp <class> { ask | list } "
      "[ <propnam> [ <value> ] ]" << endl;
  cerr << "  \"list\" displays list from which to choose" << endl;
  cerr << "  \"ask\" asks for keys (empty ok for many)" << endl;
}


// ===============================================================
// Auxiliary function _makeKey()
//
// Build a CIMKeyBinding object from the specified property
// ===============================================================
CIMKeyBinding _makeKey(const CIMProperty &prop)
{
  // Figure out what type of key
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

  // Return a built CIMKeyBinding
  return CIMKeyBinding(prop.getName(),              // name
                    prop.getValue().toString(),  // value
                    t);                          // type
}


// ===============================================================
// Auxiliary function _makeValue()
// ===============================================================

CIMValue _makeValue(const char *v, const CIMProperty &pDef)
{
  // create a value from a string depending on property's type
  // start with null value and then set it appropriately
  CIMValue val;
  switch (pDef.getType())
  {
    case CIMTYPE_BOOLEAN:
    {
      val.set(String::equalNoCase(v,"true"));
      break;
    }

    case CIMTYPE_UINT8:
    {
      Uint8 t;
      sscanf(v,"%hu",&t);
      val.set(t);
      break;
    }

    case CIMTYPE_SINT8:
    {
      Sint8 t;
      sscanf(v,"%hd",&t);
      val.set(t);
      break;
    }

    case CIMTYPE_UINT16:
    {
      Uint16 t;
      sscanf(v,"%hu",&t);
      val.set(t);
      break;
    }

    case CIMTYPE_SINT16:
    {
      Sint16 t;
      sscanf(v,"%hd",&t);
      val.set(t);
      break;
    }

    case CIMTYPE_UINT32:
    {
      Uint32 t;
      sscanf(v,"%u",&t);
      val.set(t);
      break;
    }

    case CIMTYPE_SINT32:
    {
      Sint32 t;
      sscanf(v,"%d",&t);
      val.set(t);
      break;
    }

    case CIMTYPE_UINT64:
    {
      Uint64 t;
      sscanf(v,"%I64u",&t);
      val.set(t);
      break;
    }

    case CIMTYPE_SINT64:
    {
      Sint64 t;
      sscanf(v,"%I64d",&t);
      val.set(t);
      break;
    }

    case CIMTYPE_REAL32:
    {
      Real32 t;
      sscanf(v,"%f",&t);
      val.set(t);
      break;
    }

    case CIMTYPE_REAL64:
    {
      Real64 t;
      sscanf(v,"%lf",&t);
      val.set(t);
      break;
    }

    case CIMTYPE_CHAR16:
    {
      Char16 t;
      sscanf(v,"%hu",&t);
      val.set(t);
      break;
    }

    case CIMTYPE_STRING:
    {
      val.set(String(v));
      break;
    }

    case CIMTYPE_DATETIME:
    {
      try
      {
        val.set(CIMDateTime(v));
      }
      catch (Exception &e)
      {
        cerr << e.getMessage() << endl;
        exit(1);
      }
      break;
    }

    case CIMTYPE_REFERENCE:
    {
      val.set(CIMObjectPath(v));
      break;
    }

    default: break; // can't happen
  }
  return val;
}


// ==============================================================
// Auxiliary function _makeNamespaceClass()
//
// Returns a class definition of __Namespace
// ==============================================================

CIMClass _makeNamespaceClass()
{
    CIMClass cldef("__Namespace");
    CIMProperty p("Name",CIMValue(CIMTYPE_STRING,false));
    p.addQualifier(CIMQualifier("Key",                         // name
                        CIMValue(true),                // value
                        CIMFlavor(CIMFlavor::DISABLEOVERRIDE))); // flavor
    cldef.addProperty(p);
    return cldef;
}


// ===============================================================
// Auxiliary function _recursiveClassEnum()
// ===============================================================

int _recursiveClassEnum(const CIMName &className, String tab)
{
  Array<CIMName> cn;

  try
  {
    cn = _c.enumerateClassNames( _nameSpace, className );
  }
  catch (CIMException& e)
  {
    cerr << "Error: [" << e.getCode() << "] " <<
        cimStatusCodeToString(e.getCode()) << endl;
    return 1;
  }

  for (int i=0; i<cn.size(); i++)
  {
    cout << tab << cn[i].getString() << endl;
    if (_recursiveClassEnum( cn[i], tab+"  " ) != 0) return 1;
  }
  return 0;
}

Boolean verifyServerCertificate(SSLCertificateInfo &certInfo)
{
    return true;
}
