/**
 * This file is part of AdelaideOS (http://adelaideos.sourceforge.net)
 * Original version developed by Peter Kelly <kellypmk@gmail.com>
 * The contents of this file are in the public domain.
 */

#include "kernel.h"

void *memset(void *b, int c, size_t len)
{
  char *dst = (char*)b;
  size_t i;
  for (i = 0; i < len; i++)
    dst[i] = c;
  return b;
}

void *memmove(void *dst, const void *src, size_t len)
{
  size_t pos;
  char *to = dst;
  const char *from = src;
  for (pos = 0; pos < len; pos++)
    *(to++) = *(from++);
  return dst;
}

void *memcpy(void *dst, const void *src, size_t len)
{
  return memmove(dst,src,len);
}

size_t strlen(const char *s)
{
  size_t len = 0;
  while (s[len])
    len++;
  return len;
}

int strcmp(const char *s1, const char *s2)
{
  int pos = 0;
  for (; s1[pos] && s2[pos]; pos++) {
    if (s1[pos] != s2[pos])
      return (s1[pos] - s2[pos]);
  }
  if (s1[pos])
    return -1;
  else if (s2[pos])
    return 1;
  else
    return 0;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
  int pos = 0;
  for (; (pos < n) && s1[pos] && s2[pos]; pos++) {
    if (s1[pos] != s2[pos])
      return (s1[pos] - s2[pos]);
  }
  if (pos < n) {
    if (s2[pos])
      return -1;
    else
      return 1;
  }
  return 0;
}

#define addchar(_pos,_c) { if ((_pos)+1 < size)   \
                             str[(_pos)] = (_c);  \
                           (_pos)++; }

void print_num(char *str, size_t size, size_t *pos, unsigned int val, unsigned int base)
{
  char *hexdigits = "0123456789abcdef";
  unsigned int digits = 1;
  unsigned int mult = 1;
  unsigned int x = val;
  for (; x >= base; x /= base, mult *= base)
    digits++;
  for (x = val; x >= base; x /= base, mult /= base)
    addchar(*pos,hexdigits[(val/mult)%base]);
  addchar(*pos,hexdigits[val%base]);
}

void print_field(char *str, size_t size, size_t *pos, const char *c, va_list *apptr)
{
  switch (*c) {
  case 's': {
    char *s;
    for (s = va_arg(*apptr,char*); *s; s++)
      addchar(*pos,*s);
    break;
  }
  case 'c': {
    char val = (char)va_arg(*apptr,int);
    addchar(*pos,val);
    break;
  }
  case 'd': {
    int val = va_arg(*apptr,int);
    if (val < 0) {
      val = -val;
      addchar(*pos,'-');
    }
    print_num(str,size,pos,val,10);
    break;
  }
  case 'u':
    print_num(str,size,pos,va_arg(*apptr,unsigned int),10);
    break;
  case 'p':
    addchar(*pos,'0');
    addchar(*pos,'x');
    /* fall through */
  case 'x':
    print_num(str,size,pos,va_arg(*apptr,unsigned int),16);
    break;
  default:
    addchar(*pos,'%');
    break;
  }
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
  size_t pos = 0;
  const char *c = format;
  for (; *c; c++) {
    if (*c == '%') {
      c++;

      int right_aligned = 1;
      int pad = 0;
      va_list start_ap = ap;
      size_t start_pos = pos;

      if ('-' == *c) {
        right_aligned = 0;
        c++;
      }

      for (; ('0' <= *c) && ('9' >= *c); c++)
        pad = pad*10+*c-'0';

      print_field(str,size,&pos,c,&ap); /* Print first time */
      pad -= (pos-start_pos);

      if (right_aligned) {
        pos = start_pos;
        ap = start_ap;
        for (; 0 < pad; pad--) /* Add padding on right */
          addchar(pos,' ');
        print_field(str,size,&pos,c,&ap); /* Print again */
      }
      else {
        for (; 0 < pad; pad--) /* Add padding on left */
          addchar(pos,' ');
      }
    }
    else {
      addchar(pos,*c);
    }
  }
  if (0 < size) {
    if (pos+1 < size)
      str[pos] = '\0';
    else
      str[size-1] = '\0';
  }
  return pos;
}

int snprintf(char *str, size_t size, const char *format, ...)
{
  va_list ap;
  va_start(ap,format);
  int r = vsnprintf(str,size,format,ap);
  va_end(ap);
  return r;
}

int printf(const char *format, ...)
{
  if (!in_user_mode())
    assert(!"printf cannot be called from kernel mode; use kprintf instead!");
  char buf[4096];
  va_list ap;
  va_start(ap,format);
  int len = vsnprintf(buf,4096,format,ap);
  va_end(ap);
  write(STDOUT_FILENO,buf,len);
  return len;
}

int kprintf(const char *format, ...)
{
  if (in_user_mode())
    assert(!"kprintf cannot be called from kernel mode; use printf instead!");
  char buf[4096];
  va_list ap;
  va_start(ap,format);
  int len = vsnprintf(buf,4096,format,ap);
  va_end(ap);
  write_to_screen(buf,len);
  return len;
}

void user_mode_assert(const char *str, const char *function)
{
  printf("Assertion failure in %s: %s\n",function,str);
  exit(1);
}

