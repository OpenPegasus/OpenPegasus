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
// Author:      Adrian Schuur, schuur@de.ibm.com
//
// Modified By: Mark Hamzy, hamzy@us.ibm.com
//
//%/////////////////////////////////////////////////////////////////////////////
package org.pegasus.jmpi;

import java.util.*;
import org.pegasus.jmpi.*;

public class CIMClient
{
    public static final String  WQL  = "WQL";
    public static final String  WQL1 = "WQL1";
    public static final String  WQL2 = "WQL2";
    public static final int     HTTP = 2;
    public static final int     XML  = 2;
    public static final boolean DEEP = true;

    private CIMNameSpace ns;
    private long         cInst;
    private long         cNsInst;

    private native long   _newNaUnPw              (long          name,
                                                   String        userName,
                                                   String        passWord);

    private native void   _disconnect             (long          cc);
    private native long   _getClass               (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   boolean       localOnly,
                                                   boolean       includeQualifiers,
                                                   boolean       includeClassOrigin,
                                                   String        propertyList[]);
    private native void   _deleteClass            (long          cc,
                                                   long          ns,
                                                   long          path);
    private native void   _createClass            (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   long          ci);
    private native void   _setClass               (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   long          ci);

    private native long   _getInstance            (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   boolean       localOnly,
                                                   boolean       includeQualifiers,
                                                   boolean       includeClassOrigin,
                                                   String        propertyList[]);
    private native void   _deleteInstance         (long          cc,
                                                   long          ns,
                                                   long          path);
    private native long   _createInstance         (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   long          ci);
    private native void   _modifyInstance         (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   long          ci,
                                                   boolean       includeQualifiers,
                                                   String        propertyList[]);

    private native long   _enumerateClasses       (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   boolean       deep,
                                                   boolean       localOnly,
                                                   boolean       includeQualifiers,
                                                   boolean       includeClassOrigin);
    private native long   _enumerateClassNames    (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   boolean       deep);
    private native long   _enumerateInstanceNames (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   boolean       deep);
    private native long   _enumerateInstances     (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   boolean       deep,
                                                   boolean       localOnly,
                                                   boolean       includeQualifiers,
                                                   boolean       includeClassOrigin,
                                                   String        propertyList[]);
    private native long   _enumerateQualifiers    (long          cc,
                                                   long          ns,
                                                   long          path);

    private native long   _getQualifier           (long          cc,
                                                   long          ns,
                                                   long          path);
    private native void   _setQualifier           (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   long          type);
    private native void   _deleteQualifier        (long          cc,
                                                   long          ns,
                                                   long          path);

    private native long   _getProperty            (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   String        propertyName);
    private native void   _setProperty            (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   String        propertyName,
                                                   long          newValue);

    private native long   _execQuery              (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   String        query,
                                                   String        ql);

    private native long   _invokeMethod           (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   String        methodName,
                                                   Vector        inParams,
                                                   Vector        outParams)
          throws CIMException;
    private native long   _invokeMethod24         (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   String        methodName,
                                                   CIMArgument[] inParams,
                                                   CIMArgument[] outParams)
          throws CIMException;

    private native long   _associatorNames        (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   String        assocClass,
                                                   String        resultClass,
                                                   String        role,
                                                   String        resultRole);
    private native long   _associators            (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   String        assocClass,
                                                   String        resultClass,
                                                   String        role,
                                                   String        resultRole,
                                                   boolean       includeQualifiers,
                                                   boolean       includeClassOrigin,
                                                   String        propertyList[]);
    private native long   _referenceNames         (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   String        resultClass,
                                                   String        role);
    private native long   _references             (long          cc,
                                                   long          ns,
                                                   long          path,
                                                   String        resultClass,
                                                   String        role,
                                                   boolean       includeQualifiers,
                                                   boolean       includeClassOrigin,
                                                   String        propertyList[]);

    private native void   _createNameSpace        (long          cc,
                                                   String        ns);
    private native Vector _enumerateNameSpaces    (long          cc,
                                                   long          path,
                                                   boolean       deep,
                                                   Vector        v);
    private native void   _deleteNameSpace        (long          cc,
                                                   String        ns);

    private native void   _finalize               (long          cInst);

    protected void finalize ()
    {
        _finalize (cInst);
    }

    protected long cInst ()
    {
        return cInst;
    }

    public CIMNameSpace getNameSpace ()
    {
        return ns;
    }

    public CIMClient(CIMNameSpace nameSpace,
                     String       userName,
                     String       pword)
         throws CIMException
    {
        ns      = nameSpace;
        cNsInst = nameSpace.cInst ();
        cInst   = _newNaUnPw (cNsInst, userName, pword);
    }

    public CIMClient (CIMNameSpace nameSpace,
                      String       userName,
                      String       pword,
                      long         type)
         throws CIMException
    {
        if (type != HTTP)
            throw new CIMException ("Specified protocol type not supported.");

        ns      = nameSpace;
        cNsInst = nameSpace.cInst ();
        cInst   = _newNaUnPw (cNsInst, userName, pword);
    }
/*
    public CIMClient (CIMNameSpace name,
                      String       userName,
                      String       pword,
                      String       roleName,
                      String       rolePwd)
         throws CIMException
    {
    }

    public CIMClient (CIMNameSpace name,
                      String       userName,
                      String       pword,
                      String       roleName,
                      String       rolePwd,
                      long         type)
         throws CIMException
    {
    }
*/

    public void close ()
         throws CIMException
    {
       if (cInst != 0)
       {
          _disconnect (cInst);
       }
    }

    public void createNameSpace (CIMNameSpace ins)
         throws CIMException
    {
       if (cInst != 0)
       {
          _createNameSpace (cInst, ins.getNameSpace ());
       }
    }

    public void deleteNameSpace (CIMNameSpace cns)
         throws CIMException
    {
       if (cInst != 0)
       {
          _deleteNameSpace (cInst, cns.getNameSpace ());
       }
    }

    public void deleteClass (CIMObjectPath path)
         throws CIMException
    {
       if (cInst != 0)
       {
          _deleteClass (cInst, cNsInst, path.cInst ());
       }
    }

    public void deleteInstance (CIMObjectPath path)
         throws CIMException
    {
       if (cInst != 0)
       {
          _deleteInstance (cInst, cNsInst, path.cInst ());
       }
    }

    public Enumeration enumNameSpace (CIMObjectPath path,
                                      boolean       deep)
         throws CIMException
    {
       Vector ret = new Vector ();

       if (cInst != 0)
       {
          _enumerateNameSpaces (cInst, path.cInst (), deep, ret);
       }

       return ret.elements();
    }

    public Enumeration enumerateNameSpaces (CIMObjectPath path,
                                            boolean       deepInheritance)
         throws CIMException
    {
       return enumNameSpace (path, deepInheritance);
    }

    public Enumeration enumClass (CIMObjectPath path,
                                  boolean       deep,
                                  boolean       local)
         throws CIMException
    {
       long ciEnumeration = 0;

       if (cInst != 0)
       {
          ciEnumeration = _enumerateClasses (cInst,
                                             cNsInst,
                                             path.cInst (),
                                             deep,
                                             local,
                                             true,
                                             false);
       }

       if (ciEnumeration != 0)
       {
          return new ClassEnumeration (ciEnumeration);
       }
       else
       {
          return null;
       }
    }

    public Enumeration enumerateClasses (CIMObjectPath path,
                                         boolean       deepInheritance,
                                         boolean       localOnly,
                                         boolean       includeQualifiers,
                                         boolean       includeClassOrigin)
         throws CIMException
    {
       long ciEnumeration = 0;

       if (cInst != 0)
       {
          ciEnumeration = _enumerateClasses (cInst,
                                             cNsInst,
                                             path.cInst (),
                                             deepInheritance,
                                             localOnly,
                                             includeQualifiers,
                                             includeClassOrigin);
       }

       if (ciEnumeration != 0)
       {
          return new ClassEnumeration (ciEnumeration);
       }
       else
       {
          return null;
       }
    }

    public Enumeration enumClass (CIMObjectPath path,
                                  boolean       deep)
         throws CIMException
    {
       long ciEnumeration = 0;

       if (cInst != 0)
       {
          ciEnumeration = _enumerateClassNames (cInst,
                                                cNsInst,
                                                path.cInst (),
                                                deep);
       }

       if (ciEnumeration != 0)
       {
          return new PathEnumeration (ciEnumeration);
       }
       else
       {
          return null;
       }
    }

    public Enumeration enumerateClassNames (CIMObjectPath path,
                                            boolean       deep)
         throws CIMException
    {
       return enumClass (path, deep);
    }

    public Enumeration enumInstances (CIMObjectPath path,
                                      boolean       deep)
         throws CIMException
    {
       long ciEnumeration = 0;

       if (cInst != 0)
       {
          ciEnumeration = _enumerateInstanceNames (cInst,
                                                   cNsInst,
                                                   path.cInst (),
                                                   deep);
       }

       if (ciEnumeration != 0)
       {
          return new PathEnumeration (ciEnumeration);
       }
       else
       {
          return null;
       }
    }

    public Enumeration enumerateInstanceNames (CIMObjectPath path)
         throws CIMException
    {
       return enumInstances (path, false);
    }

    public Enumeration enumInstances (CIMObjectPath path,
                                      boolean       deep,
                                      boolean       localOnly)
         throws CIMException
    {
       return enumerateInstances (path,
                                  deep,
                                  localOnly,
                                  true,
                                  false,
                                  (String[])null);
    }

    public Enumeration enumerateInstances (CIMObjectPath path,
                                           boolean       deepInheritance,
                                           boolean       localOnly,
                                           boolean       includeQualifiers,
                                           boolean       includeClassOrigin,
                                           String        propertyList[])
         throws CIMException
    {
       long ciEnumeration = 0;

       if (cInst != 0)
       {
          ciEnumeration = _enumerateInstances (cInst,
                                               cNsInst,
                                               path.cInst (),
                                               deepInheritance,
                                               localOnly,
                                               includeQualifiers,
                                               includeClassOrigin,
                                               propertyList);
       }

       if (ciEnumeration != 0)
       {
          return new InstEnumeration (ciEnumeration);
       }
       else
       {
          return null;
       }
    }

    public CIMClass getClass (CIMObjectPath path,
                              boolean       localOnly)
         throws CIMException
    {
       return getClass (path, localOnly, true, false, (String[])null);
    }

    public CIMClass getClass (CIMObjectPath path,
                              boolean       localOnly,
                              boolean       includeQualifiers,
                              boolean       includeClassOrigin,
                              String        propertyList[])
         throws CIMException
    {
       long ciClass = 0;

       if (cInst != 0)
       {
          ciClass = _getClass (cInst,
                               cNsInst,
                               path.cInst (),
                               localOnly,
                               includeQualifiers,
                               includeClassOrigin,
                               propertyList);
       }

       if (ciClass != 0)
       {
          return new CIMClass (ciClass);
       }
       else
       {
          return null;
       }
    }

    public CIMInstance getInstance (CIMObjectPath path,
                                    boolean       localOnly)
         throws CIMException
    {
       return getInstance (path, localOnly, true, false, (String[])null);
    }

    public CIMInstance getInstance (CIMObjectPath path,
                                    boolean       localOnly,
                                    boolean       includeQualifiers,
                                    boolean       includeClassOrigin,
                                    String        propertyList[])
         throws CIMException
    {
       long ciInstance = 0;

       if (cInst != 0)
       {
          ciInstance = _getInstance (cInst,
                                     cNsInst,
                                     path.cInst (),
                                     localOnly,
                                     includeQualifiers,
                                     includeClassOrigin,
                                     propertyList);
       }

       if (ciInstance != 0)
       {
          return new CIMInstance (ciInstance);
       }
       else
       {
          return null;
       }
    }

    public CIMValue invokeMethod (CIMObjectPath path,
                                  String        methodName,
                                  Vector        inParams,
                                  Vector        outParams)
         throws CIMException
    {
       long ciValue = 0;

       if (cInst != 0)
       {
          ciValue = _invokeMethod (cInst,
                                   cNsInst,
                                   path.cInst (),
                                   methodName,
                                   inParams,
                                   outParams);
       }

       if (ciValue != 0)
       {
          return new CIMValue (ciValue);
       }
       else
       {
          return null;
       }
    }

    public CIMValue invokeMethod (CIMObjectPath path,
                                  String        methodName,
                                  CIMArgument[] inParams,
                                  CIMArgument[] outParams)
         throws CIMException
    {
       long ciValue = 0;

       if (cInst != 0)
       {
          ciValue = _invokeMethod24 (cInst,
                                     cNsInst,
                                     path.cInst (),
                                     methodName,
                                     inParams,
                                     outParams);
       }

       if (ciValue != 0)
       {
          return new CIMValue (ciValue);
       }
       else
       {
          return null;
       }
    }

    public Enumeration enumQualifierTypes (CIMObjectPath path)

         throws CIMException
    {
       long ciEnumeration = 0;

       if (cInst != 0)
       {
          ciEnumeration = _enumerateQualifiers (cInst,
                                                cNsInst,
                                                path.cInst ());
       }

       if (ciEnumeration != 0)
       {
          return new QualEnumeration (ciEnumeration);
       }
       else
       {
          return null;
       }
    }

    public Enumeration enumerateQualifiers (CIMObjectPath path)

         throws CIMException
    {
       long ciEnumeration = 0;

       if (cInst != 0)
       {
          ciEnumeration = _enumerateQualifiers (cInst,
                                                cNsInst,
                                                path.cInst ());
       }

       if (ciEnumeration != 0)
       {
          return new QualEnumeration (ciEnumeration);
       }
       else
       {
          return null;
       }
    }

    public void deleteQualifierType (CIMObjectPath path)
         throws CIMException
    {
       if (cInst != 0)
       {
          _deleteQualifier (cInst, cNsInst, path.cInst ());
       }
    }

    public void deleteQualifier (CIMObjectPath path)
         throws CIMException
    {
       if (cInst != 0)
       {
          _deleteQualifier (cInst, cNsInst, path.cInst ());
       }
    }

    public CIMQualifierType getQualifierType (CIMObjectPath path)

         throws CIMException
    {
       long ciQualifier = 0;

       if (cInst != 0)
       {
          ciQualifier = _getQualifier (cInst,
                                       cNsInst,
                                       path.cInst ());
       }

       if (ciQualifier != 0)
       {
          return new CIMQualifierType (ciQualifier);
       }
       else
       {
          return null;
       }
    }

    public CIMQualifierType getQualifier (CIMObjectPath path)

         throws CIMException
    {
       long ciQualifier = 0;

       if (cInst != 0)
       {
          ciQualifier = _getQualifier (cInst,
                                       cNsInst,
                                       path.cInst ());
       }

       if (ciQualifier != 0)
       {
          return new CIMQualifierType (ciQualifier);
       }
       else
       {
          return null;
       }
    }

    public void createQualifierType (CIMObjectPath    path,
                                     CIMQualifierType qt)
         throws CIMException
    {
       if (cInst != 0)
       {
          _setQualifier (cInst, cNsInst, path.cInst (), qt.cInst ());
       }
    }

    public void setQualifierType (CIMObjectPath    path,
                                  CIMQualifierType qt)
         throws CIMException
    {
       if (cInst != 0)
       {
          _setQualifier (cInst, cNsInst, path.cInst (), qt.cInst ());
       }
    }

    public void setQualifier (CIMObjectPath    path,
                              CIMQualifierType qualifierType)
         throws CIMException
    {
        setQualifierType (path, qualifierType);
    }

   public void createClass (CIMObjectPath path,
                            CIMClass      cc)
         throws CIMException
    {
      if (cInst != 0)
      {
         _createClass (cInst, cNsInst, path.cInst (), cc.cInst ());
      }
    }

    public void setClass (CIMObjectPath path,
                          CIMClass      cc)
         throws CIMException
    {
       if (cInst != 0)
       {
          _setClass (cInst,cNsInst, path.cInst (), cc.cInst ());
       }
    }

    public void modifyClass (CIMObjectPath path,
                             CIMClass      modifiedClass)
         throws CIMException
    {
        setClass (path, modifiedClass);
    }

    public CIMObjectPath createInstance (CIMObjectPath path,
                                         CIMInstance   ci)
         throws CIMException
    {
       long ciObjectPath = 0;

       if (cInst != 0)
       {
          ciObjectPath = _createInstance (cInst,
                                          cNsInst,
                                          path.cInst (),
                                          ci.cInst ());
       }

       if (ciObjectPath != 0)
       {
          return new CIMObjectPath (ciObjectPath);
       }
       else
       {
          return null;
       }
    }

    public void setInstance (CIMObjectPath path,
                             CIMInstance   ci)
         throws CIMException
    {
        modifyInstance (path, ci, true, (String[])null);
    }

    public void modifyInstance (CIMObjectPath path,
                                CIMInstance   modifiedInstance,
                                boolean       includeQualifiers,
                                String        propertyList[])
         throws CIMException
    {
       if (cInst != 0)
       {
          _modifyInstance (cInst,
                           cNsInst,
                           path.cInst (),
                           modifiedInstance.cInst (),
                           includeQualifiers,
                           propertyList);
       }
    }

    public CIMValue getProperty (CIMObjectPath path,
                                 String        propertyName)
         throws CIMException
    {
       long ciValue = 0;

       if (cInst != 0)
       {
          ciValue = _getProperty (cInst,
                                  cNsInst,
                                  path.cInst (),
                                  propertyName);
       }

       if (ciValue != 0)
       {
          return new CIMValue (ciValue);
       }
       else
       {
          return null;
       }
    }

    public void setProperty (CIMObjectPath path,
                             String        propertyName,
                             CIMValue      newValue)
         throws CIMException
    {
       if (cInst != 0)
       {
          _setProperty (cInst, cNsInst, path.cInst (), propertyName, newValue.cInst ());
       }
    }

    public Enumeration execQuery (CIMObjectPath path,
                                  String        query,
                                  String        ql)
         throws CIMException
    {
       long ciEnumeration = 0;

       if (cInst != 0)
       {
          ciEnumeration = _execQuery (cInst,
                                      cNsInst,
                                      path.cInst (),
                                      query,
                                      ql);
       }

       if (ciEnumeration != 0)
       {
          return new InstEnumeration (ciEnumeration);
       }
       else
       {
          return null;
       }
    }

    public Enumeration associatorNames (CIMObjectPath path,
                                        String        assocClass,
                                        String        resultClass,
                                        String        role,
                                        String        resultRole)

         throws CIMException
    {
       long ciEnumeration = 0;

       if (cInst != 0)
       {
          ciEnumeration = _associatorNames (cInst,
                                            cNsInst,
                                            path.cInst (),
                                            assocClass,
                                            resultClass,
                                            role,
                                            resultRole);
       }

       if (ciEnumeration != 0)
       {
          return new PathEnumeration (ciEnumeration);
       }
       else
       {
          return null;
       }
    }

    public Enumeration associators (CIMObjectPath path,
                                    String        assocClass,
                                    String        resultClass,
                                    String        role,
                                    String        resultRole,
                                    boolean       includeQualifiers,
                                    boolean       includeClassOrigin,
                                    String        propertyList[])

         throws CIMException
    {
       long ciEnumeration = 0;

       if (cInst != 0)
       {
          ciEnumeration = _associators (cInst,
                                        cNsInst,
                                        path.cInst (),
                                        assocClass,
                                        resultClass,
                                        role,
                                        resultRole,
                                        includeQualifiers,
                                        includeClassOrigin,
                                        propertyList);
       }

       if (ciEnumeration != 0)
       {
          return new InstEnumeration (ciEnumeration);
       }
       else
       {
          return null;
       }
    }

    public Enumeration referenceNames (CIMObjectPath path,
                                       String        resultClass,
                                       String        role)

         throws CIMException
    {
       long ciEnumeration = 0;

       if (cInst != 0)
       {
          ciEnumeration = _referenceNames (cInst,
                                           cNsInst,
                                           path.cInst (),
                                           resultClass,
                                           role);
       }

       if (ciEnumeration != 0)
       {
          return new PathEnumeration (ciEnumeration);
       }
       else
       {
          return null;
       }
    }

    public Enumeration references (CIMObjectPath path,
                                   String        resultClass,
                                   String        role,
                                   boolean       includeQualifiers,
                                   boolean       includeClassOrigin,
                                   String        propertyList[])

         throws CIMException
    {
       long ciEnumeration = 0;

       if (cInst != 0)
       {
          ciEnumeration = _references (cInst,
                                       cNsInst,
                                       path.cInst (),
                                       resultClass,
                                       role,
                                       includeQualifiers,
                                       includeClassOrigin,
                                       propertyList);
       }

       if (ciEnumeration != 0)
       {
          return new InstEnumeration (ciEnumeration);
       }
       else
       {
          return null;
       }
    }
/*
    public CIMNameSpace getNameSpace ()
    {
    }

    public CIMFeatures getServerFeatures ()
         throws CIMException
    {
    }

    public void authenticate (String userName,
                              String password)
         throws CIMException
    {
    }
*/
    static {
        System.loadLibrary("JMPIProviderManager");
    }
}
