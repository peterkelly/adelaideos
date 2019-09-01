#!/bin/bash
BASE=`pwd`
for i in versions/version*; do
  cd $BASE/$i
  make clean
done
