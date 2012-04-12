#!/bin/bash
getpvs -I /usr/lib/epics/dbd -I . -o pmacUtil_pmacVariableWriteRecords.pot base.dbd asynRecord.dbd sCalcoutRecord.dbd ~/local/data/work/hg/repos/pmacUtil/pmacUtilApp/Db/pmacVariableWriteRecords.vdb
