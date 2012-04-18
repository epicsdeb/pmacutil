#!/bin/bash
IOC_PATH=~/local/data/work/hg/repos/modules/pmacUtil
applypvs  -i $IOC_PATH/pypdb/pmacUtil_pmacStatusAxis.pot \
	-o $IOC_PATH/fls \
	$IOC_PATH/pmacUtilApp/opi/edl/pmacStatusAxis.edl
