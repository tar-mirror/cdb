/*
 * Copyright (C) 2002 Uwe Ohse, uwe@ohse.de
 * This is free software, licensed under the terms of the GNU General 
 * Public License Version 2, of which a copy is stored at:
 *    http://www.ohse.de/uwe/licenses/GPL-2
 * Later versions may or may not apply, see 
 *    http://www.ohse.de/uwe/licenses/
 * for information after a newer version has been published.
 */
#include "dcache.h"
#include "error.h"
#include "dcachei.h"
#include "fmt.h"
#include "readwrite.h"
#include "buffer.h"
#include "bailout.h"
#include "uogetopt.h"
#include "str.h"
#include "common.h"
#include "tai.h"
#define X(y,z) if (-1==(buffer_put(buffer_1,y,z))) \
	xbailout(111,errno,"failed to write",0,0,0);
#define XNL(y,z) {int ii; \
  for (ii=0;ii<z;ii++) { unsigned char cc=y[ii]; \
    if ((cc&0x7f)<32 || cc=='\\') { \
      char nbx[FMT_ULONG]; fmt_xlong0(nbx,cc,2); \
      X("\\",1); X(nbx,2); \
      continue; \
    } \
    X(y+ii,1); \
  } \
}

dcache c;

static void
hex(dcache_uint32 x)
{
	char nb[FMT_ULONG];
	unsigned int l;
	l=fmt_xlong(nb,x);
	if (l<8)
		X("00000000",8-l);
	X(nb,l);
}
static void
hex64(uo_uint64_t ui64)
{
	char nb[FMT_ULONG];
	unsigned int l;
	unsigned long x;
	x=ui64 >> 32;
	if (x) {
		l=fmt_xlong(nb,x);
		if (l<8)
			X("00000000",8-l);
		X(nb,l);
	}
	x=ui64 & 0xffffffff;
	l=fmt_xlong(nb,x);
	if (l<8)
		X("00000000",8-l);
	X(nb,l);
}

char rbspace[BUFFER_INSIZE];
buffer rb;

static void
copy(int fd, dcache_reclen len)
{
  buffer_init(&rb,(buffer_op)read,fd,rbspace,sizeof(rbspace));

  while (len) {
    char *p;
    int r;
    r=buffer_feed(&rb);
    if (-1==r) xbailout(111,errno,"failed to read",0,0,0);
    if (!r) 
      xbailout(111,errno, "unexpected end of file reading cache",0,0,0);
    p=buffer_peek(&rb);
    if (r>len)
      r=len;
    XNL(p,r);
    buffer_seek(&rb,r);
    len-=r;
  }
}


static int
prth(dcache_uint32 ind, dcache_pos pos, dcache_uint32 hash,
	dcache_time x, int fd,dcache_reclen keylen, dcache_reclen datalen)
{
	char nb[FMT_ULONG];
	unsigned int l;
	/* slotnumber */
	X("#",1);
	l=fmt_ulong(nb,ind);
	X(nb,l);
	X("\n",1);
	/* should-be in slot */
	X("S",1);
	X(nb,fmt_ulong(nb,hash%c.elements));
	X("\n",1);
	/* key */
	X("I",1);
	copy(fd,keylen);
	X("\n",1);
	/* position of record */
	X("P",1);
	hex64(pos);
	X("\n",1);
	/* hash value */
	X("H",1);
	hex(hash);
	X("\n",1);
	/* expire */
	if (x) {
		X("@",1);
		hex64(x);
		X("\n",1);
	}
	/* data length */
	X("L",1);
	l=fmt_ulong(nb,datalen);
	X(nb,l);
	X("\n",1);
	return 0;
}

static int
prt(dcache_uint32 ind, int fd, 
	dcache_time x, dcache_reclen keylen, dcache_reclen datalen)
{
	char nb[FMT_ULONG];
	unsigned int l;
	l=fmt_ulong(nb,ind);
	if (l<5)
		X("          ",5-l);
	X(nb,l);
	if (x) {
		X(" @",2);
		hex64(x);
	} else {
		/* 12345678901234567890 */
		X("                  ",18);
	}
	l=fmt_ulong(nb,datalen);
	if (l<10)
		X("          ",10-l);
	X(nb, l);

	X(" ",1);
	copy(fd,keylen);
	X("\n",1);
	return 0;
}

static int opt_ignore_expire=0;
static int opt_machine=0;
static uogetopt2 myopts[]={
{'i',"ignore-expire",uogo_flag,UOGO_NOARG , &opt_ignore_expire, 1 , 
"ignore expiration date",
"The default is to honor the expiration date and therefore to not list "
"expired records.",0},
{'m',"machine",uogo_flag,UOGO_NOARG , &opt_machine, 1 , 
"machine readable output format.",
"This dumps the hash table.",0},
{  0,"output-format-help", uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD, 
0,1,
"A description of the output format.",
"By default the following information is printed, with one line per record:\n"
"  An index number.\n"
"  The expiration time, if not empty, as tai64n label.\n"
"  The data length.\n"
"  The key.\n"
"Control characters, white space and the backslash character will be "
"backslash-escaped (\\20 stands for a space, \\5c for a backslash).\n\n"
"Machine readable output format is printed as one information per "
"line. The first character on each line is a tag.\n"
"  # The index number.\n"
"  S The slot where the record should be (it may be somewhere else in\n"
"    case of collisions).\n"
"  I The key, followed by a newline. The key is encoded as describe above.\n"
"  P The position of the record in the file.\n"
"  H The hash of the key.\n"
"  @ The expiration time - if any.\n"
"  L The data length.\n"
,0},
{  0,"caveats", uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD, 
0,1,
"Caveats you might want to know about.",
"This command is meant to print an overview of what is in the cache. Use "
"dcachedump(1) instead if the keys might contain non-printable characters.\n"
,0},
{  0,"see-also",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,
  0,0,
  "Where to find related information.",
  "dcachedump(1) dumps a dcache.\n"
  COMMON_RELATED_INFO,0},
{
  0,"examples",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,
  0,0,
  "Usage examples.",
  "dcachelist <dcache\n"
  "  lists the content of dcache.\n"
  "dcachelist -m <dcache\n"
  "  lists the content in a different format.\n"
  "dcachelist -i <dcache\n"
  "  also shows expired records.\n"
  ,0
},


COMMON_OPTIONS
{0,0,0,0,0,0,0,0,0}
};

static uogetopt_env myoptenv={
"dcachelist",PACKAGE,VERSION,
"dcachelist [options] <DCACHE",
"list the records of a cache file.",
"dcachelist lists the record in DCACHE to the standard output.\n",
COMMON_BUGREPORT_INFO,
1,1,0,0,uogetopt_out,myopts
};



int
main(int argc, char **argv) 
{
	struct tai now;
	dcache_time now_dc;
	bailout_progname(argv[0]);
	flag_bailout_fatal_begin=3;
	myoptenv.program=flag_bailout_log_name;
	uogetopt_parse(&myoptenv,&argc,argv);

	if (-1==dcache_lck_share(0))
		xbailout(111,errno,"failed to lock cache",0,0,0);
	if (dcache_init(&c,0,0))
		xbailout(111,errno,"failed to initialize cache",0,0,0);
	if (opt_machine) {
		/* XXX this definitively knows to much about the internals */
		dcache_uint32 i;
		for (i=0;i<c.elements;i++) {
			if (c.ep[i]) {
				dcache_reclen keylen,datalen;
				dcache_uint32 t1,t2;
				dcache_time xp;
				char head[DC_HEADLEN];
				if (-1==dcache_seek(0,c.ep[i]))
					xbailout(111,errno,"failed to seek in cache",0,0,0);
				if (-1==dcache_read(0,head,sizeof(head)))
					xbailout(111,errno,"failed to read from cache",0,0,0);
				dcache_reclen_fromdisk(head+DC_OFF_KEYLEN,&keylen);
				dcache_reclen_fromdisk(head+DC_OFF_DATALEN,&datalen);
				dcache_uint32_fromdisk(head+DC_OFF_EXPIRE,&t1);
				dcache_uint32_fromdisk(head+DC_OFF_EXPIRE2,&t2);
				xp=((dcache_time)t1)<<32 | t2;
				prth(i,c.ep[i],c.hp[i],xp,0,keylen,datalen);
			}
		}

	} else {
		dcache_uint32 i;
		dcache_walkstart(&c);
		tai_now(&now);
		now_dc=tai_approx(&now);
		for (i=0;;i++) {
			int r;
			dcache_time xp;
			r=dcache_walk(&c);
			if (!r) break;
			if (-1==r) 
				xbailout(111,errno,"failed to walk through cache",0,0,0);
			xp=dcache_datatime(&c);
			if (!opt_ignore_expire && xp && xp <now_dc)
				continue; /* already expired */
			prt(i,0, xp,dcache_keylen(&c),dcache_datalen(&c));
		}
	}
	
	if (-1==buffer_flush(buffer_1)) xbailout(111,errno,"failed to write",0,0,0);
	return (0);
}

