#!/bin/bash
BASE=`pwd`
if [ ! -f GenVersions.class ]; then
  javac GenVersions.java
fi
rm -rf ver
mkdir ver
cp -pR testfiles ver/testfiles
rm -f ver/testfiles/bin/*
java -ea GenVersions source ver
find ver -name .svn -exec rm -rf '{}' ';' >/dev/null 2>&1
for i in ver/version*; do
  chmod +x $i/mkbootimage.sh
done
rm -f ver/version1/link-user.ld
rm -f ver/version2/link-user.ld
rm -f ver/version3/link-user.ld
rm -f ver/version4/link-user.ld
rm -f ver/version5/link-user.ld
rm -f ver/version6/link-user.ld
rm -f ver/version7/link-user.ld
