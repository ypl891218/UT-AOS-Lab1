#!/bin/bash

timestamp=$(date -Iseconds)
log="./log/result."$timestamp
touch $log

# for -P, -e, -S
xarg1=$1
xarg2=$2 
xarg3=$3

[ ! -f 1gb_file ] && dd if=/dev/zero of=1gb_file bs=1M count=1024

make

echo "./task4 -a -s $xarg1 $xarg2 $xarg3" >> $log
./task4 -a -s $xarg1 $xarg2 $xarg3  >> $log

echo "./task -a -r $xarg1 $xarg2 $xarg3" >> $log
./task4 -a -r $xarg1 $xarg2 $xarg3 >> $log

echo "./task4 -f 1gb_file -s -p $xarg1 $xarg2 $xarg3" >> $log 
./task4 -f 1gb_file -s -p $xarg1 $xarg2 $xarg3 >> $log

echo "./task4 -f 1gb_file -s -m $xarg1 $xarg2 $xarg3" >> $log
./task4 -f 1gb_file -s -m $xarg1 $xarg2 $xarg3 >> $log

echo "./task4 -f 1gb_file -r -p $xarg1 $xarg2 $xarg3" >> $log
./task4 -f 1gb_file -r -p $xarg1 $xarg2 $xarg3 >> $log

echo "./task4 -f 1gb_file -r -m $xarg1 $xarg2 $xarg3" >> $log
./task4 -f 1gb_file -r -m $xarg1 $xarg2 $xarg3 >> $log

