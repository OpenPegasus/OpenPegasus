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
 *  Original Author: Mike Day md@soft-hackle.net
 *                                mdd@us.ibm.com
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




#ifndef _LSLPDEFS_INCLUDE
#define _LSLPDEFS_INCLUDE
#ifdef __cplusplus
extern "C" {
#endif

#ifdef PEGASUS_OS_TYPE_WINDOWS
#pragma pack( push, lslp_defs )
#pragma pack(1)
#endif
#include "config.h"
#include "endian.h"



  /******************** circular list macros *********************/

  /*---------------------------------------------------------------------**
  ** structures used with these macros MUST have the following elements: **
  ** struct type-name {                                                  **
  **    struct type-name *next;                                          **
  **    struct type-name *prev;                                          **
  **    BOOL isHead;                                                     **
  **    int count;                                                       **
  ** }                                                                   **
  **---------------------------------------------------------------------*/

  /*  is node x the head of the list? */
  /* BOOL IS_HEAD(node *x); */
#define _LSLP_IS_HEAD(x) ((x)->isHead )

  /* where h is the head of the list */
  /* BOOL _LSLP_IS_EMPTY(head);          */
#define _LSLP_IS_EMPTY(h) \
    ((((h)->next == (h)) && ((h)->prev == (h)) ) ? TRUE : FALSE)

  /* where n is the new node, insert it immediately after node x 
  * x can be the head of the list                               
  * void _LSLP_INSERT(new, after);
  */

#define _LSLP_INSERT(n, x)   \
    {(n)->prev = (x); \
    (n)->next = (x)->next; \
    (x)->next->prev = (n); \
    (x)->next = (n); }

#define _LSLP_INSERT_AFTER _LSLP_INSERT
#define _LSLP_INSERT_BEFORE(n, x) \
    {(n)->next = (x); \
    (n)->prev = (x)->prev; \
    (x)->prev->next = (n); \
    (x)->prev = (n); }

#define _LSLP_INSERT_WORKNODE_LAST(n, x) \
        {gettimeofday(&((n)->timer));  \
    (n)->next = (x); \
    (n)->prev = (x)->prev; \
    (x)->prev->next = (n); \
    (x)->prev = (n); }

#define _LSLP_INSERT_WORKNODE_FIRST(n, x) \
        {gettimeofday(&((n)->timer));  \
    (n)->prev = (x); \
    (n)->next = (x)->next; \
    (x)->next->prev = (n); \
    (x)->next = (n); }

  /* delete node x  - harmless if list is empty */
  /* void _LSLP_DELETE_(x);                        */
#define _LSLP_UNLINK(x) \
    {(x)->prev->next = (x)->next; \
    (x)->next->prev = (x)->prev;}

  /* given the head of the list h, determine if node x is the last node */
  /* BOOL _LSLP_IS_LAST(head, x);                                           */
#define _LSLP_IS_LAST(h, x) \
        (((x)->next == (h) && (h)->prev == (x)) ? TRUE : FALSE)

  /* given the head of the list h, determine if node x is the first node */
  /* BOOL _LSLP_IS_FIRST(head, x);                                          */
#define _LSLP_IS_FIRST(h, x) \
    (((x)->prev == (h) && (h)->next == (x)) ? TRUE : FALSE)

  /* given the head of the list h, determine if node x is the only node */
  /* BOOL _LSLP_IS_ONLY(head, x);                                           */
#define _LSLP_IS_ONLY(h, x) \
    (((x)->next == (h) && (h)->prev == (x)) ? TRUE : FALSE)

  /* void _LSLP_LINK_HEAD(dest, src); */
  // src is no more the head
#define _LSLP_LINK_HEAD(d, s) \
    { \
        (d)->next = (s)->next; \
        (d)->prev = (s)->prev; \
        (s)->next->prev = (d); \
        (s)->prev->next = (d); \
        (s)->prev = (s)->next = (s) ; \
    }

  /************* bit-set macros *********************************/
  /* how many dwords do we need to allocate to hold b bits ? */
#define _LSLP_SIZEOF_BITARRAY(b) (((b) >> 5) + 1)
  /*  operating on an array of dwords */
#define _LSLP_IS_BIT_SET(b, a) (*((a) + ((b) >> 5)) & (1 << (((b)%32) - 1)))
#define _LSLP_SET_BIT(b, a) (*((a) + ((b) >> 5)) |= (1 << (((b)%32) - 1)))
#define _LSLP_CLEAR_BIT(b, a) (*((a) + ((b) >> 5)) ^= (1 << (((b)%32) - 1)))

  /* the usual */
#define _LSLP_MIN(a, b) ((a) < (b) ? (a) : (b))
#define _LSLP_MAX(a, b) ((a) > (b) ? (a) : (b))

  /********************** SLP V2 MACROS *************************/
  /**************************************************************/
#define LSLP_PROTO_VER 2
#define LSLP_PORT 427
#define _LSLP_LOCAL_BCAST inet_addr("255.255.255.255")
#define _LSLP_MCAST       inet_addr("239.255.255.253")
  /* derive an address for a directed broadcast */
  /* a = uint32 addr, m = uint32 mask */
#define _LSLP_DIR_BCAST(a, m) {(a) &= (m); (a) |= ~(m);}
//jeb #define LSLP_EN_US "EN"
#define LSLP_EN_US "en"
#define LSLP_EN_US_LEN 2
  /* the absolute minimum hdr size */
#define LSLP_MIN_HDR  14

  /* slp v2 message types */
#define LSLP_SRVRQST 1
#define LSLP_SRVRPLY 2
#define LSLP_SRVREG 3
#define LSLP_SRVDEREG 4
#define LSLP_SRVACK 5
#define LSLP_ATTRREQ 6
#define LSLP_ATTRRPLY 7
#define LSLP_DAADVERT 8
#define LSLP_SRVTYPERQST 9
#define LSLP_SRVTYPERPLY 10
#define LSLP_SAADVERT 11

  /* slp error codes */
#define LSLP_LANGUAGE_NOT_SUPPORTED 1
#define LSLP_PARSE_ERROR 2
#define LSLP_INVALID_REGISTRATION 3
#define LSLP_SCOPE_NOT_SUPPORTED 4
#define LSLP_AUTHENTICATION_UNKNOWN 5
#define LSLP_AUTHENTICATION_ABSENT 6
#define LSLP_AUTHENTICATION_FAILED 7
#define LSLP_VERSION_NOT_SUPPORTED 9
#define LSLP_INTERNAL_ERROR 10
#define LSLP_DA_BUSY 11
#define LSLP_OPTION_NOT_UNDERSTOOD 12
#define LSLP_INVALID_UPDATE 13
#define LSLP_MSG_NOT_SUPPORTED 14
#define LSLP_REFRESH_REJECTED 15

  /* stream  SLP error codes */
#define LSLP_OK 0x00000000
#define LSLPERR_BASE 0x00002000
#define LSLPERR_INVALID_SEM LSLPERR_BASE + 0x00000001
#define LSLP_NOT_INITIALIZED LSLPERR_BASE + 0x00000002
#define LSLP_TIMEOUT LSLPERR_BASE + 0x00000003
#define LSLP_MALLOC_ERROR LSLPERR_BASE + 0x00000004


  /* offsets into the SLPv2 header */
#define LSLP_VERSION 0
#define LSLP_FUNCTION 1
#define LSLP_LENGTH 2
#define LSLP_FLAGS 5
#define LSLP_NEXT_EX 7
#define LSLP_XID 10
#define LSLP_LAN_LEN 12
#define LSLP_LAN 14


  /* macros to gain access to SLP header fields */
  /* h = (uint8 *)slpHeader */
  /* o = int32 offset */
  /* i = int32 value */

#if __BYTE_ORDER__ == __LITTLE_ENDIAN__
#define _LSLP_GETBYTE(h, o)  (0x00ff  & *(uint8 *) &((h)[(o)]))
#define _LSLP_GETSHORT(h, o) ((0xff00 & _LSLP_GETBYTE((h), (o)) << 8) + \
    (0x00ff & _LSLP_GETBYTE((h), (o) + 1)) )
#define _LSLP_SETBYTE(h, i, o) ((h)[(o)] = (uint8)i)
#define _LSLP_SETSHORT(h, i, o) {_LSLP_SETBYTE((h),(0xff & ((i) >> 8)),(o)); \
    _LSLP_SETBYTE((h), (0xff & (i)), (o) + 1); }
#define _LSLP_GETLONG(h, o) ( (0xffff0000 & _LSLP_GETSHORT((h), (o)) << 16) +\
    (0x0000ffff & _LSLP_GETSHORT((h), (o) + 2)) )
#define _LSLP_SETLONG(h,i,o) {_LSLP_SETSHORT((h),(0xffff & ((i) >> 16)),(o)); \
    _LSLP_SETSHORT((h), (0xffff & (i)), (o) + 2); }
  /* length is a 3-byte value */
#define _LSLP_GETLENGTH(h) ((0xff0000&(*(uint8 *)&((h)[LSLP_LENGTH]) << 16)) +\
    (0x00ff00 & (*(uint8 *) &((h)[LSLP_LENGTH + 1]) << 8)) + \
    (0x0000ff & (*(uint8 *) &((h)[LSLP_LENGTH + 2]))))
#define _LSLP_SETLENGTH(h,i) {_LSLP_SETSHORT((h),(((i)&0xffff00) >> 8),\
    LSLP_LENGTH); \
    _LSLP_SETBYTE((h), ((i) & 0x0000ff), LSLP_LENGTH + 2);}
  /* next option offset is a 3-byte value */
#define _LSLP_GETNEXTEXT(h) ((0xff0000&(*(uint8 *)&((h)[LSLP_NEXT_EX])<<16))+\
    (0x00ff00 & (*(uint8 *) &((h)[LSLP_NEXT_EX + 1]) << 8)) + \
    (0x0000ff & (*(uint8 *) &((h)[LSLP_NEXT_EX + 2]))))
#define _LSLP_SETNEXTEXT(h,i) {_LSLP_SETSHORT((h),(((i)&0xffff00) >> 8), \
    LSLP_NEXT_EX ); \
    _LSLP_SETBYTE((h), ((i) & 0x0000ff), LSLP_NEXT_EX + 2);}
#define _LSLP_SET3BYTES(h,i,o) {_LSLP_SETSHORT((h),(((i)&0xffff00) >> 8),(o));\
    _LSLP_SETBYTE((h), ((i) & 0x0000ff), (o) + 2);}
#define _LSLP_GET3BYTES(h, o) ((0xff0000 & (*(uint8 *) &((h)[(o)]) << 16)) + \
    (0x00ff00 & (*(uint8 *) &((h)[(o) + 1]) << 8)) + \
    (0x0000ff & (*(uint8 *) &((h)[(o) + 2]))))

#else  /* BIG ENDIAN */

#define _LSLP_GETBYTE(h, o)  (0x00ff  & *((uint8 *) &((h)[(o)])))
#define _LSLP_GETSHORT(h, o) ((0xff00 & _LSLP_GETBYTE((h), (o))) + \
                               (0x00ff & _LSLP_GETBYTE((h), (o) + 1)))

#define _LSLP_SETBYTE(h, i, o) ((h)[(o)] = (uint8)(i))
#define _LSLP_SETSHORT(h, i, o) { _LSLP_SETBYTE((h), (0xff00 & (i)), (o)); \
    _LSLP_SETBYTE((h), (0x00ff & (i)), (o) + 1);}

#define _LSLP_GETLONG(h, o)   ((0xffff0000 & _LSLP_GETSHORT((h), (o))) + \
    (0x0000ffff & _LSLP_GETSHORT((h), (o) + 2)))

#define _LSLP_SETLONG(h, i, o) { _LSLP_SETSHORT((h), (0xffff0000 & (i)), (o));\
    _LSLP_SETSHORT((h), (0x0000ffff & (i)), (o) + 2); }

  /* length is a 3-byte value */

#define _LSLP_GET3BYTES(h,o) ((0xff0000 & (_LSLP_GETBYTE((h), (o)) << 16)) + \
    (0x00ff00 & (_LSLP_GETBYTE((h), ((o) + 1)) << 8)) + \
    (0x0000ff & (_LSLP_GETBYTE((h), ((o) + 2)))))

#define _LSLP_SET3BYTES(h,i,o) ((_LSLP_SETBYTE((h),(0xff&((uint8)((i)>>16))), \
    (o)) + \
    (_LSLP_SETBYTE((h), (0xff & ((uint8)((i) >> 8))), ((o) + 1))) + \
    (_LSLP_SETBYTE((h), (0xff & ((uint8)((i)))), ((o) + 2)))

#define _LSLP_GETLENGTH(h) _LSLP_GET3BYTES((h), LSLP_LENGTH)
#define _LSLP_SETLENGTH(h, i) _LSLP_SET3BYTES((h), (i), LSLP_LENGTH)

#define _LSLP_GETFIRSTEXT(h) _LSLP_GET3BYTES((h), LSLP_NEXT_EX)
#define _LSLP_SETFIRSTEXT(h, i) _LSLP_SET3BYTES((h), (i), LSLP_NEXT_EX)

#define _LSLP_GETNEXTEXT _LSLP_GET3BYTES
#define _LSLP_SETNEXTEXT _LSLP_SET3BYTES

#endif  /* ENDIAN definitions */

  /* macros to get and set header fields */
#define _LSLP_GETVERSION(h) _LSLP_GETBYTE((h), LSLP_VERSION)
#define _LSLP_SETVERSION(h, i) _LSLP_SETBYTE((h), (i), LSLP_VERSION)
#define _LSLP_GETFUNCTION(h) _LSLP_GETBYTE((h), LSLP_FUNCTION)
#define _LSLP_SETFUNCTION(h, i) _LSLP_SETBYTE((h), (i), LSLP_FUNCTION)
#define _LSLP_GETFLAGS(h) _LSLP_GETBYTE((h), LSLP_FLAGS)
#define _LSLP_SETFLAGS(h, i) _LSLP_SETBYTE((h), (i), LSLP_FLAGS)
#define _LSLP_GETLANLEN(h) _LSLP_GETSHORT((h), LSLP_LAN_LEN)
#define _LSLP_SETLANLEN(h, i) _LSLP_SETSHORT((h), (i), LSLP_LAN_LEN)
#define _LSLP_SETLAN(h, s, l) \
{ _LSLP_SETSHORT((h), (l), LSLP_LAN_LEN) ; \
  memcpy( &((h)[LSLP_LAN]), (s), (l)) ; }

/* #define _LSLP_SETLAN(h, s)
    {_LSLP_SETSHORT((h),(_LSLP_MIN( (strlen((s)) + 2), 17)), LSLP_LAN_LEN); \*/
/* memcpy(&(h)[LSLP_LAN],(s),(_LSLP_MIN( (strlen((s)) + 2) , 17)));}*/
#define _LSLP_GETXID(h) _LSLP_GETSHORT((h), LSLP_XID)
#define _LSLP_SETXID(h, i) _LSLP_SETSHORT((h), (i), LSLP_XID)
#define _LSLP_HDRLEN(h) (14 + _LSLP_GETLANLEN((h)))
  /* give (work *), how big is the msg header ? */
#define _LSLP_HDRLEN_WORK(work) (14 + (work)->hdr.langLen )
  /* given (work *), how big is the data field ? */
#define _LSLP_DATALEN_WORK(work) (((work)->hdr.len)-_LSLP_HDRLEN_WORK((work)))

#define LSLP_FLAGS_OVERFLOW 0x80
#define LSLP_FLAGS_FRESH    0x40
#define LSLP_FLAGS_MCAST    0x20

  /* URL entry definitions */
#define LSLP_URL_LIFE 1
#define LSLP_URL_LEN 3
#define _LSLP_GETURLLIFE(h) _LSLP_GETSHORT((h), LSLP_URL_LIFE)
#define _LSLP_SETURLLIFE(h, i) _LSLP_SETSHORT((h), (i), LSLP_URL_LIFE)
#define _LSLP_GETURLLEN(h) _LSLP_GETSHORT((h), LSLP_URL_LEN)
#define _LSLP_SETURLLEN(h, i) _LSLP_SETSHORT((h), (i), LSLP_URL_LEN)


  /* simplified API definitions */

#define LSLP_API_HDR_VERSION 1
#define LSLP_API_SIGNATURE "legato slp stream interface\0"
#define LSLP_API_SIGNATURE_SIZE 28
#define LSLP_API_HDR_SIZE 38

#define _LSLP_PERROR(a, b) fprintf(stderr, "%s: Error %s (%s).\n", (a), (b), \
    strerror(errno))

struct api_hdr {
    int32 version;
    char  signature[28];
    int16 msgLen; /* including header */
    int16 errCode;
    int16 function_id;
};

#define LSLP_API_ACK 0x2000 /* with err code in first three bits */
#define LSLP_SIMPLE_SRVREQ 0x1001
#define LSLP_SIMPLE_SRVRPLY 0x1002
#define LSLP_SIMPLE_SRVREG 0x1003
#define LSLP_SIMPLE_SRVDEREG 0x1004

#define _LSLP_FREE_DEINIT(a) if((a) != NULL) { free(a); a = NULL; }

#ifdef __cplusplus
}
#endif
#ifdef PEGASUS_OS_TYPE_WINDOWS
#pragma pack( pop, lslp_defs )
#endif
#endif /* _LSLPDEFS_INCLUDE */
