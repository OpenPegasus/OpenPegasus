# Start of section pegasus/rpm/tog-specfiles/tog-pegasus-arch.spec
#
%ifarch ia64
%global PEGASUS_HARDWARE_PLATFORM LINUX_IA64_GNU
%else
%ifarch x86_64
%if %{PEGASUS_BUILD_WITH_CLANG}
%global PEGASUS_HARDWARE_PLATFORM LINUX_X86_64_CLANG
%else
%global PEGASUS_HARDWARE_PLATFORM LINUX_X86_64_GNU
%endif
%else
%ifarch ppc
%global PEGASUS_HARDWARE_PLATFORM LINUX_PPC_GNU
%else
%ifarch ppc64 pseries
%global PEGASUS_HARDWARE_PLATFORM LINUX_PPC64_GNU
%else
%ifarch s390
%global PEGASUS_HARDWARE_PLATFORM LINUX_ZSERIES_GNU
%else
%ifarch s390x zseries
%global PEGASUS_HARDWARE_PLATFORM LINUX_ZSERIES64_GNU
%else
%if %{PEGASUS_BUILD_WITH_CLANG}
%global PEGASUS_HARDWARE_PLATFORM LINUX_IX86_CLANG
%else
%global PEGASUS_HARDWARE_PLATFORM LINUX_IX86_GNU
%endif
%endif
%endif
%endif
%endif
%endif
%endif
#
# End of section pegasus/rpm/tog-specfiles/tog-pegasus-arch.spec
