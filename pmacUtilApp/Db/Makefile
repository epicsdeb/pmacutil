TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#----------------------------------------------------
#  Optimization of db files using dbst (DEFAULT: NO)
#DB_OPT = YES

#----------------------------------------------------
# In a Diamond Support Module, install
# templates like this
#
DB += energise.template
DB += pmacVariableWrite.template
DB += gather.template
DB += motorstatus.template
DB += translated_motor.template
DB += autohome.template
DB += dls_pmac_asyn_motor.template
DB += dls_pmac_cs_asyn_motor.template
DB += dls_pmac_patch_asyn_motor.template
DB += nanomotor_pmac.template
DB += nanomotor_axis.template
DB += compensationtable.template
DB += energyModes.template
DB += motion_stop.template
DB += encoderReadback.template
DB += positionCompare.template
DB += positionCompare_nojitter.template
DB += pmacStatus.template
DB += pmacStatusAxis.template
DB += pmacStatus8Axes.template
DB += pmacStatus32Axes.template

# not using streams since this doesnt work with tpmac
#DB += energise_streams.template

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
