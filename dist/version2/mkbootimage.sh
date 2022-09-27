#!/bin/bash
if [ ! -f grub.img ]; then
  echo "File grub.img not found - cannot build boot disk image"
  exit 1
fi

mdel -i grub.img ::kernel.bin || true
echo -n "Copying files to disk image... "
mcopy -i grub.img kernel.bin :: || exit 1
echo done
