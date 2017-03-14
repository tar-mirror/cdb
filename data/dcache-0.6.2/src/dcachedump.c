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
#include "fmt.h"
#include "error.h"
#include "readwrite.h"
#include "buffer.h"
#include "bailout.h"
#include "uogetopt.h"
#include "str.h"
#include "common.h"
#include "tai.h"
#define X(y,z) do {if (-1==(buffer_put(buffer_1,y,z))) \
	xbailout(111,errno,"failed to write",0,0,0); } while(0)

static int opt_n=0;
static int opt_ignore_expire=0;
static int opt_no_expire_date=0;

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

char rbspace[BUFFER_INSIZE];
buffer rb;

static void
copy(dcache_reclen len)
{
	while (len) {
			char *p;
			int r;
			r=buffer_feed(&rb);
			if (-1==r) xbailout(111,errno,"failed to read",0,0,0);
			if (!r) 
				xbailout(111,errno,
					"unexpected end of file reading cache",0,0,0);
			p=buffer_peek(&rb);
			if (r>len)
					r=len;
			X(p,r);
			buffer_seek(&rb,r);
			len-=r;
	}
}

static int
prt(dcache_time x, int fd, dcache_reclen keylen, dcache_reclen datalen)
{
	char nb[FMT_ULONG];
	buffer_init(&rb,(buffer_op)read,fd,rbspace,sizeof(rbspace));

	X("+",1);
	X(nb, fmt_ulong(nb,keylen));
	X(",",1);
	X(nb, fmt_ulong(nb,datalen));
	if (x && !opt_no_expire_date) {
		uo_uint32_t y,z;
		X("@",1);
		z=x & 0xffffffff;
		y=x >> 32;
		hex(y);
		hex(z);
	}
	X(":",1);
	copy(keylen);
	X("->",2);
	copy(datalen);
	X("\n",1);
	return 0;
}

static uogetopt2 myopts[]={
{'i',"ignore-expire",uogo_flag,UOGO_NOARG , &opt_ignore_expire, 1 , 
"Ignore  expiration  dates, if any.",
"The default is to not dump expired records.",0},
{'n',"-no-newline",uogo_flag,UOGO_NOARG , &opt_n, 1 , 
"Don't print the final newline.",
"A final newline character is needed for dcachemake.",0},
{  0,"-no-expire-date",uogo_flag,UOGO_NOARG , &opt_no_expire_date, 1 , 
"Do not print expiration dates.",
"The output format will be compatible with that of cdb if you use this option."
,0},
{  0,"output-format-help", uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD, 
0,1,
"A description of the output format.",
"Each record is printed as follows:\n"
"  +KL,DL:KEY->DATA NL\n"
"or\n"
"  +KL,DL@EXP:KEY->DATA NL\n"
"where the symbols have this meaning:\n" 
"  +     A `plus' character.\n"
"  KL    The length of KEY, as a series of decimal digits.\n"
"  ,     A comma.\n"
"  DL    The length of DATA, as a series of decimal digits.\n"
"  @     An `at' character. This is only printed if followed by EXPIRE.\n"
"  EXP   An expiration date, if the record has one.\n"
"  :     A colon.\n"
"  KEY   The key, as a series of 8bit characters.\n"
"  ->    A minus sign followed by a `larger' sign.\n"
"  DATA  The data, as a series of 8bit characters.\n"
"  NL    A newline character (\\n, 0x0a).\n"
"EXPIRE is not printed if the --no-expire-data option is in use.\n"
"By default another NEWLINE is printed after the last record.\n"
"Note that there is no space between DATA and NL.\n",0},
{  0,"see-also",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,
  0,0,
"Where to find related information.",
"dcachemake(1) creates a dcache.\n"
COMMON_RELATED_INFO,0},
COMMON_OPTIONS
{ 
  0,"examples",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,
  0,0,
  "Usage examples.",
  "dcachedump -n <cache >file\n"
  "  dumps the content of cache to file.\n"
  "dcachedump <old-cache | dcachemake -M 1024 new-cache\n"
  "  copies the content of one cache to a new one with 1GB of space.\n"
  "(dcachedump -n <cache1 ; dcachedump <cache2 ) | dcachemake -a cache3\n"
  "  adds the content of cache1 and cache2 to cache3.\n"
  ,0
},
{0,0,0,0,0,0,0,0,0}
};

static uogetopt_env myoptenv={
"dcachedump",PACKAGE,VERSION,
"dcachedump [options] <DCACHE",
"dump a dcache to the standard output.",
"dcachedump dumps the content of CACHE to the standard output.\n",
COMMON_BUGREPORT_INFO,
1,1,0,0,uogetopt_out,myopts
};


dcache c;
int
main(int argc, char **argv) 
{
	struct tai now;
	dcache_time now_dc;
	bailout_progname(argv[0]);
	flag_bailout_fatal_begin=3;
  bailout_progname(argv[0]);
  flag_bailout_fatal_begin=3;
  myoptenv.program=flag_bailout_log_name;
  uogetopt_parse(&myoptenv,&argc,argv);

	if (-1==dcache_lck_share(0))
		xbailout(111,errno,"failed to lock cache",0,0,0);
	if (dcache_init(&c,0,0))
		xbailout(111,errno,"failed to initialize cache",0,0,0);
	dcache_walkstart(&c);
	tai_now(&now);
	now_dc=tai_approx(&now);
	while (1) {
		int r;
		dcache_time xp;
		r=dcache_walk(&c);
		if (0==r)
			break;
		if (-1==r)
			xbailout(111,errno,"failed to dump cache",0,0,0);

		xp=dcache_datatime(&c);
		if (!opt_ignore_expire) {
			if (xp && xp <now_dc)
				continue; /* already expired */
		}
		prt(xp,dcache_fd(&c),dcache_keylen(&c),dcache_datalen(&c));
	}
	if (!opt_n)
		X("\n",1);
	
	if (-1==buffer_flush(buffer_1)) xbailout(111,errno,"failed to write",0,0,0);
	return (0);
}

