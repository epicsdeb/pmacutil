#!/bin/bash
IOC_PATH=~/local/data/work/hg/repos/modules/pmacUtil
applypvs  -i $IOC_PATH/pypdb/pmacUtil_pmac_asyn_motor.pot \
	-o $IOC_PATH/fls \
	$IOC_PATH/pmacUtilApp/Db/pmac_asyn_motor.template
