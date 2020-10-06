# Start of section pegasus/rpm/tog-specfiles/tog-pegasus-preun.spec
#           install   remove   upgrade  reinstall
#  preun       -        0         1         -
if [ $1 -eq 0 ]; then
   # Check if the cimserver is running
   isRunning=`ps -el | grep cimserver |  grep -v "grep cimserver"`
   if [ "$isRunning" ]; then
      %PEGASUS_SBIN_DIR/cimserver -s
   fi
   /sbin/chkconfig --del %{Flavor}-pegasus;
   rm -f %PEGASUS_CONFIG_DIR/cimserver_current.conf;
   [ -d %PEGASUS_REPOSITORY_DIR ]  && rm -rf %PEGASUS_REPOSITORY_DIR;
   [ -d %PEGASUS_VARDATA_CACHE_DIR ]  && rm -rf %PEGASUS_VARDATA_CACHE_DIR;
   rm -f %PEGASUS_LOCAL_DOMAIN_SOCKET_PATH;
   rm -f %PEGASUS_CIMSERVER_START_FILE;
   rm -f %PEGASUS_CIMSERVER_START_LOCK_FILE;
   rm -f %PEGASUS_CONFIG_DIR/%PEGASUS_SSL_CERT_FILE
   rm -f %PEGASUS_CONFIG_DIR/%PEGASUS_SSL_KEY_FILE
   rm -f %PEGASUS_CONFIG_DIR/%PEGASUS_SSL_TRUSTSTORE
   rm -f %PEGASUS_CONFIG_DIR/ssl.cnf
fi
#
# End of section pegasus/rpm/tog-specfiles/tog-pegasus-preun.spec
