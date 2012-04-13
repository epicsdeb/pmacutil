TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#----------------------------------------------------
#  Optimization of db files using dbst (DEFAULT: NO)
#DB_OPT = YES

#----------------------------------------------------
#
DB += energise.db
DB += pmacVariableWrite.db
DB += gather.db
DB += motorstatus.db
DB += translated_motor.template
DB += autohome.db
DB += pmac_asyn_motor.template
DB += pmac_cs_asyn_motor.template
DB += pmac_patch_asyn_motor.template
DB += nanomotor_pmac.template
DB += nanomotor_axis.template
DB += compensationtable.db
DB += energyModes.db
DB += motion_stop.template
DB += encoderReadback.template
DB += positionCompare.db
DB += positionCompare_nojitter.db
DB += pmacStatus.db
DB += pmacStatusAxis.db
DB += pmacStatus8Axes.db
DB += pmacStatus32Axes.db

# not using streams since this doesnt work with tpmac
#DB += energise_streams.template

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
