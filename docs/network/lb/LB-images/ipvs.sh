#!/bin/bash
ipvs_modules_dir="/lib/modules/`uname -r`/kernel/net/netfilter/ipvs"
for i in `ls $ipvs_modules_dir | sed -r 's#(.*).ko.sz#\1#'`; do
	modinfo -F filename $i &> /dev/null
	if [ $? -eq 0 ]; then
		modprobe $i
	fi
done
