#
# Regular cron jobs for the epics-pmacutil-dev package
#
0 4	* * *	root	[ -x /usr/bin/epics-pmacutil-dev_maintenance ] && /usr/bin/epics-pmacutil-dev_maintenance
