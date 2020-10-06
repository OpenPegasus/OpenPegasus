# Start of section pegasus/rpm/tog-specfiles/tog-pegasus-post.spec
#
#           install   remove   upgrade  reinstall
# %post        1        -         2         2
#
   export PEGASUS_ARCH_LIB=%PEGASUS_ARCH_LIB

   if [ $1 -eq 1 ]; then
%if %{AUTOSTART}
       /sbin/chkconfig --add %{Flavor}-pegasus
%endif
   :;
   elif [ $1 -gt 0 ]; then
       /etc/init.d/%{Flavor}-pegasus condrestart
   :;
   fi
#
# End of section pegasus/rpm/tog-specfiles/tog-pegasus-post.spec
