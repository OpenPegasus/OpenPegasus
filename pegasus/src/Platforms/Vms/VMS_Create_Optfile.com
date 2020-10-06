$ ! VMS_Create_Optfile.com
$  env = f$parse(f$environment("PROCEDURE"),,,"NAME")
$  v=f$verify(0+f$integer(f$trnlnm("''env'"+"_VERIFY")))
$  Goto Run_Begin
$ !
$ ! Use the above "env" symbol to enable "set verify" -
$ !     $define VMS_CREATE_OPTFILE_VERIFY 1	
$ !
$ !
$ !
$ ! Create an option file for the executable or sharable image
$ ! 
$ ! P1 = Options filename
$ ! P2 = Value of LIBRARIES
$ ! P3 = Value of EXTRA_LIBRARIES
$ ! P4 = Value of VMS_VECTOR
$ ! P5 = Value of LINKER_OPTIONS
$ ! if OBJECTS_IN_OPTIONFILE set
$ !   P6 = Value of SOURCES
$ !   P7 = Objects directory
$ ! endif
$Run_Begin:
$ !
$  Prev_lib = ""
$  First_obj = "true"
$  use_static = f$edit(f$trnlnm("PEGASUS_USE_STATIC_LIBRARIES"),"LOWERCASE")
$ !
$ sav_ver = f$environment("VERIFY_PROCEDURE")
$ if sav_ver
$ then
$   MyOut :== "Write Sys$output"
$ else
$   MyOut :== "!"
$ endif
$ ! 
$  'MyOut "%VMSCROPT - P1 = ''P1'"
$  'MyOut "%VMSCROPT - P2 = ''P2'"
$  'MyOut "%VMSCROPT - P3 = ''P3'"
$  'MyOut "%VMSCROPT - P4 = ''P4'"
$  'MyOut "%VMSCROPT - P5 = ''P5'"
$  'MyOut "%VMSCROPT - P6 = ''P6'"
$  'MyOut "%VMSCROPT - P7 = ''P7'"
$ ! 
$ !
$ ! Use the program name to open a new .opt file
$ ! 
$  if "''P1'" .eqs. ""
$  then
$    Goto run_error
$  endif
$ ! 
$  optname = f$trnlnm("PEGASUS_VMSHOMEA") + "opt]" + "''P1'" + ".opt"
$ ! 
$  'MyOut "%VMSCROPT - Optname = ''Optname'"
$ !
$  open/write/error=optfile_openerror optfile 'Optname
$ !
$  Optnum = 0
$ !
$Get_linker_opts_loop:
$ ! 
$ ! Make the list of linker options
$ ! 
$  linkopt = f$element(Optnum, " ", "''P5'")
$  if (linkopt .nes. " ") .and. (linkopt .nes. "")
$  then
$ ! 
$    Optnum = Optnum + 1
$ ! 
$ ! Write it too the option file
$ ! 
$    write/error=optfile_writeerror optfile "''linkopt'"
$ ! 
$ ! Try again
$ ! 
$    Goto Get_linker_opts_loop
$  endif
$ !
$ ! 
$ ! Look for "OBJECTS_IN_OPTIONFILE".
$ ! 
$  if "''P7'" .nes. ""
$  then
$    'MyOut "%VMSCROPT - OBJECTS_IN_OPTIONFILE is defined!"
$    Write sys$output "%VMSCROPT - OBJECTS_IN_OPTIONFILE is defined!"
$ ! 
$ ! Write the object filename list to the option file.
$ ! 
$    Objnum = 0
$ !
$Get_objects_loop:
$ ! 
$ ! Make the list of sources/objects
$ ! 
$    PegObj = f$element(Objnum, " ", "''P7'")
$ ! 
$    if (PegObj .nes. " ") .and. (PegObj .nes. "")
$    then
$ ! 
$ ! Create the object file entry
$ ! 
$      if (first_obj .eqs. "true")
$      then
$        objnam = "''P6'" + "''PegObj'" + ".obj"
$        first_obj = "false"
$      else
$        objnam = "''PegObj'" + ".obj"
$      endif
$ ! 
$      Objnum = Objnum + 1
$ ! 
$ ! Write it too the option file
$ ! 
$      write/error=optfile_writeerror optfile "''objnam'"
$ ! 
$ ! Try again
$ ! 
$      Goto Get_objects_loop
$    endif
$  endif
$ !
$  Libnum = 0
$  prev_libtype = ""
$ !
$Get_libraries_loop:
$ ! 
$ ! Make the list of libraries
$ ! 
$  PegLib = f$element(Libnum, " ", "''P2'")
$ ! 
$  if (PegLib .nes. " ") .and. (PegLib .nes. "")
$  then
$ ! 
$ ! if .exe exists, use the shareable instead of the .olb.
$    is_dll = "false"
$    if "''use_static'" .eqs. "false" 
$    then 
$      dllname = -
         f$trnlnm("PEGASUS_VMSHOMEA") + "bin]" + "lib" + "''PegLib'" + ".exe"
$      if f$search(dllname) .nes. ""
$      then
$        is_dll = "true"
$      endif
$    endif
$ !
$ ! Create the library file entry
$ ! 
$    if "''is_dll'" .eqs. "true"
$    then
$      subdir = "bin"
$      libtype = "share"
$    else
$      subdir = "lib"
$      libtype = "lib"
$    endif
$  !
$    if (prev_libtype .nes. libtype)
$    then
$      libnam = f$trnlnm("PEGASUS_VMSHOMEA") + "''subdir']" + -
                         "lib" + "''PegLib'" + "/''libtype'"
$    else
$      libnam = "lib" + "''PegLib'" + "/''libtype'"
$    endif
$    prev_libtype = libtype
$ ! 
$    Libnum = Libnum + 1
$ ! 
$ ! Write it too the option file
$ ! 
$    write/error=optfile_writeerror optfile "''libnam'"
$ ! 
$ ! Try again
$ ! 
$    Goto Get_libraries_loop
$  endif
$ !
$  Libnum = 0
$  prev_libdir = ""
$  prev_libtype = ""
$ !
$Get_extra_libraries_loop:
$ ! 
$ ! Make the list of libraries
$ ! 
$  PegLib = f$element(Libnum, " ", "''P3'")
$ ! 
$  if (PegLib .nes. " ") .and. (PegLib .nes. "")
$  then
$    LibType = f$extract(0,2,PegLib)
$    Lib = f$extract(2,f$length(PegLib)-2,PegLib)
$    if LibType .eqs. "-L"
$    then
$      if f$trnlnm(Lib) .eqs. ""
$      then
$        libdir = Lib
$      else
$        libdir = f$trnlnm(Lib)
$      endif
$      if f$locate(":", libdir) .eq. f$length(libdir)
$      then
$        libdir = libdir + ":"
$      endif
$    else
$ ! 
$ ! if .exe exists, use the shareable instead of the .olb.
$ ! check the name and the name prepended with "lib"
$ !
$      is_dll = "false"
$      dllname = libdir + Lib + ".exe"
$      if f$search(dllname) .nes. ""
$      then
$        is_dll = "true"
$      else
$        olbname = libdir + Lib + ".olb"
$        if f$search(olbname) .eqs. ""
$        then
$          Lib = "lib" + Lib
$          dllname = libdir + Lib + ".exe"
$          if f$search(dllname) .nes. ""
$          then
$            is_dll = "true"
$          endif
$        endif
$      endif
$ !
$ ! Create the library file entry
$ ! 
$      if "''is_dll'" .eqs. "true"
$      then
$        libtype = "share"
$      else
$        libtype = "lib"
$      endif
$      !
$      if (prev_libtype .nes. libtype) .or. (prev_libdir .nes. libdir)
$      then
$        libnam = libdir + Lib + "/''libtype'"
$      else
$        libnam = Lib + "/''libtype'"
$      endif
$      prev_libdir = libdir
$      prev_libtype = libtype
$ ! 
$ ! Write it too the option file
$ ! 
$      write/error=optfile_writeerror optfile "''libnam'"
$    endif
$ ! 
$    Libnum = Libnum + 1
$ ! 
$ ! Try again
$ ! 
$    Goto Get_extra_libraries_loop
$  endif
$ !
$ ! Looking for "VMS_VECTOR"
$ ! Could be comma separated list of vectors
$ ! In the dynamic case, the vector is already defined by the compiler
$ ! 
$ if "''use_static'" .eqs. "true"
$ then
$   'MyOut "%VMSCROPT - VMS_VECTOR defined"
$   vec_num = 0
$Get_vms_vector_loop:
$   vec = f$element(vec_num, " ", P4)
$   if (vec .nes. " ") .and. (vec .nes. "")
$   then
$     vecsym = "SYMBOL_VECTOR=(" + "''vec'" + "=PROCEDURE)"
$     'MyOut "%VMSCROPT -  vecsym = ''vecsym'"
$     write/error=optfile_writeerror optfile "''vecsym'"
$     vec_num = vec_num + 1
$     goto Get_vms_vector_loop
$   endif
$ endif
$ ! 
$ ! Close the options file.
$ ! 
$  Close optfile
$ ! 
$  Goto run_exit
$ ! 
$makefile_openerror:
$ Write sys$output "%VMSCROPT - Can't open option file! - ''Optname'"
$ ! 
$ Goto run_exit
$ ! 
$optfile_writeerror:
$ !
$ Write sys$output "%VMSCROPT - Error writing options file! - ''Optname'"
$ ! 
$ Goto run_exit
$ !
$run_error:
$ ! 
$ Write sys$output "%VMSCROPT - Can't find option filename!"
$ Goto run_exit
$ ! 
$run_exit:
$ Exit 1+0*f$verify(v)
