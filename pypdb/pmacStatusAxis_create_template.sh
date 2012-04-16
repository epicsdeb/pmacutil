#!/bin/bash
IOC_PATH=~/local/data/work/hg/repos/modules/pmacUtil

getpvs -I /usr/lib/epics/dbd \
	-I . \
	-o pmacUtil_pmacStatusAxis.pot \
	base.dbd \
	$IOC_PATH/pmacUtilApp/Db/pmacStatusAxis.db
