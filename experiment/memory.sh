#!/bin/sh
folderpath="/mnt/server/nishimoto/output/dynamic_r_index"
querypath="/mnt/server/nishimoto/output/drfmi/query"
logpath="/mnt/server/nishimoto/result/dynamic_r_index/memory"


list=("cere" "modified_coreutils" "einstein.de.txt" "einstein.en.txt" "Escherichia_Coli" "influenza" "kernel" "para" "world_leaders" "chr19-1000" "enwiki-20241201-pages-meta-history1.xml-p1p812" )

for item in "${list[@]}" ; do
  set -- $item
  echo "Update $1"
  nohup /usr/bin/time -f "#memory, DR-index, _, $1, Time(sec), %e, Memory(KB), %M" ./query.out -i ${folderpath}/$1.drfmi -q ${querypath}/empty.tsv -w ${logpath}/$1.r.update.1.log >> ${logpath}/update_r.1.log
done
