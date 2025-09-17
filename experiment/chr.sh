#!/bin/sh
folderpath="/mnt/server/nishimoto/bigbwt_data"
logpath="/mnt/server/nishimoto/result"
outputpath="/mnt/server/nishimoto/output"

list=("chr19-1000")

for item in "${list[@]}" ; do
  set -- $item
  echo "build $1.bwt"
  nohup /usr/bin/time -f "#bigbwt, $1, Time(sec), %e, Memory(KB), %M" ./Big-BWT/bigbwt ${folderpath}/$1 >> ${logpath}/bigbwt/bigbwt_$1.log
  echo "build $1.dri"
  nohup /usr/bin/time -f "#build_dr $1, %e sec, %M KB" ./dynamic_r_index/build_r_index.out -i ${folderpath}/$1.bwt -o ${outputpath}/dynamic_r_index/$1.drfmi -u 1  >> ${logpath}/dynamic_r_index/build_$1_r.log
  echo "build $1.dse"
  nohup /usr/bin/time -f "#build dynamic string index, $1, %e sec, %M KB" ./dynamic_signature_encoding/build_index.out -i ${folderpath}/$1 -o ${outputpath}/dynamic_signature_encoding/$1.dse  >> ${logpath}/dynamic_signature_encoding/build_$1_dse.log
  echo "build ri-index: $1"
  nohup /usr/bin/time -f "#build, ri-index, $1, Time(sec), %e, Memory(KB), %M" ./r-index/mod_ri-build -o ${outputpath}/drfmi/$1 ${folderpath}/$1.bwt  >> ${logpath}/drfmi/build_$1_ri.log
done
echo "Finished."
