#!/bin/sh
folderpath="/mnt/server/nishimoto/output/dynamic_r_index"
querypath="/mnt/server/nishimoto/output/drfmi/locate"
logpath="/mnt/server/nishimoto/result/dynamic_r_index/locate_100"

list=("cere" "modified_coreutils" "einstein.de.txt" "einstein.en.txt" "Escherichia_Coli" "influenza" "kernel" "para" "world_leaders" "chr19-1000" "enwiki-20241201-pages-meta-history1.xml-p1p812" )


for item in "${list[@]}" ; do
  set -- $item
  echo "Search $2"
  nohup /usr/bin/time -f "#locate, DR-index, 8, $1, pattern_length, 100, Time(sec), %e, Memory(KB), %M" ./query.out -i ${folderpath}/$1.drfmi -q ${querypath}/$1.locate.100.tsv -w ${logpath}/$1.r.locate.100.log >> ${logpath}/locate.dr_100.log
done