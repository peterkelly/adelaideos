#!/bin/bash
if [ ! -f grub.img ]; then
  echo "File grub.img not found - cannot build boot disk image"
  exit 1
fi
#ifdef USE_UNIXPROC

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
#endif

mdel -i grub.img ::kernel.bin || true
#ifdef USE_UNIXPROC
mdel -i grub.img ::filesystem.img || true
#endif
echo -n "Copying files to disk image... "
mcopy -i grub.img kernel.bin :: || exit 1
#ifdef USE_UNIXPROC
mcopy -i grub.img filesystem.img :: || exit 1
#endif
echo done
