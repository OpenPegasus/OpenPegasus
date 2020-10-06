# Start of section pegasus/rpm/tog-specfiles/tog-pegasus-clean.spec
#
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT;
[ "${RPM_BUILD_DIR}" != "/" ] && rm -rf ${RPM_BUILD_DIR}/%{name}-%{version};
#
# End of section pegasus/rpm/tog-specfiles/tog-pegasus-clean.spec
