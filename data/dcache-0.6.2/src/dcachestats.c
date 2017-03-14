/*
 * Copyright (C) 2002 Uwe Ohse, uwe@ohse.de
 * This is free software, licensed under the terms of the GNU General 
 * Public License Version 2, of which a copy is stored at:
 *    http://www.ohse.de/uwe/licenses/GPL-2
 * Later versions may or may not apply, see 
 *    http://www.ohse.de/uwe/licenses/
 * for information after a newer version has been published.
 */
#include "attributes.h"
#include "error.h"
#include "dcache.h"
#include "dcachei.h"
#include "bailout.h"
#include "buffer.h"
#include "uogetopt.h"
#include "fmt.h"
#include "common.h"

static int opt_nolock;
static uogetopt2 myopts[]={
{0,"nolock",uogo_flag,UOGO_NOARG, &opt_nolock, 1 , "do not lock cache.",
  "It's not safe to rely on the values read.",0},
COMMON_OPTIONS
{ 
0,"see-also",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,
0,0,
"Where to find related information.",
"dcachelist(1) lists the content of a dcache.\n"
COMMON_RELATED_INFO,0,
},
{ 
0,"examples",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,
0,0,
"Usage examples.",
"dcachestats <cache\n"
"  Shows the statistics about 'cache'.\n"
,0
},
{0,"output-format",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,0,0,
"Output format.",
"The tool prints:\n\n"
"- version\n"
"  Cache format version.\n"
"- slots\n"
"  The absolute number of slots in the hash area.\n"
"- slotsinuse\n"
"  The number of slots in use.\n"
"- slotuselimit\n"
"  The maximum number of slots used. `slots - slotuselimit' slots will be\n"
"  left free.\n"
"- datalimit\n"
"  The maximum size of the data in the data area.\n"
"- writepos\n"
"  New records will be inserted here.\n"
"- deletepos\n"
"  If space in data or hash area is needed then the record at this\n"
"  position will be deleted.\n"
"- lastpos\n"
"  Position of the byte after the highest record in the data area.\n"
"- datainuse\n"
"  Approximation of the bytes used by data records. This does not account\n"
"  deleted records as one would expect.\n"
"- inserted\n"
"  The number of insertions done.\n"
"- deleted\n"
"  The number of deletions done.\n"
"- slot-moves\n"
"  Copy operations needed to restore the collision chains after a record\n"
"  has been deleted. A `high' number is bad and means that the hash\n"
"  function is inappropriate.\n"
"- need-replay\n"
"  Shows whether there are unfinished transactions: 'yes' or 'no'.\n"
,0},
{0,0,0,0,0,0,0,0,0}
};

struct uogetopt_env myenv={
  "dcachestats",PACKAGE,VERSION,"dcachestats <DCACHE",
  "print statistics about a dcache file",
  "dcachestats prints statistics about a cache file read from the "
  "standard input",
  "Report bugs to dcache@lists.ohse.de",
  0,1,0,0,uogetopt_out,myopts
};

static dcache c;
static void die_write_stdout(void) attribute_noreturn;

static void die_write_stdout(void)
{ xbailout(100,errno,"failed to write to stdout",0,0,0); }

static void
out_num(dcache_pos x)
{
	char spc[128];
	unsigned int l=0;
	dcache_pos y=x;
	do {
		l++;
		y/=10;
	} while (y);
	spc[l]=0;
	while (l) {
		spc[--l]="0123456789"[x % 10];
		x/=10;
	}
	if (-1==buffer_puts(buffer_1, spc)) die_write_stdout();
}
static void
out(const char *s)
{
	if (-1==buffer_puts(buffer_1, s)) die_write_stdout();
}

int
main(int argc, char **argv) 
{	
	dcache_pos v;
	dcache_uint32 ui;
	bailout_progname(argv[0]);
	flag_bailout_fatal_begin=3;
	uogetopt_parse(&myenv,&argc,argv);
	if (!opt_nolock && dcache_lck_share(0))
		xbailout(111,errno,"failed to lock cache",0,0,0);
	if (dcache_init(&c,0,0)) 
		xbailout(111,errno,"failed to initialize cache",0,0,0);
	out("dcachestats:\n");
	dcache_uint32_fromdisk(((char *)c.mapped)+DCACHE_OFF_VERSION,&ui);
	out("version:       "); out_num(ui);
	out("\nslots:         "); out_num(c.elements);
	out("\nslotsinuse:    "); out_num(c.inuse);
	out("\nslotuselimit:  "); out_num(c.slotuselimit);
	out("\ndatalimit:     "); out_num(c.maxsize);
	out("\nwritepos:      "); out_num(c.n);
	out("\ndeletepos:     "); out_num(c.o);
	out("\nlastpos:       "); out_num(c.l);
	if (c.n==c.o) {
		if (!c.inuse)
			v=0;
		else
			v=c.maxsize;
	} else if (c.o < c.n)
		v=c.n - c.o;
	else
		v=c.maxsize - c.o +c.n;
	out("\ndatainuse:     "); out_num(v);
	out("\ninserted:      "); out_num(c.inserted);
	out("\ndeleted:       "); out_num(c.deleted);
	out("\nslot-moves:    "); out_num(c.moves);
	out("\nneed-replay:   "); 
	if (dcache_trans_need_replay(&c))
	  out("yes");
	else
	  out("no");
	out("\n");
	if (-1==buffer_flush(buffer_1)) die_write_stdout();

	return (0);
}

