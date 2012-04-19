#!/bin/bash
getpvs -I /usr/lib/epics/dbd -I . \
	-o pmacUtil_pmac_asyn_motor.pot \
	base.dbd \
	~/local/data/work/hg/repos/modules/pmacUtil/pmacUtilApp/Db/pmac_asyn_motor.template
