/**
 * This file is part of AdelaideOS (http://adelaideos.sourceforge.net)
 * Original version developed by Peter Kelly <kellypmk@gmail.com>
 * The contents of this file are in the public domain.
 */

#include "constants.h"
#ifdef USE_UNIXPROC
#include "user.h"

int main(int argc, char **argv)
{
  if (2 > argc) {
    printf("Usage: cat <filename>\n");
    return -1;
  }

  char *path = argv[1];
  int fd;
  if (0 > (fd = open(path,0))) {
    perror(argv[1]);
    return -1;
  }
  else {
    int bsize = 20;
    char buf[bsize];
    int r;
    while (0 < (r = read(fd,buf,bsize))) {
      write(STDOUT_FILENO,buf,r);
    }
    if (0 > r) {
      perror("read");
      return -1;
    }
    close(fd);
  }
  return 0;
}
#endif /* USE_UNIXPROC */
