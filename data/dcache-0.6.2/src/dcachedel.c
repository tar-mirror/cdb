/*
 * Copyright (C) 2002 Uwe Ohse, uwe@ohse.de
 * This is free software, licensed under the terms of the GNU General 
 * Public License Version 2, of which a copy is stored at:
 *    http://www.ohse.de/uwe/licenses/GPL-2
 * Later versions may or may not apply, see 
 *    http://www.ohse.de/uwe/licenses/
 * for information after a newer version has been published.
 */
#include "error.h"
#include "attributes.h"
#include "dcache.h"
#include "bailout.h"
#include "open.h"
#include "uogetopt.h"
#include "str.h"
#include "common.h"

unsigned long opt_recno=1;
int opt_all;
static int opt_sync = 1;
static int opt_transaction = 0;

static uogetopt2 myopts[]={
{'a',"all",uogo_flag,UOGO_NOARG ,&opt_all, 1 ,
  "Delete all records with the same key.",
  "The default is to delete the first record\n"
  "found.",0},
{'n',"record-number",uogo_ulong,0,&opt_recno, 0 ,
 "Delete the n'th record with the key.",0,"NUMBER"},
{'N',"no-sync",uogo_flag,UOGO_NOARG ,&opt_sync, 0 ,
  "Do not fsync cache to disk.",
"The default is to call fsync, which waits until the "
"data has been written to disk. This is costly but safe. "
"Using this option allows the operating system to schedule the "
"writting to a later time, thus making it impossible to detect "
"any error due to, for example, a bad hard disk.\n"
"The use of this option is not recommended unless speed is more "
"important than correctness. Note that fsync costs about 5 to 10 "
"percent of a second.",0},
{'T',"transaction",uogo_flag,UOGO_NOARG ,&opt_transaction, 0 ,
  "Delete entries in one transaction.",
  "The default is to do one operation after each other. With\n"
  "this option all changes will be done at once, ensuring that\n"
  "the cache either contains the old or the new state.\n"
  "Note: this costs memory and disk-space." ,0},
COMMON_OPTIONS
{
  0,"see-also",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,
  0,0,
  "Where to find related information.",
  "dcacheadd(1) adds data to a dcache.\n"
  "dcachelist(1) lists the content of a dcache.\n"
  COMMON_RELATED_INFO,0,
},
{
  0,"examples",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,
  0,0,
  "Usage examples.",
  "dcachedel cache 'funny key'\n"
  "  deletes the first record with the key 'funny key'.\n"
  "dcachedel -T -a cache 'funny'\n"
  "  removes all traces of 'funny' in one transaction.\n"
  ,0
},
{0,0,0,0,0,0,0,0,0}
};
static uogetopt_env myenv={
  "dcachedel",PACKAGE,VERSION,
  "dcachedel [OPTIONS] CACHE KEY",
  "dcachedel - delete records from a cache file.",
  "dcachedel by default deletes the first record found by a search for "
  "KEY in CACHE.",
  COMMON_BUGREPORT_INFO,
  3,3,0,0,uogetopt_out,myopts
};

dcache c;
static void die_lookup(void) attribute_noreturn;
static void die_lookup(void) 
{ xbailout(111,errno,"failed read/search in database",0,0,0);}

int main(int argc, char **argv)
{
	int fd;
	int retcode=1;
	bailout_progname(argv[0]);
	flag_bailout_fatal_begin=3;
	uogetopt_parse(&myenv,&argc,argv);

	fd=open_readwrite(argv[1]);
	if (-1==fd) xbailout(100,errno,"failed to open ",argv[1],0,0);
	if (dcache_lck_excl(fd))
		xbailout(111,errno,"failed to lock cache",0,0,0);
	if (dcache_init(&c,fd,1))
		xbailout(111,errno,"failed to initialize cache",0,0,0);
        if (opt_transaction)
		if (-1==dcache_trans_start(&c))
			xbailout(111,errno,"failed to start transaction",0,0,0);
	if (opt_all) {
		while (1) {
			int r;
			dcache_lookupstart(&c);
			r=dcache_lookupnext(&c,argv[2],str_len(argv[2]));
			if (-1==r)
				die_lookup();
			if (!r)
				break;
			dcache_delete(&c);
			retcode=0;
		}
	} else {
		unsigned long recno=0;
		dcache_lookupstart(&c);
		while (1) {
			int r;
			r=dcache_lookupnext(&c,argv[2],str_len(argv[2]));
			if (-1==r)
				die_lookup();
			if (!r)
				break;
			recno++;
			if (recno==opt_recno) {
				dcache_delete(&c);
				retcode=0;
			}
		}
	}
        if (opt_transaction) {
                if (-1==dcache_trans_commit(&c))
                        xbailout (100, errno, "failed to commit transaction",
                          0, 0, 0);
                if (-1==dcache_trans_replay(&c)) {
                        warning (errno, "failed to replay transaction", 
                          0, 0, 0);
                        xbailout(111, 0, 
                          "use dcachereplay to continue operation",0,0,0);
                }
        }

	if (opt_sync) {
		if (-1 == dcache_sync (&c))
			xbailout (100, errno, "failed to sync cache", 0, 0, 0);
	} else {
		if (-1 == dcache_flush (&c))
			xbailout (100, errno, "failed to flush cache", 0, 0, 0);
	}

	return (retcode);
}
