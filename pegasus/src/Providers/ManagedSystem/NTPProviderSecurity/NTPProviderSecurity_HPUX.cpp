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

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
//Pegasus includes
#include "NTPProviderSecurity.h"

//------------------------------------------------------------------------------
PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

//==============================================================================
//
// Class [NTPProviderSecurity] methods
//
//==============================================================================

//------------------------------------------------------------------------------
// Constructor to set context
//------------------------------------------------------------------------------
NTPProviderSecurity::NTPProviderSecurity(const OperationContext & context)
{
    IdentityContainer container(context.get(IdentityContainer::NAME));
    secUsername.assign(container.getUserName());
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
NTPProviderSecurity::~NTPProviderSecurity()
{
}

//------------------------------------------------------------------------------
// FUNCTION: checkAccess
//
// REMARKS: Status of context user
//
// PARAMETERS:    [IN]  filename  -> file name to verify access
//                [IN]  chkoper   -> valid options: SEC_OPT_READ,
//                                                  SEC_OPT_WRITE,
//                                                  SEC_OPT_READ_WRITE or
//                                                  SEC_OPT_EXECUTE
//
// RETURN: TRUE, if user have privileges, otherwise FALSE
//------------------------------------------------------------------------------
Boolean NTPProviderSecurity::checkAccess(
    const String filename,
    const String chkoper)
{
    struct passwd *pwd;
    struct group *grp;
    struct stat st;
    int ps, opt;
    Boolean ok = false,
            isRoot = false,
            okUser = (secUsername.size() > 0);
    char *member;
    // Groups array
    Array<gid_t> grps;
    // store user id
    uid_t user_id = -1;
    // store group id  - is there only one group id?
    gid_t group_id;
    int accessrights;
    String strTmp;
    String path;
    String strValue;
    Array<String> strCmd;
    Array<String> strMembers;

    if (okUser)
    {
        // Retrieve uid from user
        strValue.clear();
        // Go through password entries and find the entry that matches
        // "secUsername"
        pwd = getpwent();
        if (pwd != NULL)
        {
            strValue.assign(pwd->pw_name);
            while(!String::equalNoCase(strValue, secUsername))
            {
                pwd = getpwent();
                if(pwd == NULL)
                    break;
                strValue.assign(pwd->pw_name);
            }
        }
        // indicate that the processing of the password database is complete
        endpwent();

        // If we didn't find the entry - just return
        if (strValue.size() == 0 || !String::equalNoCase(strValue, secUsername))
            return ok;

        // DLH set the group and user id
        user_id = pwd->pw_uid;
        group_id = pwd->pw_gid;
        grps.clear();
        isRoot = (user_id == 0);

        if (!isRoot)
        {
            grps.append(group_id);

            // Find the groups to which this user belongs and store the list
            // in "member"
            strValue.clear();
            // Return a pointer to the first group structure in the group
            // database
            grp = getgrent();
            while (grp)
            {
                int i = 0;
                strMembers.clear();
                member = grp->gr_mem[i++];
                while (member)
                {
                    strMembers.append(member);
                    member = grp->gr_mem[i++];
                }
                for (i=0; i < strMembers.size(); i++)
                {
                    strValue.assign(strMembers[i]);
                    ps = strValue.find(secUsername);
                    if (ps >= 0)
                    {
                        grps.append(grp->gr_gid);
                        break;
                    }
                }
                // Get the next group structure
                grp = getgrent();
            }
            // Indicate that the processing of the group database is complete
            endgrent();
        }
    }

    // Build the command with path of file
    strCmd.clear();
    ps = filename.reverseFind('/');
    if (ps > 0)
    {
        path.assign(filename.subString(0, ps));
        strCmd.append(path);
    }

    // Build the command to retrieve user informations
    strCmd.append(filename);

    //
    // Identify the type test
    //
    opt = 0;
    if (String::equalNoCase(chkoper, SEC_OPT_READ) ||
        String::equalNoCase(chkoper, SEC_OPT_READ_WRITE))
    {
        opt = 1;
    }
    else if (String::equalNoCase(chkoper, SEC_OPT_WRITE) ||
             String::equalNoCase(chkoper, SEC_OPT_READ_WRITE))
    {
        opt = 2;
    }
    else if (String::equalNoCase(chkoper, SEC_OPT_EXECUTE) ||
            String::equalNoCase(chkoper, SEC_OPT_ALL))
    {
        opt = 3;
    }

    // Verify permissions from directory and file name
    for (int i=0; i<strCmd.size(); i++)
    {
        ok = false;
        strTmp.assign(strCmd[i]);

        // The stat call gets information about the file access permissions
        if (stat(strTmp.getCString(), &st) == -1)
            return ok;

        // Return ok, if is invalid user_id and other permission or is root
        if (!okUser && st.st_basemode & 0x04 || isRoot)
            ok = true;
        else if (user_id > 0)
        {
            // Use getaccess to check permission instead of stat so that we
            // get consistent response from OS
            accessrights = getaccess(
                strTmp.getCString(),
                user_id,
                grps.size(),
                grps.getData(),
                (void *) 0,
                (void *) 0);
            if (accessrights == -1)
            {
                // if error - just return with ok set to false
                return ok;
            }

            // Verify status by type test
            switch (opt)
            {
                case 1:
                    ok = (accessrights & R_OK);
                    break;
                case 2:
                    ok = (accessrights & W_OK);
                    break;
                case 3:
                    ok = (accessrights & X_OK);
                    break;
                default:
                    break;
            }
        }
        if (!ok)
            break;
    }
    return ok;
}
