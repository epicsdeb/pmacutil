#!/bin/bash
getpvs -I /usr/lib/epics/dbd -I . -I ~/local/data/work/hg/repos/modules/genSub/dbd -o pmacUtil_pmacStatus.pot genSubRecord.dbd base.dbd ../pmacUtilApp/Db/pmacStatus.vdb
