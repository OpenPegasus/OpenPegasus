# Start of section pegasus/rpm/tog-specfiles/tog-pegasus-requires.spec
#
Requires:           bash, sed, grep, coreutils, procps, openssl >= 0.9.6, pam
#Requires:          krb5-libs, chkconfig, SysVinit, bind-libs
Requires:           e2fsprogs, bind-utils, net-tools
Requires(post):     bash, sed, grep, coreutils, procps, openssl >= 0.9.6, pam
#Requires(post):    krb5-libs, chkconfig, SysVinit, bind-libs
Requires(post):     e2fsprogs, bind-utils, net-tools
Requires(pre):      bash, sed, grep, coreutils, procps, openssl >= 0.9.6, pam
#Requires(pre):     krb5-libs, chkconfig, SysVinit, bind-libs
Requires(pre):      e2fsprogs, bind-utils, net-tools
Requires(postun):   bash, sed, grep, coreutils, procps, openssl >= 0.9.6, pam
#Requires(postun):  krb5-libs, chkconfig, SysVinit, bind-libs
Requires(postun):   e2fsprogs, bind-utils, net-tools
