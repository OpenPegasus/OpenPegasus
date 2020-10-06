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
/*****************************************************************************
 *  Description:
 *
 *  Originated: December 20, 2001
 *    Original Author: Mike Day md@soft-hackle.net
 *                       mdday@us.ibm.com
 *
 *
 *  Copyright (c) 2001 - 2003  IBM
 *  Copyright (c) 2000 - 2003 Michael Day
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#ifndef SLP_CLIENT_INC
#define SLP_CLIENT_INC

#include <Pegasus/Common/Config.h>
#include "slp_utils.h"

#ifndef NUCLEUS
#include "lslp-common-defs.h"
#include "lslp.h"
#endif

#define DA_SRVTYPE "service:directory-agent"
#define DA_SRVTYPELEN 23
#define DA_SCOPE "DEFAULT"
#define DA_SCOPELEN 7

#define SA_SRVTYPE "service:service-agent"
#define SA_SRVTYPELEN 21
#define SA_SCOPE "DEFAULT"
#define SA_SCOPELEN 7

#define SERVICE_WBEM "service:wbem"

#define TYPE_UNKKNOWN 0
#define TYPE_DA_LIST 1
#define TYPE_RPLY_LIST 2
//#define LSLP_MTU ETHERNT_MTU //jeb
#undef LSLP_WAIT_OK
#define LSLP_WAIT_OK 0

// IPv6 multicast groups
#ifdef PEGASUS_ENABLE_IPV6

// SVRLOC multicast groups for LINK, ADMIN, SITE and ORGANIZATION local
// scopes. These groups are used to receive Service-Type and Attribute
// Request messages.

#define SLP_MC_LINK_SVRLOC "FF02::116"
#define SLP_MC_ADMIN_SVRLOC "FF04::116"
#define SLP_MC_SITE_SVRLOC "FF05::116"
#define SLP_MC_ORG_SVRLOC "FF08::116"

// SVRLOC-DA multicast groups for LINK, ADMIN, SITE and ORGANIZATION  local
//  scopes. These groups are used to receive DA advertisements.

#define SLP_MC_LINK_SVRLOC_DA "FF02::123"
#define SLP_MC_ADMIN_SVRLOC_DA "FF04::123"
#define SLP_MC_SITE_SVRLOC_DA "FF05::123"
#define SLP_MC_ORG_SVRLOC "FF08::116"

#endif

#define slp_safe_free(a)  if(a != NULL) {free(a);}

#ifdef SLP_LIB_IMPORT
# define SLP_STORAGE_DECL PEGASUS_IMPORT
#else
# define SLP_STORAGE_DECL PEGASUS_EXPORT
#endif

#ifdef    __cplusplus
extern "C"
{
#endif

    typedef struct lslp_auth_block
    {
        struct lslp_auth_block *next;
        struct lslp_auth_block *prev;
        BOOL isHead;
        uint16 descriptor;
        uint16 len;
        time_t timestamp;
        uint16 spiLen;
        char *spi;
        char *block;
    }lslpAuthBlock;

    typedef struct lslp_atom_list
    {
        struct lslp_atom_list *next;
        struct lslp_atom_list *prev;
        BOOL isHead;
        char *str;
        uint32 hash;
    }lslpAtomList;

    typedef struct lslp_atomized_url
    {
        struct lslp_atomized_url *next;
        struct lslp_atomized_url *prev;
        BOOL isHead;
        char *url;
        uint32 urlHash;
        lslpAtomList srvcs;
        lslpAtomList site;
        lslpAtomList path;
        lslpAtomList attrs;
    }lslpAtomizedURL;


    typedef struct lslp_url_entry
    {
        struct lslp_url_entry *next;
        struct lslp_url_entry *prev;
        BOOL isHead;
        time_t lifetime;
        uint16 len;
        char *url;
        uint8 auths;
        lslpAuthBlock *authBlocks;
        lslpAtomizedURL *atomized;
        lslpAtomList *attrs;
    }lslpURL ;

    typedef struct lslp_ext
    {
        uint16 id;
        uint32 nextOffset;
        void *data;
    }lslpExt;

    typedef enum attrTypes
    {
        head = -1,
        string,
        integer,
        bool_type,
        opaque,
        tag
    }lslpTypes;

    /* opaque vals should always have the first four bytes */
    /* contain the length of the value */
    typedef union lslp_attr_value
    {
        char *stringVal;
        uint32 intVal;
        int32 boolVal;
        void *opaqueVal;
    }lslpAttrVal;

    typedef struct lslp_attr_list
    {
        struct lslp_attr_list *next;
        struct lslp_attr_list *prev;
        BOOL isHead;
        int16 attr_string_len;
        char *attr_string;
        char *name;
        char type;
        int32 attr_len;
        lslpAttrVal val;
    }lslpAttrList;

    enum ldap_operator_types
    {
        ldap_and = 259,    /* to match token values assigned in y_filter.h */
        ldap_or,
        ldap_not,
        expr_eq,
        expr_gt,
        expr_lt,
        expr_present,
        expr_approx
    };

    struct ldap_filter_struct ;
    typedef struct ldap_filter_struct_head
    {
        struct ldap_filter_struct_head  *next;
        struct ldap_filter_struct_head  *prev;
        BOOL isHead;
        int _operator;
    }filterHead;

    typedef struct ldap_filter_struct
    {
        struct ldap_filter_struct *next;
        struct ldap_filter_struct *prev;
        BOOL isHead;
        int _operator;
        int nestingLevel;
        BOOL logical_value;
        filterHead children ;
        lslpAttrList attrs;
    }lslpLDAPFilter ;

    typedef struct lslp_scope_list
    {
        struct lslp_scope_list *next;
        struct lslp_scope_list *prev;
        BOOL isHead;
        BOOL isProtected;
        char *scope;
    }lslpScopeList;

    typedef lslpScopeList lslpSPIList;

    typedef struct lslp_srv_reg_instance
    {
        struct lslp_srv_reg_instance *next;
        struct lslp_srv_reg_instance *prev;
        BOOL isHead;
        lslpURL *url;   /* includes url auth block */
        char *srvType;
        lslpScopeList *scopeList;
        lslpAttrList *attrList;
        lslpAuthBlock *authList;  /* signature(s) for attribute list  */
        time_t directoryTime;
    }lslpSrvRegList;

    typedef struct lslp_srv_reg_head
    {
        struct lslp_srv_reg_instance *next;
        struct lslp_srv_reg_instance *prev;
        BOOL isHead;
    }lslpSrvRegHead;   /* holds the auth blocks for all attrs in the list */


    /******** internal slp message definitions ********/

    /* URL entry definitions */
#define LSLP_URL_LIFE    1
#define LSLP_URL_LEN     3
#define _LSLP_GETURLLIFE(h) _LSLP_GETSHORT((h), LSLP_URL_LIFE)
#define _LSLP_SETURLLIFE(h, i) _LSLP_SETSHORT((h), (i), LSLP_URL_LIFE)
#define _LSLP_GETURLLEN(h) _LSLP_GETSHORT((h), LSLP_URL_LEN)
#define _LSLP_SETURLLEN(h, i) _LSLP_SETSHORT((h), (i), LSLP_URL_LEN)

/*       0                   1                   2                   3 */
/*       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |       Service Location header (function = SrvRqst = 1)        | */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |      length of <PRList>       |        <PRList> String        \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |   length of <service-type>    |    <service-type> String      \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |    length of <scope-list>     |     <scope-list> String       \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |  length of predicate string   |  Service Request <predicate>  \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |  length of <SLP SPI> string   |       <SLP SPI> String        \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */

    /* ----- service request ------ */
    struct lslp_srv_req
    {
        uint16 prListLen;
        char *prList;
        uint16 srvcTypeLen;
        char *srvcType;
        lslpScopeList *scopeList;
        uint16 predicateLen;
        char *predicate;
        lslpSPIList *spiList;
        char *next_ext;
        char *ext_limit;
    };

    /* ----- service reply ------ */
    struct lslp_srv_rply
    {
        uint16 errCode;
        uint16 urlCount;
        uint16 urlLen;
        lslpURL *urlList;
        lslpAtomList *attr_list;
    };


/*    SLP messages all begin with the following header: */

/*       0                   1                   2                   3 */
/*       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |    Version    |  Function-ID  |            Length             | */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      | Length, contd.|O|F|R|       reserved          |Next Ext Offset| */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |  Next Extension Offset, contd.|              XID              | */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |      Language Tag Length      |         Language Tag          \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */

/* 4.3. URL Entries */

/*       0                   1                   2                   3 */
/*       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |   Reserved    |          Lifetime             |   URL Length  | */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |URL len, contd.|            URL (variable length)              \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |# of URL auths |            Auth. blocks (if any)              \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */



/* 8.2. Service Reply */

/*       0                   1                   2                   3 */
/*       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |        Service Location header (function = SrvRply = 2)       | */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |        Error Code             |        URL Entry count        | */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |       <URL Entry 1>          ...       <URL Entry N>          \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */

    struct lslp_srv_rply_out
    {
        uint16 errCode;
        uint16 urlCount;
        uint16 urlLen;
        char *urlList;
    };


/*    The format of the Attribute List Extension is as follows: */

/*        0                   1                   2                   3 */
/*        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 */
/*       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*       |      Extension ID = 0x0002    |     Next Extension Offset     | */
/*       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*       | Offset, contd.|      Service URL Length       |  Service URL  / */
/*       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*       |     Attribute List Length     |         Attribute List        / */
/*       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*       |# of AttrAuths |(if present) Attribute Authentication Blocks.../ */
/*       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */


/*       0                   1                   2                   3 */
/*       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |         Service Location header (function = SrvReg = 3)       | */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |                          <URL-Entry>                          \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      | length of service type string |        <service-type>         \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |     length of <scope-list>    |         <scope-list>          \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |  length of attr-list string   |          <attr-list>          \ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/*      |# of AttrAuths |(if present) Attribute Authentication Blocks...\ */
/*      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */


    /* ----- service registration ------ */
    struct lslp_srv_reg
    {
        lslpURL *url;                 /* includes url auth block */
        uint16 srvTypeLen;
        char *srvType;
        lslpScopeList *scopeList;
        lslpAttrList *attrList;
        lslpAuthBlock *attrAuthList;
    };

    /* ----- service acknowledgement ----- */
    struct lslp_srv_ack
    {
        uint16 errCode;
    };

    /* ----- da advertisement ----- */
    struct lslp_da_advert
    {
        uint16 errCode;
        uint32 statelessBootTimestamp;
        uint16 urlLen;
        char *url; /* not a url-entry - merely a url string */
        uint16 scopeListLen;
        char *scopeList;
        uint16 attrListLen;
        char *attrList;
        uint16 spiListLen;
        char *spiList;  /* comma separated values - see 6.4.1*/
        uint8 auths;
        lslpAuthBlock *authBlocks;
    };

    /* -----sa advertisement ----- */
    struct lslp_sa_advert
    {
        uint16 urlLen;
        char *url; /* not a url-entry - merely a url string */
        uint16 scopeListLen;
        char *scopeList;
        uint16 attrListLen;
        char *attrList;
        uint8 auths;
        lslpAuthBlock *authBlocks;
    };

    /* ----- srv type request  ----- */
    struct lslp_srvtype_req
    {
        uint16 prListLen;
        char *prList;
        uint16 nameAuthLen;
        char *nameAuth;
        lslpScopeList *scopeList;
    };

    /* ----- srv type reply ----- */
    struct lslp_srvtype_rep
    {
        uint16 errCode;
        uint16 srvTypeListLen;
        /* see 4.1 for srvtype-string; this is a string-list */
        char *srvTypeList;
    };

    /* ----- attr request ----- */
    struct lslp_attr_req
    {
        uint16 prListLen;
        char *prList;
        uint16 urlLen;
        char *url; /* not a url-entry - merely a url string */
        lslpScopeList *scopeList;
        uint16 tagListLen;
        char *tagList;
        lslpSPIList *spiList;
    };

    /* ----- attr reply ----- */
    struct lslp_attr_rep
    {
        uint16 errCode;
        uint16 attrListLen;
        char *attrList;
        uint8 auths;
        lslpAuthBlock *authBlocks;
    };

    /* ----- srv deregistration ----- */
    struct lslp_srv_dereg
    {
        uint16 scopeListLen;
        char *scopeList;
        lslpURL *urlList;
        uint16 tagListLen;
        char *tagList;
    };

    /* general internal representation of all slp msgs */
    union lslp_msg_u
    {
        struct lslp_srv_req srvReq;
        struct lslp_srv_rply srvRply;
        struct lslp_srv_reg    srvReg;
        struct lslp_srv_ack srvAck;
        struct lslp_da_advert daAdvert;
        struct lslp_sa_advert saAdvert;
        struct lslp_srvtype_req srvTypeReq;
        struct lslp_srvtype_rep srvTypeRep;
        struct lslp_attr_req attrReq;
        struct lslp_attr_rep attrRep;
        struct lslp_srv_dereg srvDereg;
    };

    enum msg_types
    {
        srvReq = 1,
        srvRply,
        srvReg,
        srvDereg,
        srvAck,
        attrReq,
        attrRep,
        daAdvert,
        srvTypeReq,
        srvTypeRep,
        saAdvert
    };

    typedef enum msg_types msgTypes;

    typedef struct lslp_hdr
    {
        uint8 ver;
        uint8 msgid;
        uint32 len;
        uint16 flags;
        uint32 nextExt;
        uint16 xid;
        uint32 errCode;
        uint16 langLen;
        uint8 lang[19];
        void *data;
    }lslpHdr;

    typedef struct lslp_msg
    {
        struct lslp_msg *next;
        struct lslp_msg *prev;
        BOOL isHead;
        msgTypes type;
        lslpHdr hdr;
        char dynamic;
        union lslp_msg_u msg;
    }lslpMsg;

    struct da_list
    {
        struct da_list *next;
        struct da_list *prev;
        BOOL isHead;
        char function;
        uint16 err;
        uint32 stateless_boot;
        char *url;
        char *scope;
        char *attr;
        char *spi;
        char auth_blocks;
        char *auth;
#ifdef PEGASUS_ENABLE_IPV6
        char remote[PEGASUS_INET6_ADDRSTR_LEN];
#else
        char remote[PEGASUS_INET_ADDRSTR_LEN];
#endif
    };

    struct rply_list
    {
        struct rply_list *next;
        struct rply_list *prev;
        BOOL isHead;
        char function;
        uint16 err;
        uint16 lifetime;
        char *url;
        char auth_blocks;
        char *auth;
#ifdef PEGASUS_ENABLE_IPV6
        char remote[PEGASUS_INET6_ADDRSTR_LEN];
#else
        char remote[PEGASUS_INET_ADDRSTR_LEN];
#endif
    };

    struct reg_list
    {
        struct reg_list *next;
        struct reg_list *prev;
        BOOL isHead;
        char *url;
        char *attributes;
        char *service_type;
        char *scopes;
        time_t lifetime;
    };

    struct url_entry
    {
        struct url_entry *next;
        struct url_entry *prev;
        BOOL isHead;
        uint16 lifetime;
        int16 len;
        char *url;
        uint8 num_auths;
        char *auth_blocks;
    };

    /* Represents Interface address either IPv4 or IPv6 */
    struct slp_if_addr
    {
        uint16 af; // Address family
        union
        {
            struct in_addr ip4_addr;
#ifdef PEGASUS_ENABLE_IPV6
            struct in6_addr ip6_addr;
#endif
        };
    };

    struct slp_client
    {
        uint16 _pr_buf_len;
        uint16 _buf_len;
        char _version;
        uint16 _xid;
        uint16 _target_port;

        struct slp_if_addr _target_addr;
        struct slp_if_addr _local_addr;
        struct slp_if_addr *_local_addr_list[2];

        BOOL _ip4_stack_active;
        BOOL _ip6_stack_active;
        BOOL _local_addr_any;
        BOOL _target_addr_any;

        uint32 _msg_counts[12];
        lslpSPIList *_spi;
        lslpScopeList *_scopes;

        char* _pr_buf;
        char* _msg_buf;
        char* _rcv_buf;
        char* _scratch;
        char* _err_buf;
        char* _srv_type;

        BOOL _use_das;
        uint16 _da_target_port;
        struct slp_if_addr _da_target_addr;

        /* add field and record separators for shell scripting */
        BOOL _use_separators;
        char _fs;
        char _rs;

        time_t _last_da_cycle;
        struct timeval _tv;
        int _retries;
        int _ttl;
        int  _convergence;
        void *_crypto_context;

        SOCKETD _rcv_sock[2];

#if defined(PEGASUS_OS_TYPE_WINDOWS)
        int _winsock_count ;
        WSADATA _wsa_data;
#endif
        struct da_list das;
        lslpMsg replies;
        lslpSrvRegHead *regs;

        void (*slp_open_listen_socks)(struct slp_client *client);
        lslpMsg *(*get_response) (struct slp_client *, lslpMsg *);
        int (*find_das)(
            struct slp_client *,
            const char *,
            const char *);
        void (*converge_srv_req)(
            struct slp_client *,
            const char *,
            const char *,
            const char *);
        void (*unicast_srv_req)(
            struct slp_client *,
            const char *,
            const char *,
            const char *,
            SOCKADDR *);
        void (*local_srv_req)(
            struct slp_client *,
            const char *,
            const char *,
            const char *);
        void (*srv_req)(
            struct slp_client *,
            const char *,
            const char *,
            const char *,
            BOOL);
        /** <<< Sat Jul 24 14:56:59 2004 mdd >>> add attr request **/

        void (*converge_attr_req)(
            struct slp_client *,
            const char *,
            const char *,
            const char * );

        void (*unicast_attr_req)(
            struct slp_client *,
            const char *,
            const char *,
            const char *,
            SOCKADDR *);

        void (*local_attr_req)(
            struct slp_client *,
            const char *,
            const char *,
            const char * );

        void (*attr_req)(
            struct slp_client *,
            const char *,
            const char *,
            const char *,
            BOOL);

        void (*decode_attr_rply)( struct slp_client * );
        /** <<< Sat Jul 24 15:10:07 2004 mdd >>>  end **/

        BOOL (*srv_reg)(
            struct slp_client *,
            const char *,
            const char *,
            const char *,
            const char *,
            int16 );
        int (*srv_reg_all)(
            struct slp_client *,
            const char *,
            const char *,
            const char *,
            const char *,
            int16 lifetime );
        int (*srv_reg_local) (
            struct slp_client *,
            const char *,
            const char *,
            const char *,
            const char *,
            uint16 lifetime ); //jeb int16 to uint16

        int32 (*service_listener)(
            struct slp_client *,
            SOCKETD,
            lslpMsg *);  //jeb

        void (*prepare_pr_buf)(
            struct slp_client *,
            const char *);

        BOOL (*prepare_query)(
            struct slp_client *,
            uint16,
            const char *,
            const char *,
            const char *);

        void (*decode_msg)(struct slp_client *client, SOCKADDR *remote);

        void (*decode_srvreq)(struct slp_client *, SOCKADDR *);

        void (*decode_srvrply)(struct slp_client *);

        void (*decode_daadvert)( struct slp_client *, SOCKADDR *);

        void (*decode_attrreq)(struct slp_client *, SOCKADDR *);

        BOOL (*send_rcv_udp)(struct slp_client *) ;

        int32 (*service_listener_wait)(
            struct slp_client *,
            time_t,
            SOCKETD extra,
            BOOL,
            lslpMsg *);

        BOOL (*slp_previous_responder)(struct slp_client *, char *, int);
    };


/*----------------------------- prototypes ----------------------------------*/

#define LSLP_DESTRUCTOR_DYNAMIC 1
#define LSLP_DESTRUCTOR_STATIC  0
    struct da_list *alloc_da_node(BOOL head);
    struct da_list *da_node_exists(struct da_list *head, const void *key);
    void free_da_list_members(struct da_list *da);
    void free_da_list_node(struct da_list *da);

    void slp_get_local_interfaces(struct slp_client *client);
    BOOL slp_join_multicast(SOCKETD sock, struct slp_if_addr addr) ;
    int slp_join_multicast_all(SOCKETD sock, int af);
    void slp_open_listen_sock(struct slp_client *client);
    void make_srv_ack(
        struct slp_client *client,
        SOCKADDR *remote,
        char response,
        int16 code);
    void prepare_pr_buf(struct slp_client *client, const char *address);
    BOOL prepare_query(
        struct slp_client *client,
        uint16 xid,
        const char *service_type,
        const char *scopes,
        const char *predicate);
    /*
     * Get the responses hold by the client. This also changes the head of
     * list to head from the client->replies
     * @param client slp_client for which we will get the responses
     * @param head store/exchange the head of the list for the replies to head
     */
    lslpMsg *get_response(struct slp_client *client , lslpMsg *head);

    void converge_srv_req(
        struct slp_client *client,
        const char *type,
        const char *predicate,
        const char *scopes);

    void unicast_srv_req(
        struct slp_client *client,
        const char *type,
        const char *predicate,
        const char *scopes,
        SOCKADDR *addr );
    void local_srv_req(
        struct slp_client *client,
        const char *type,
        const char *predicate,
        const char *scopes );

    void srv_req(
        struct slp_client *client,
        const char *type,
        const char *predicate,
        const char *scopes,
        BOOL retry );


    /** <<< Sat Jul 24 14:56:59 2004 mdd >>> add attr request **/

    void converge_attr_req(
        struct slp_client *client,
        const char *url,
        const char *scopes,
        const char *tags);

    void unicast_attr_req(
        struct slp_client *client,
        const char *url,
        const char *scopes,
        const char *tags,
        SOCKADDR *addr);

    void local_attr_req(
        struct slp_client *client,
        const char *url,
        const char *scopes,
        const char *tags );

    void attr_req(
        struct slp_client *client,
        const char *url,
        const char *scopes,
        const char *tags,
        BOOL retry);

    void decode_attr_rply( struct slp_client *client, SOCKADDR *remote);
    /** <<< Sat Jul 24 15:10:07 2004 mdd >>>  end **/

    void decode_srvreg(struct slp_client *client, SOCKADDR *remote);

    void decode_msg(struct slp_client *client, SOCKADDR *remote);
    void decode_srvrply( struct slp_client *client);
    void decode_attrreq(struct slp_client *client, SOCKADDR *remote);
    void decode_daadvert(struct slp_client *client, SOCKADDR *remote);
    void decode_srvreq(struct slp_client *client, SOCKADDR *remote);

    BOOL srv_reg(
        struct slp_client *client,
        const char *url,
        const char *attributes,
        const char *service_type,
        const char *scopes,
        int16 lifetime) ;

    BOOL send_rcv_udp(struct slp_client *client);

    int32 __service_listener(
        struct slp_client *client,
        SOCKETD extra_sock);  //jeb

    int32 service_listener(
        struct slp_client *client,
        SOCKETD extra_sock,
        lslpMsg *);                //jeb

    int srv_reg_all(
        struct slp_client *client,
        const char *url,
        const char *attributes,
        const char *service_type,
        const char *scopes,
        int16 lifetime);

    int srv_reg_local(
        struct slp_client *client,
        const char *url,
        const char *attributes,
        const char *service_type,
        const char *scopes,
        uint16 lifetime);   //jeb int16 to uint16

    void __srv_reg_local(
        struct slp_client *client,
        const char *url,
        const char *attributes,
        const char *service_type,
        const char *scopes,
        uint16 lifetime);   //jeb int16 to uint16

    BOOL slp_previous_responder(struct slp_client *client, char *pr_list,
        int af);



    /* attribute and attribute parser */
    lslpAttrList *_lslpDecodeAttrString(char *s);

    lslpAttrList *lslpAllocAttr(
        const char *name,
        char type,
        const void *val,
        int16 len);

    lslpAttrList *lslpAllocAttrList();
    void lslpFreeAttr(lslpAttrList *attr);
    void lslpFreeAttrList(lslpAttrList *list, BOOL staticFlag);

    BOOL lslpStuffAttrList(
        char **buf,
        int16 *len,
        lslpAttrList *list,
        lslpAttrList *include);

    lslpAttrList *lslpUnstuffAttr(char **buf, int16 *len, int16 *err) ;


    /* url and url parser */
    lslpURL *lslpAllocURL();
    lslpURL *lslpAllocURLList();
    void lslpFreeURL(lslpURL *url);
    void lslpFreeURLList(lslpURL *list);
    BOOL  lslpStuffURL(char **buf, int16 *len, lslpURL *url) ;
    BOOL lslpStuffURLList(char **buf, int16 *len, lslpURL *list) ;
    lslpURL *lslpUnstuffURL(char **buf, int16 *len, int16 *err) ;


    lslpAtomList *lslpAllocAtom();
    lslpAtomList *lslpAllocAtomList();
    void lslpFreeAtom(lslpAtomList *l);
    void lslpFreeAtomList(lslpAtomList *l, int32 flag);
    lslpAtomizedURL *lslpAllocAtomizedURL();
    lslpAtomizedURL *lslpAllocAtomizedURLList();
    void lslpFreeAtomizedURL(lslpAtomizedURL *u);
    void lslpFreeAtomizedURLList(lslpAtomizedURL *l, int32 flag);
    void lslpInitAtomLists();
    void lslpInitURLList();

    lslpAtomizedURL *_lslpDecodeURLs(char *u[], int32 count);


    /* ldap filter and parser */

    lslpLDAPFilter *lslpAllocFilter(int _operator);
    void lslpFreeFilter(lslpLDAPFilter *filter);
    void lslpFreeFilterList(lslpLDAPFilter *head, BOOL static_flag);
    void lslpInitFilterList();
    void lslpCleanUpFilterList();
    void lslpFreeFilterTree(lslpLDAPFilter *root);


    lslpLDAPFilter *_lslpDecodeLDAPFilter(char *filter) ;
    BOOL lslpEvaluateOperation(int compare_result, int operation);
    BOOL lslpEvaluateAttributes(
        const lslpAttrList *a,
        const lslpAttrList *b,
        int op);
    BOOL lslpEvaluateFilterTree(
        lslpLDAPFilter *filter,
        const lslpAttrList *attrs);

    /* scope lists */
    lslpScopeList *lslpAllocScope();
    lslpScopeList *lslpAllocScopeList();
    void lslpFreeScope(lslpScopeList *s);
    void lslpFreeScopeList(lslpScopeList *head);

#define lslpFreeSPI(a) lslpFreeScope((lslpScopeList *)(a))
#define lslpFreeSPIList(a) lslpFreeScopeList((lslpScopeList *)(a))


    BOOL lslpStuffScopeList(char **buf, int16 *len, lslpScopeList *list);
    lslpScopeList *lslpUnstuffScopeList(char **buf, int16 *len, int16 *err);
    lslpSPIList *lslpUnstuffSPIList(char **buf, int16 *len, int16 *err);
    BOOL lslpStuffSPIList(char **buf, int16 *len, lslpSPIList *list);

    lslpScopeList *lslpScopeStringToList(const char *s, int16 len) ;

    char *lslp_foldString(char *s);

    BOOL lslp_scope_intersection(lslpScopeList *a, lslpScopeList *b) ;
    /* caseless compare that folds whitespace */
    int lslp_string_compare(char *s1, char *s2);
    int lslp_isscope(int c) ;
    BOOL lslp_islegal_scope(char *s);

    /* service registrations */
    lslpSrvRegList *lslpAllocSrvReg();
    lslpSrvRegHead *lslpAllocSrvRegList();
    void lslpFreeSrvReg(lslpSrvRegList *s);
    void lslpFreeSrvRegList(lslpSrvRegHead *head);

    /* authorization blocks */

    lslpAuthBlock *lslpAllocAuthBlock();
    lslpAuthBlock *lslpAllocAuthList();
    void lslpFreeAuthBlock(lslpAuthBlock *auth);
    void lslpFreeAuthList(lslpAuthBlock *list);
    BOOL lslpStuffAuthList(char **buf, int16 *len, lslpAuthBlock *list);
    lslpAuthBlock *lslpUnstuffAuthList(char **buf, int16 *len, int16 *err);

    uint32 lslpCheckSum(char *s, int16 l);
    lslpHdr * lslpAllocHdr();
    void lslpFreeHdr(lslpHdr *hdr);
    void lslpDestroySrvReq(struct lslp_srv_req *r, char flag);
    void lslpDestroySrvRply(struct lslp_srv_rply *r, char flag) ;
    void lslpDestroySrvReg(struct lslp_srv_reg *r, char flag);
    void lslpDestroySrvAck(struct lslp_srv_ack *r, char flag);
    void lslpDestroyDAAdvert(struct lslp_da_advert *r, char flag);
    void lslpDestroySAAdvert(struct lslp_sa_advert *r, char flag);
    void lslpDestroySrvTypeReq(struct lslp_srvtype_req *r, char flag);
    void lslpDestroySrvTypeReply(struct lslp_srvtype_rep *r, char flag);
    void lslpDestroyAttrReq(struct lslp_attr_req *r, char flag);
    void lslpDestroyAttrReply(struct lslp_attr_rep *r, char flag);
    void lslpDestroySrvDeReg(struct lslp_srv_dereg *r, char flag);

    BOOL check_duplicate_resp(struct slp_client *client, lslpMsg *msg);
    struct lslp_srv_rply_out *_lslpProcessSrvReq(
        struct slp_client *client,
        struct lslp_srv_req *msg,
        int16 errCode,
        SOCKADDR *remote);
    /* a is an attribute list, while b is a string representation
       of an ldap filter  */
    BOOL lslp_predicate_match(lslpAttrList *a, char *b);

    char * lslp_get_next_ext(char *hdr_buf);


    /***** Functions Exported by the library *****/

    SLP_STORAGE_DECL int find_das(
        struct slp_client *client,
        const char *predicate,
        const char *scopes);

    SLP_STORAGE_DECL BOOL lslp_pattern_match(
        const char *s,
        const char *p,
        BOOL case_sensitive);

    SLP_STORAGE_DECL BOOL lslp_pattern_match2(
        const char *s,
        const char *p,
        BOOL case_sensitive);

    SLP_STORAGE_DECL struct slp_client *create_slp_client(
        const char *target_addr,
        const char *local_interface,
        uint16 target_port,
        const char *spi,
        const char *scopes,
        BOOL should_listen,
        BOOL use_das,
        const char *srv_type);

    SLP_STORAGE_DECL void destroy_slp_client(struct slp_client *client);
    SLP_STORAGE_DECL char *encode_opaque(void *buffer, int16 length);
    SLP_STORAGE_DECL void *decode_opaque(char *buffer);
    SLP_STORAGE_DECL lslpMsg *alloc_slp_msg(BOOL head);
    SLP_STORAGE_DECL void lslpDestroySLPMsg(lslpMsg *msg);
    SLP_STORAGE_DECL void lslp_print_srv_rply(lslpMsg *srvrply);
    SLP_STORAGE_DECL void lslp_print_srv_rply_parse(
        lslpMsg *srvrply,
        char fs,
        char rs);

    SLP_STORAGE_DECL void lslp_print_attr_rply(lslpMsg *attrrply);
    SLP_STORAGE_DECL void lslp_print_attr_rply_parse(
        lslpMsg *attrrply,
        char fs,
        char rs);

/** test functions - use these to test the correctness of
    slp strings. They will use the actual parsers to
    try and parse the strings. The return code indicates
    if the string is grammatically correct or not.
**/
    SLP_STORAGE_DECL BOOL test_service_type(char *type);
    SLP_STORAGE_DECL BOOL test_service_type_reg(char *type);
    SLP_STORAGE_DECL BOOL test_url(char *url);
    SLP_STORAGE_DECL BOOL test_attribute(char *attr);
    SLP_STORAGE_DECL BOOL test_scopes(char *scopes);
    SLP_STORAGE_DECL BOOL test_predicate(char *predicate);


/*****************************************************************
 *  test_srv_reg
 *
 *  test the correctness of a service registration -
 *  return codes:
 *     0 == SUCCESS
 *     1 == type string failed parsing
 *     2 == url string failed parsing
 *     3 == attribute string failed parsing
 *     4 == scope string failed parsing
 *
 *****************************************************************/
    SLP_STORAGE_DECL uint32 test_srv_reg(
        char *type,
        char *url,
        char *attr,
        char *scopes);

/*****************************************************************
 * test_query
 *
 * test the correctness of a service request
 *
 * return codes:
 *    0 == SUCCESS
 *    1 == type string failed parsing
 *    2 == predicate string failed parsing
 *    3 == scope string failed parsing
 *****************************************************************/

    SLP_STORAGE_DECL uint32 test_query(
        char *type,
        char *predicate,
        char *scopes);

#ifdef __cplusplus
}
#endif

#endif /* SLP_CLIENT_INC */
