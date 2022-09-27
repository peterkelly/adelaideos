/**
 * This file is part of AdelaideOS (http://adelaideos.sourceforge.net)
 * Original version developed by Peter Kelly <kellypmk@gmail.com>
 * The contents of this file are in the public domain.
 */

#include "kernel.h"
#include "filesystem.h"

extern char *filesystem;
extern process *current_process;

/*==============================================================================
syscall_stat

Implementation of the stat system call, which obtains information about a
particular file or directory. You should use this as a guideline for how to
implement the other system calls in assignment 3.
==============================================================================*/
int syscall_stat(const char *path, struct stat *buf)
{
  /* Ensure the supplied path name and buffer are valid pointers */
  if (!valid_string(path))
    return -EFAULT;
  if (!valid_pointer(buf,sizeof(struct stat)))
    return -EFAULT;

  /* Get the directory_entry object for this path from the filesystem */
  directory_entry *entry;
  int r;

  if (0 != (r = get_directory_entry(filesystem,path,&entry)))
    return r;

  /* Set all fields of the stat buffer */
  buf->st_mode = entry->mode;
  buf->st_uid = 0;
  buf->st_gid = 0;
  buf->st_size = entry->size;
  buf->st_mtime = entry->mtime;

  return 0;
}

