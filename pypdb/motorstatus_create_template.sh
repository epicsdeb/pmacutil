#!/bin/bash
getpvs -I /usr/lib/epics/dbd -I . \
	-o pmacUtil_motorstatus.pot \
	base.dbd \
	~/local/data/work/hg/repos/modules/pmacUtil/pmacUtilApp/Db/motorstatus.db