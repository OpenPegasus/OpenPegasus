# Start of section pegasus/rpm/tog-specfiles/tog-pegasus-pre.spec
#           install   remove   upgrade  reinstall
# pre          1        -         2         2
#
# If it exists, the active OpenPegasus Repository
#    directory, %PEGASUS_REPOSITORY_DIR is renamed
#    to %PEGASUS_PREV_REPOSITORY_DIR.  If
#    %PEGASUS_PREV_REPOSITORY_DIR already exists,
#    it is saved as an archive file.

if [ -d %PEGASUS_REPOSITORY_DIR"/root#PG_Internal" ]; then
   #
   # Save the current repository to prev_repository
   #
   if [ -d %PEGASUS_PREV_REPOSITORY_DIR ]; then
        /bin/tar -C %PEGASUS_REPOSITORY_PARENT_DIR -cf \
            %PEGASUS_PREV_REPOSITORY_DIR`date '+%Y-%m-%d-%s.%N'`.tar \
            %PEGASUS_PREV_REPOSITORY_DIR_NAME
        rm -rf %PEGASUS_PREV_REPOSITORY_DIR
   fi
   /bin/mv %PEGASUS_REPOSITORY_DIR %PEGASUS_PREV_REPOSITORY_DIR
fi
if [ $1 -gt 0 ]; then
   #  Create the 'pegasus' user and group:
   /usr/sbin/groupadd pegasus > /dev/null 2>&1 || :;
   /usr/sbin/useradd -c "%{Flavor}-pegasus OpenPegasus WBEM/CIM services" \
        -g pegasus -s /sbin/nologin -r -d %PEGASUS_VARDATA_DIR pegasus \
         > /dev/null 2>&1 || :;
fi
if [ $1 -eq 2 ]; then
    rm -f %PEGASUS_LOCAL_DOMAIN_SOCKET_PATH;
    rm -f %PEGASUS_CIMSERVER_START_FILE;
    rm -f %PEGASUS_CIMSERVER_START_LOCK_FILE;
fi
#
# End of section pegasus/rpm/tog-specfiles/tog-pegasus-pre.spec
