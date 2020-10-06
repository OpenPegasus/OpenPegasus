PLATFORM_FILES=$(wildcard $(ROOT)/mak/platform*.mak)
PLATFORM_TEMP=$(subst $(ROOT)/mak/platform_,, $(PLATFORM_FILES))
VALID_PLATFORMS=$(subst .mak,  , $(PLATFORM_TEMP))

ifndef PEGASUS_PLATFORM
    $(error PEGASUS_PLATFORM environment variable undefined. Please set to\
        one of the following:$(VALID_PLATFORMS))
endif

PLATFORM_FILE = $(ROOT)/mak/platform_$(PEGASUS_PLATFORM).mak
ifneq ($(wildcard $(PLATFORM_FILE)), )
    include $(PLATFORM_FILE)
else
  $(error  PEGASUS_PLATFORM environment variable must be set to one of\
        the following:$(VALID_PLATFORMS))
endif

include $(ROOT)/mak/SDKEnvVar.mak
