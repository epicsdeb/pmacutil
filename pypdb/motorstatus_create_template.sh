#!/bin/bash
getpvs -I /usr/lib/epics/dbd -I . \
	-o pmacUtil_motor_status.pot \
	base.dbd \
	~/local/data/work/hg/repos/modules/pmacUtil/pmacUtilApp/Db/motor_status.template
