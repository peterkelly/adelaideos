#!/bin/bash
BASE=`pwd`
for i in ver/version*; do
  cd $BASE/$i
  make clean
done
