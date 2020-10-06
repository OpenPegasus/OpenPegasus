# Start of section pegasus/rpm/tog-specfiles/tog-pegasus-postun.spec
#
#           install   remove   upgrade  reinstall
#  postun      -        0         1         -
if [ $1 -eq 0 ]; then
   /usr/sbin/userdel pegasus > /dev/null 2>&1 || :;
   /usr/sbin/groupdel pegasus > /dev/null 2>&1 || :;
fi;
#
# End of section pegasus/rpm/tog-specfiles/tog-pegasus-postun.spec
