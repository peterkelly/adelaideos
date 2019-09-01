#!/bin/bash
set -eu
BASE=`pwd`
if [ ! -f GenVersions.class ]; then
  javac GenVersions.java
fi
rm -rf versions
mkdir versions
cp -pR testfiles versions/testfiles
java -ea GenVersions source versions
for i in versions/version*; do
  chmod +x $i/mkbootimage.sh
done
for ((i = 1; i <= 7; i++)); do
    rm -f ver/version$i/link-user.ld
done
