#!/bin/sh
folderpath="/mnt/server/nishimoto/bigbwt_data"
outputpath="/mnt/server/nishimoto/output/dynamic_r_index"
logpath="/mnt/server/nishimoto/result/dynamic_r_index"

list=("cere" "modified_coreutils" "einstein.de.txt" "einstein.en.txt" "Escherichia_Coli" "influenza" "kernel" "para" "world_leaders" "chr19-1000" "enwiki-20241201-pages-meta-history1.xml-p1p812" )

for item in "${list[@]}" ; do
  set -- $item
  echo "build R $2"
  nohup /usr/bin/time -f "#build_r $1, %e sec, %M KB" ./build_r_index.out -i ${folderpath}/$1.bwt -o ${outputpath}/$1.drfmi -u 1  >> ${logpath}/build_r.log
done



echo "Finished."
