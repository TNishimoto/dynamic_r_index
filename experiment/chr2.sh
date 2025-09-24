#!/bin/sh
indexpath="/mnt/server/nishimoto/output"
logpath="/mnt/server/nishimoto/result"
outputpath="/mnt/server/nishimoto/output"
querypath="/mnt/server/nishimoto/output/drfmi"


list=("chr19-1000")

for item in "${list[@]}" ; do
  set -- $item
  echo "Search $1"
  nohup /usr/bin/time -f "#locate, DR-index, 8, $1, pattern_length, 100, Time(sec), %e, Memory(KB), %M" ./dynamic_r_index/query.out -i ${indexpath}/dynamic_r_index/$1.drfmi -q ${querypath}/locate/$1.locate.100.tsv -w ${logpath}/dynamic_r_index/locate_100/$1.r.locate.100.log -u 1 >> ${logpath}/dynamic_r_index/locate_100/locate.dr_100.log
  echo "Search $1"
  nohup /usr/bin/time -f "#locate, DSE-index, $1, pattern_length, 100, Time(sec), %e, Memory(KB), %M" ./dynamic_signature_encoding/query.out -i ${indexpath}/dynamic_signature_encoding/$1.dse -q ${querypath}/locate/$1.locate.100.tsv -w ${logpath}/dynamic_signature_encoding/locate_100/$1.dse.locate.100.log -u 1 >> ${logpath}/dynamic_signature_encoding/locate_100/locate.dse_100.log
  echo "Search $1"
  nohup /usr/bin/time -f "#locate, r-index, _, $1, pattern_length, 100, Time(sec), %e, Memory(KB), %M" ./r-index/ri-locate ${indexpath}/drfmi/$1.ri ${querypath}/locate/$1.locate.100.txt >> ${logpath}/drfmi/locate_100/locate_$1_ri.100.log

  #echo "Update $1"
  #nohup /usr/bin/time -f "#update, DR-index, _, $1, pattern_length, 1, Time(sec), %e, Memory(KB), %M" ./dynamic_r_index/query.out -i ${indexpath}/dynamic_r_index/$1.drfmi -q ${querypath}/query/$1.update.1.tsv -w ${logpath}/dynamic_r_index/$1.r.update.1.log >> ${logpath}/dynamic_r_index/update_r.1.log
  #echo "Update $1"
  #nohup /usr/bin/time -f "#update, DSE-index, _, $1, pattern_length, 1, Time(sec), %e, Memory(KB), %M" ./dynamic_signature_encoding/query.out -i ${indexpath}/dynamic_signature_encoding/$1.dse -q ${querypath}/query/$1.update.1.tsv -w ${logpath}/dynamic_signature_encoding/$1.dse.update.1.log >> ${logpath}/dynamic_signature_encoding/update_dse.1.log
done
echo "Finished."