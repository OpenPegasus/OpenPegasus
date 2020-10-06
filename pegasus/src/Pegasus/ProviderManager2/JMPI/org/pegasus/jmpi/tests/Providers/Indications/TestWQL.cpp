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
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/WQL/WQLSelectStatement.h>
#include <Pegasus/WQL/WQLParser.h>

#include <iostream>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

int
main (int argc, char *argv[])
{
   CIMClient client;

   try
   {
      client.connectLocal ();
   }
   catch (const Exception &e)
   {
      cerr << "Caught: " << e.getMessage () << endl;
   }

   Array<CIMInstance> instances;

   try
   {
      instances = client.enumerateInstances(
                      CIMNamespaceName("root/cimv2"),
                      CIMName ("PG_UnixProcess"),
                      true,  // deepInheritance
                      false, // localOnly
                      false, // includeQualifiers
                      true); // includeClassOrigin

      cout << "instances.size () = " << instances.size () << endl;
   }
   catch (const Exception &e)
   {
      cerr << "Caught: " << e.getMessage () << endl;
   }

   String queryLanguage = "WQL";
   String query=
       "SELECT CSName, OSName FROM PG_UnixProcess WHERE Handle = \"2037\"";
   WQLSelectStatement *stmt = new WQLSelectStatement(queryLanguage, query);

   try
   {
      WQLParser::parse (query, *stmt);
   }
   catch (const Exception &e)
   {
      cerr << "Caught: " << e.getMessage () << endl;
   }

   cout << "stmt->getQueryLanguage () " << stmt->getQueryLanguage () << endl;
   cout << "stmt->getQuery () " << stmt->getQuery () << endl;
   cout << "stmt->getAllProperties () " << stmt->getAllProperties () << endl;

   // public String getSelectString ()

   CIMPropertyList cplSelect = stmt->getSelectPropertyList ();

   cout << "cplSelect.size () = " << cplSelect.size () << endl;

   for (Uint32 i = 0; i < cplSelect.size (); i++)
   {
      cout << "cplSelect[" << i << "] = " << cplSelect[i].getString () << endl;
   }

   // public SelectList getSelectList ()
   // public FromExp getFromClause ()
   // public QueryExp getWhereClause ()

   CIMPropertyList cplWhere = stmt->getWherePropertyList ();

   cout << "cplWhere.size () = " << cplWhere.size () << endl;

   for (Uint32 i = 0; i < cplWhere.size (); i++)
   {
      cout << "cplWhere[" << i << "] = " << cplWhere[i].getString () << endl;
   }

   for (Uint32 i = 0; i < instances.size (); i++)
   {
//////cout << "evaluate ("
//////     << instances[i].getPath ().toString ()
//////     << ") = "
//////     << stmt->evaluate (instances[i])
//////     << endl;

      if (stmt->evaluate (instances[i]))
      {
         CIMInstance selectedInstance = instances[i].clone ();

         stmt->applyProjection (selectedInstance, false);

         cout << selectedInstance.getPath ().toString () << endl;
      }
   }

   stmt->print ();

   delete stmt;

   client.disconnect ();

   return 0;
}
