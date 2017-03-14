/*
 * Copyright (C) 2001-2002 Uwe Ohse, uwe@ohse.de
 * This is free software, licensed under the terms of the GNU Lesser
 * General Public License Version 2.1, of which a copy is stored at:
 -    http://www.ohse.de/uwe/licenses/LGPL-2.1
 * Later versions may or may not apply, see 
 *    http://www.ohse.de/uwe/licenses/
 * for information after a newer version has been published.
 */
#include "dcache.h"
#include "dcachei.h"
#include "auto-have_ftruncate.h"
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

int dcache_trans_need_replay(dcache *d)
{
  struct stat st;
  if (-1==fstat(d->fd,&st)) return -1;
  if (st.st_size!=d->dataoffset+d->maxsize) {
    char x;
    if (-1==dcache_seek(d->fd,d->dataoffset+d->maxsize)) return -1;
    if (-1==dcache_read(d->fd,&x,1)) return -1;
    if (x)
      return 1;
  }
  return 0;
}

int
dcache_trans_start(dcache *d)
{
  int x;
  x=dcache_trans_need_replay(d);
  if (-1==x) return x;
  if (x) { errno=EBUSY; return -1; }

  if (d->t_buf) {
    errno=EWOULDBLOCK;
    return -1;
  }
  d->t_len=4096;
  d->t_used=0;
  d->t_buf=malloc(d->t_len);
  if (!d->t_buf) {
    errno=ENOMEM;
    return -1;
  }
  return 0;
}
int dcache_trans_cancel(dcache *d)
{
	if (!d->t_buf) {
		errno=EINVAL;
		return -1;
	}
	free(d->t_buf);
	d->t_buf=0;
	return 0;
}
int dcache_trans_commit(dcache *d)
{
  if (!d->t_buf) {
    errno=EINVAL;
    return -1;
  }
  if (d->t_used) {
    if (-1==dcache_seek(d->fd,d->dataoffset+d->maxsize))
      return -1;

    /* this is not needed, we could seek one byte ahead */
    /* but make sure this doesn't look like a valid log! */
    if (-1==dcache_write(d->fd,"",1)) return -1;

    /* write almost all */
    if (-1==dcache_write(d->fd,d->t_buf+1,d->t_used-1)) return -1;

    /* mark end of log */
    if (-1==dcache_write(d->fd,"",1)) return -1;

    /* write the first byte */
    if (-1==dcache_seek(d->fd,d->dataoffset+d->maxsize)) return -1;
    if (-1==dcache_write(d->fd,d->t_buf,1)) return -1;
  }

  if (-1==dcache_trans_cancel(d)) return -1;
  return 0;
}
static int dcache_trans_need(dcache *d, dcache_uint32 need)
{
	char *x;
	if (d->t_len-d->t_used>=need)
		return 0;
	need=d->t_len+((need/4096)+1)*4096;
	x=realloc(d->t_buf,need);
	if (!x) {
		errno=ENOMEM;
		return -1;
	}
	d->t_buf=x;
	d->t_len=need;
	return 0;

}
int dcache_trans_enter(dcache *d,const void *key, dcache_reclen keylen,
        dcache_data *data, dcache_time timevalue)
{
	dcache_uint32 need;
	dcache_reclen datalen;
	dcache_data *p;
	if (!d->t_buf) {
		errno=EINVAL;
		return -1;
	}
	datalen=0;
	for (p=data;p;p=p->next)
		datalen+=p->len;

	/* E+timevalue+keylen+key+datalen+data */
	need=1+8+keylen+8+datalen+8; /* safe until reclen needs >8 bytes */
	if (-1==dcache_trans_need(d,need))
		return -1;
	d->t_buf[d->t_used++]='E'; /* todo enter */
	d->t_used+=dcache_pos_todisk(d->t_buf+d->t_used,timevalue);
	d->t_used+=dcache_reclen_todisk(d->t_buf+d->t_used,keylen);
	memcpy(d->t_buf+d->t_used,key,keylen);
	d->t_used+=keylen;
	d->t_used+=dcache_reclen_todisk(d->t_buf+d->t_used,datalen);
	for (p=data;p;p=p->next) {
	  if (p->p)
	    memcpy(d->t_buf+d->t_used,p->p,p->len);
	  else
	    if (-1==dcache_read(d->fd,d->t_buf+d->t_used,p->len))
	      return -1;
	    d->t_used+=p->len;
	}
	return 0;
}
int dcache_trans_delete(dcache *d, const void *key, dcache_reclen keylen,
        dcache_pos pos)
{
	dcache_uint32 need;
	if (!d->t_buf) {
		errno=EINVAL;
		return -1;
	}
	need=1+8+8+keylen; /* 4 bytes to many, in case reclen grows */
	if (-1==dcache_trans_need(d,need))
		return -1;
	d->t_buf[d->t_used++]='D'; /* todo delete */
	d->t_used+=dcache_pos_todisk(d->t_buf+d->t_used,pos);
	d->t_used+=dcache_reclen_todisk(d->t_buf+d->t_used,keylen);
	memcpy(d->t_buf+d->t_used,key,keylen);
	d->t_used+=keylen;
	return 0;
}

static int
do_delete(dcache *d, char mode, dcache_pos *pos)
{
  char nb1[sizeof(dcache_pos)];
  char nb2[sizeof(dcache_reclen)];
  dcache_pos dpos;
  dcache_reclen kl;
  dcache_uint32 used=1; /* mode char */
  char *k;

  if (-1==dcache_read(d->fd,nb1,sizeof(nb1)))
    return -1;
  used+=dcache_pos_fromdisk(nb1,&dpos);
  
  if (-1==dcache_read(d->fd,nb2,sizeof(nb2)))
    return -1;
  used+=dcache_reclen_fromdisk(nb2,&kl);
  used+=kl;

  if (mode=='d') {
    /* this transaction was already done */
    *pos+=used;
    return 0;
  }

  /* read key */
  k=malloc(kl);
  if (!k) { errno=ENOMEM; return -1; }
  if (-1==dcache_read(d->fd,k,kl)) goto error_return;

  /* find the record for 'key' and 'pos' */
  dcache_lookupstart(d);
  while (1) {
    int x;
    x=dcache_lookupnext(d,k,kl);
    if (x==-1) goto error_return;
    if (!x)
      break;
    if (d->datapos-DC_HEADLEN-kl==dpos) {
      if (-1==dcache_delete(d)) goto error_return;
      break;
    }
  }

  if (-1==dcache_seek(d->fd,*pos)) goto error_return;
  if (-1==dcache_write(d->fd,"d",1)) goto error_return;
  *pos+=used;
  free(k);
  return 0;
error_return:
  {
    int e=errno;
    free(k);
    errno=e;
    return -1;
  }
}

static int
do_enter(dcache *d, char mode, dcache_pos *pos)
{
  char nb_reclen[sizeof(dcache_reclen)];
  char nb_timevalue[sizeof(dcache_time)];
  dcache_reclen kl;
  dcache_reclen dl;
  dcache_time timevalue;
  dcache_uint32 used=1; /* mode char */
  dcache_data data;
  char *kp=0;
  char *dp=0;
  int  dupfd=-1;

  /* E timeval keylen key datalen data */

  /* read timeval */
  if (-1==dcache_read(d->fd,nb_timevalue,sizeof(nb_timevalue)))
    return -1;
  used+=dcache_pos_fromdisk(nb_timevalue,&timevalue);

  /* read keylen */
  if (-1==dcache_read(d->fd,nb_reclen,sizeof(nb_reclen)))
    return -1;
  used+=dcache_reclen_fromdisk(nb_reclen,&kl);

  /* read key */
  kp=malloc(kl);
  if (!kp) { errno=ENOMEM; return -1; }
  if (-1==dcache_read(d->fd,kp,kl))
    goto error_return;
  used+=kl;

  /* read datalen */
  if (-1==dcache_read(d->fd,nb_reclen,sizeof(nb_reclen)))
    goto error_return;
  used+=dcache_reclen_fromdisk(nb_reclen,&dl);
  used+=dl;

  if (mode=='E') { /* otherwise this transaction was already done */
    data.next=0;
    data.len=dl;
    data.p=0;
    if (dl<8192) {
      dp=malloc(dl);
      if (dp) {
	if (-1==dcache_read(d->fd,dp,dl))
	  goto error_return;
	data.p=dp;
      }
    }
    if (!data.p) {
      dupfd=dup(d->fd);
      if (-1==dupfd)
	goto error_return;
      data.fd=dupfd;
    }
    if (-1==dcache_enter(d,kp,kl,&data,timevalue)) goto error_return;
    if (-1==dcache_seek(d->fd,*pos)) goto error_return;
    if (-1==dcache_write(d->fd,"e",1)) goto error_return;
  }
  *pos+=used;
  if (kp) free(kp);
  if (dp) free(dp);
  if (-1!=dupfd) close(dupfd);
  return 0;

error_return:
  {
    int e=errno;
    if (kp) free(kp);
    if (dp) free(dp);
    if (-1!=dupfd) close(dupfd);
    errno=e;
    return -1;
  }
}

int 
dcache_trans_replay(dcache *d)
{
  struct stat st;
  dcache_pos pos;
  if (d->t_buf) {
    errno=EINVAL;
    return -1;
  }
  if (-1==fstat(d->fd,&st)) return -1;
  pos=d->dataoffset+d->maxsize;
  while (1) {
    dcache_reclen r;
    char mode;
    if (-1==dcache_seek(d->fd,pos))
      return -1;
    r=dcache_read(d->fd,&mode,1);
    if (-1==r) {
      /* special case EOF: means no log */
      if (pos==st.st_size && errno==EIO)
        return 0;
      return -1;
    }
    if (!mode)
      break;
    switch(mode) {
    case 'e':
    case 'E':
      if (-1==do_enter(d,mode,&pos))
	return -1;
      break;
    case 'd':
    case 'D':
      if (-1==do_delete(d,mode,&pos))
	return -1;
    break;
    }
  }
  if (-1==dcache_seek(d->fd,d->dataoffset+d->maxsize))
    return -1;
  if (-1==dcache_write(d->fd,"",1))
    return -1;
#ifdef HAVE_FTRUNCATE
  ftruncate(d->fd,d->dataoffset+d->maxsize);
#endif
  return 0;
}
