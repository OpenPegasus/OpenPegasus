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

#include "cwsutil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

# define MAX_PATH_LEN 4096
# define MAX_FILENAME_LEN 256

#if defined SIMULATED
typedef struct {
  int next;
  int done;
  char type;
} CWS_Control;

#include "cwssimdata.c"

#if defined(PEGASUS_PLATFORM_WIN64_IA64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN64_X86_64_MSVC) || \
    defined(PEGASUS_PLATFORM_WIN32_IX86_MSVC)
char * dirname(char *path) {
        char drive[_MAX_DRIVE];
        char *dir = (char *)malloc(_MAX_DIR*sizeof(char));
        char fname[_MAX_FNAME];
        char ext[_MAX_EXT];

        _splitpath( path, drive, dir, fname, ext);
        return dir;
}
#endif

int locateFile(const char *fn)
{
   int i;
   for (i=0; files[i].name; i++)
      if (strcmp(files[i].name,fn)==0) return i;
   return -1;
}

void* CWS_Begin_Enum(const char *topdir, int filetype)
{
  /* begin enumeration */
  CWS_Control *cc = (CWS_Control *)malloc(sizeof(CWS_Control));
  cc->type=filetype;
  cc->next=-1;
  cc->done=0;
  return cc;
}

int CWS_Next_Enum(void *handle, CWS_FILE* cwsf)
{
  CWS_Control *cc = (CWS_Control *)handle;
  int n;

  if (cc->done) return 0;
  n=++cc->next;
  while (files[n].name && files[n].ftype!=cc->type) n=++cc->next;
  if (files[n].name) {
    strcpy(cwsf->cws_name,files[n].name);
    cwsf->cws_size=files[n].size;
    cwsf->cws_ctime=files[n].cTime;
    cwsf->cws_mtime=files[n].mTime;
    cwsf->cws_atime=files[n].aTime;
    cwsf->cws_mode=files[n].mode;
    return 1;
  }
  else cc->done=0;
  return 0;
}

void CWS_End_Enum(void *handle)
{
  /* end enumeration */
  CWS_Control *cc = (CWS_Control *)handle;
  if (cc) {
    free(cc);
  }
}

int CWS_Get_File(const char *file, CWS_FILE* cwsf)
{
  int n;
  if (file && cwsf && (n=locateFile(file))>=0) {
    strcpy(cwsf->cws_name,files[n].name);
    cwsf->cws_size=files[n].size;
    cwsf->cws_ctime=files[n].cTime;
    cwsf->cws_mtime=files[n].mTime;
    cwsf->cws_atime=files[n].aTime;
    cwsf->cws_mode=files[n].mode;
    return 1;
  }
  return 0;
}

int CWS_Update_File(CWS_FILE* cwsf)
{
  int mode;
  int n;

  /* only change mode */
  if (cwsf && (n=locateFile(cwsf->cws_name))>=0) {
    mode=(files[n].mode & 07077) | (cwsf->cws_mode & 0700);
    files[n].mode=mode;
    return 1;
  }
  return 0;
}

int CWS_Update_FileSize(CWS_FILE* cwsf, const char *fn)
{
    int n;

    /* only change filesize */
    if (cwsf && (n=locateFile(fn))>=0)
    {
        files[n].size=(long)cwsf->cws_size;
        return 1;
    }
    return 0;
}

int CWS_Create_Directory(CWS_FILE* cwsf)
{
  return 0;
/*  int         state=0;
  int         mode;

  if (cwsf) {
    mode=cwsf->cws_mode | 0077; // plus umask
    state=mkdir(cwsf->cws_name,mode)==0;
  }
  return state; */
}

int CWS_Get_FileType(const char *file, char* typestring, size_t tslen)
{
  int n;
  if (file && (n=locateFile(file))>=0) {
     strncpy(typestring,files[n].type,tslen);
     return 0;
  }
  return 1;
}

#else

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

typedef struct {
  FILE *fp;
  char name[L_tmpnam];
} CWS_Control;


void * CWS_Begin_Enum(const char *topdir, int filetype)
{
  int processDir;
  CWS_Control *cc = (CWS_Control*) malloc(sizeof(CWS_Control));
  if (cc && tmpnam(cc->name)) {
  cc->fp = fopen(cc->name,"w");
    processDir = openDirectory((char*)topdir,cc->fp,filetype);
    fclose(cc->fp);
    cc->fp = fopen(cc->name,"r");
   }
   if (processDir != 0)
   {
      free(cc);
      cc = NULL;
   }   
   return cc;

}

int openDirectory(char* topDir,FILE * fp,char fileType)
{
    
    struct dirent dirEntry;
    struct dirent *dirResult;
    DIR* dirPoint;
    struct stat fileDetails;
    char path[MAX_PATH_LEN],fileName[MAX_FILENAME_LEN];
    if ((dirPoint = opendir(topDir)) != NULL)
    {
       
       while ((readdir_r(dirPoint,&dirEntry,&dirResult) == 0) && dirResult )
       {
          strcpy(fileName,dirEntry.d_name);
          strcpy(path,topDir);
          if (topDir[ strlen(topDir) - 1 ] != '/')
          {
             strcat(path,"/");
          }
          strcat(path,fileName);
          if (stat(path,&fileDetails) != -1)
          {
             
             if  ( S_ISDIR( fileDetails.st_mode ))
             {
                
                if (strcmp(dirEntry.d_name,".")==0 ||
                   strcmp(dirEntry.d_name, "..") == 0)
                {
                   continue;
                }
                else if (chdir(path) != -1)
                {
                   openDirectory(path,fp,fileType);
                }
                if ( fileType == CWS_TYPE_DIR )
                {
                    fprintf(
                        fp,
                        "%s %ld %ld %ld %ld %o\n",
                        path,
                        fileDetails.st_size,
                        fileDetails.st_ctime,
                        fileDetails.st_atime,
                        fileDetails.st_mtime,
                        fileDetails.st_mode);
                }
             }
             if ( fileType == CWS_TYPE_PLAIN )
             {
                 fprintf(
                     fp,
                     "%s %ld %ld %ld %ld %o\n",
                     path,
                     fileDetails.st_size,
                     fileDetails.st_ctime,
                     fileDetails.st_atime,
                     fileDetails.st_mtime,
                     fileDetails.st_mode);
             }
          }
          
       }
       closedir(dirPoint);
    }
    else
    {
      return errno;
    }
    return 0;
}

int CWS_Next_Enum(void *handle, CWS_FILE* cwsf)
{
  /* read next entry from result file */
  char result[2000];
  CWS_Control *cc = (CWS_Control *)handle;
  int state=0;
  if (cc && cwsf && fgets(result,sizeof(result),cc->fp))
#if defined CMPI_PLATFORM_WIN32_IX86_MSVC
    state=0<sscanf(result,"%s %I64d %ld %ld %ld %o",
#else
    state=0<sscanf(result,"%s %lld %ld %ld %ld %o",
#endif
                   cwsf->cws_name,
                   &cwsf->cws_size,
                   &cwsf->cws_ctime,
                   &cwsf->cws_mtime,
                   &cwsf->cws_atime,
                   &cwsf->cws_mode);
  return state;
}

void CWS_End_Enum(void *handle)
{
  /* end enumeration */
  CWS_Control *cc = (CWS_Control *)handle;
  if (cc) {
    fclose(cc->fp);
    remove(cc->name);
    free(cc);
  }
}

int CWS_Get_File(const char *file, CWS_FILE* cwsf)
{
  int         state=0;
  struct stat statbuf;
  if (file && cwsf && stat(file,&statbuf)==0) {
    strcpy(cwsf->cws_name,file);
    cwsf->cws_size=statbuf.st_size;
    cwsf->cws_ctime=statbuf.st_ctime;
    cwsf->cws_mtime=statbuf.st_mtime;
    cwsf->cws_atime=statbuf.st_atime;
    cwsf->cws_mode=statbuf.st_mode;
    state=1;
  }
  return state;
}

int CWS_Update_File(CWS_FILE* cwsf)
{

  int         state=0;
  struct stat statbuf;
  int         mode;

  /* only change mode */
  if (cwsf && stat(cwsf->cws_name,&statbuf)==0) {
    mode=(statbuf.st_mode & 07077) | (cwsf->cws_mode & 0700);
    state=chmod(cwsf->cws_name,mode)==0;
  }
  return state;
}

int CWS_Update_FileSize(CWS_FILE* cwsf, const char *fn)
{
    /* can't change filesize for non-simulated case */
    return 0;
}

int CWS_Create_Directory(CWS_FILE* cwsf)
{

  int         state=0;
  int         mode;

  if (cwsf) {
    mode=cwsf->cws_mode | 0077; /* plus umask */
    state=mkdir(cwsf->cws_name,mode)==0;
  }
  return state;
}

int CWS_Get_FileType(const char *file, char* typestring, size_t tslen)
{
    char  cmdbuffer[300];
    char  cmdout[300];
    FILE *fcmdout;

    if (file && tmpnam(cmdout))
    {
        sprintf(cmdbuffer,"file %s > %s",file,cmdout);
        if (system(cmdbuffer)!=0)
        {
            remove(cmdout);
            return 1;
        }
        if (!(fcmdout = fopen(cmdout,"r")))
        {
            remove(cmdout);
            return 1;
        }
        if (fgets(typestring,tslen,fcmdout))
        {
            fclose(fcmdout);
            remove(cmdout);
            return 0 ;
        }
        fclose(fcmdout);
        remove(cmdout);
    }
    return 1;
}

#endif
