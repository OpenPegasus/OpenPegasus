# Test of a fixed configure as bash shell
echo $PWD

# Set Root to the current directory.
export ROOT=$PWD
export PEGASUS_ROOT=$ROOT/pegasus
export PEGASUS_HOME=$ROOT/home
export PATH=$PEGASUS_HOME/bin:$PATH
mkdir $PEGASUS_HOME
export LD_LIBRARY_PATH=$PEGASUS_HOME/lib64:

export PEGASUS_PLATFORM=LINUX_X86_64_GNU

# Storage mode of the CIM repository created on initial build
# This may be modified using cimconfig.
# The default if this is not uses is XML
# the possible modes are BIN and XML
export PEGASUS_REPOSITORY_MODE=XML

export PEGASUS_ENABLE_AUDIT_LOGGER=true
export PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT=true
export PEGASUS_ENABLE_INDICATION_COUNT=true

export PEGASUS_ENABLE_EMAIL_HANDLER=true
export PEGASUS_ENABLE_INTEROP_PROVIDER=true
export PEGASUS_ENABLE_PROTOCOL_WSMAN=true
export PEGASUS_ENABLE_SYSTEM_LOG_HANDLER=true
export PEGASUS_ENABLE_USERGROUP_AUTHORIZATION=true
export PEGASUS_ENABLE_CQL=true
export PEGASUS_ENABLE_WQL=true
export PEGASUS_ENABLE_IPV6=true
export PEGASUS_DISABLE_PROV_USERCTXT=true
export PEGASUS_DEBUG=1
export PEGASUS_HAS_SSL=true
export PEGASUS_PAM_AUTHENTICATION=true
export PEGASUS_USE_PAM_STANDALONE_PROC=false
export PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER=true
export PEGASUS_PLATFORM_FOR_32BIT_PROVIDER_SUPPORT=LINUX_X86_64_GNU
