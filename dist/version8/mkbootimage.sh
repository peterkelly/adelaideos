#!/bin/bash
if [ ! -f grub.img ]; then
  echo "File grub.img not found - cannot build boot disk image"
  exit 1
fi

if [ -f fstool ]; then
  if [ ! -d ../testfiles ]; then
    echo "Directory ../testfiles does not exist; cannot build filesystem image"
    exit 1
  fi

  # Used for later versions of the kernel, when we have a file system
  echo -n "Building filesystem image... "
  if [ -f sh ]; then
    cp -f sh ls cat find mptest ../testfiles/bin || exit 1
  fi
  ./fstool -build filesystem.img ../testfiles >/dev/null || exit 1
  echo done
else
  rm -f filesystem.img
  touch filesystem.img
fi

mdel -i grub.img ::kernel.bin || true
mdel -i grub.img ::filesystem.img || true
echo -n "Copying files to disk image... "
mcopy -i grub.img kernel.bin :: || exit 1
mcopy -i grub.img filesystem.img :: || exit 1
echo done
