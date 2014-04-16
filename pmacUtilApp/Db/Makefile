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
DB += translated_motor.db
DB += autohome.db
DB += pmac_asyn_motor.db
DB += pmac_cs_asyn_motor.db
DB += pmac_patch_asyn_motor.db
DB += nanomotor_pmac.db
DB += nanomotor_axis.db
DB += compensationtable.db
DB += energyModes.db
DB += motion_stop.db
DB += encoderReadback.db
DB += positionCompare.db
DB += positionCompare_nojitter.db
DB += pmacStatus.db
DB += pmacStatusAxis.db
DB += pmacStatus8Axes.db
DB += pmacStatus32Axes.db
DB += pmacGPOutput.db
DB += pmacGPOutput8.substitutions
DB += enable_plc.db

# not using streams since this doesnt work with tpmac
#DB += energise_streams.template

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
