TOP=../..

include $(TOP)/configure/CONFIG

# -------------------------------
# Build an Diamond Support Module
# -------------------------------

#LIBRARY_IOC += pmacUtil

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxx.h
DBD += pmacUtilSupport.dbd

# xxxRecord.dbd will be installed into <top>/dbd
DBD += pmacUtil.dbd
pmacUtil_DBD += base.dbd
pmacUtil_DBD += asyn.dbd
pmacUtil_DBD += calcSupport.dbd
pmacUtil_DBD += stream.dbd
#pmacUtil_DBD += sscanSupport.dbd
pmacUtil_DBD += motorSupport.dbd
pmacUtil_DBD += busySupport.dbd

# The following are compiled and added to the support library
#pmacUtil_SRCS += gather.st
#pmacUtil_SRCS += compensationtable.c
#pmacUtil_SRCS += pmacStatusOutParse.c

#pmacUtil_LIBS += genSub
#pmacUtil_LIBS += seq pv
# We need to link against the EPICS Base libraries
#pmacUtil_LIBS += $(EPICS_BASE_IOC_LIBS)

include $(TOP)/configure/RULES
