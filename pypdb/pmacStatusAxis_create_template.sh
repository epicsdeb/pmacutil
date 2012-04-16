#!/bin/bash
IOC_PATH=~/local/data/work/hg/repos/pmacUtil

getpvs -I /usr/lib/epics/dbd \
	-I . \
	-o pmacUtil_pmacStatus.pot \
	base.dbd \
	$IOC_PATH/pmacUtilApp/Db/pmacStatus.vdb
