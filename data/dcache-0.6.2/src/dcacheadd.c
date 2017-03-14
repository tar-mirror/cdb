/*
 * Copyright (C) 2002 Uwe Ohse, uwe@ohse.de
 * This is free software, licensed under the terms of the GNU General 
 * Public License Version 2, of which a copy is stored at:
 *    http://www.ohse.de/uwe/licenses/GPL-2
 * Later versions may or may not apply, see 
 *    http://www.ohse.de/uwe/licenses/
 * for information after a newer version has been published.
 */
#include <sys/stat.h>
#include "error.h"
#include "buffer.h"
#include "stralloc.h"
#include "dcache.h"
#include "readclose.h"
#include "bailout.h"
#include "open.h"
#include "uogetopt.h"
#include "str.h"
#include "attributes.h"
#include "common.h"

static int opt_sync = 1;
static int opt_delete = 0;
static int opt_transaction = 0;

static uogetopt2 myopts[]={
{'d',"delete",uogo_flag,UOGO_NOARG ,&opt_delete, 1 ,
  "Delete any existing records with that key.",
  "Use this option to ensure that only one record has the\n"
  "new key.\n"
  "May be combined with --transaction.",0},
{'N',"no-sync",uogo_flag,UOGO_NOARG ,&opt_sync, 0 ,
  "Do not fsync cache to disk.",
  "The default is to call fsync, which waits until the\n"
  "data has been written to disk. This is costly but safe.\n"
  "Using this option is not recommended unless speed is the\n"
  "premium.",0},
{'T',"transaction",uogo_flag,UOGO_NOARG ,&opt_transaction, 0 ,
  "Delete and add entries in one transaction.",
  "The default is to do one operation after each other. With\n"
  "this option all changes will be done at once, ensuring that\n"
  "the cache either contains the old or the new state.\n"
  "Note: this costs memory and disk-space." ,0},
COMMON_OPTIONS
{
  0,"see-also",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,
  0,0,
  "Where to find related information.",
  "dcachemake(1) is a fast way to add multiple records to a dcache.\n"
  "dcacheget(1) allows to retrieve records.\n"
  COMMON_RELATED_INFO,0,
},
{
  0,"examples",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,
  0,0,
  "Usage examples.",
  "dcacheadd cache 'passwd' </etc/passwd\n"
  "  Copies the content of /etc/passwd to the cache.\n"
  "echo 1234 | dcacheadd cache abcd\n"
  "  Creates a record with content '1234' and key 'abcd'.\n"
  ,0
},

{0,0,0,0,0,0,0,0,0}
};
static uogetopt_env myenv={
  "dcacheadd",PACKAGE,VERSION,
  "dcacheadd CACHE KEY <DATA",
  "add a record to a cache file",
  "dcacheadd adds a a record to an existing cache file CACHE.\n\n"
  "DATA may contain arbitrary bytes. KEY may contain any character "
  "except the ASCII NUL character (0x00).\n",
  "Report bugs to dcache@lists.ohse.de",
  3,3,0,0,uogetopt_out,myopts
};
dcache c;

static void die_lookup(void) attribute_noreturn;
static void die_lookup(void) 
{ xbailout(111,errno,"failed to read/search in database",0,0,0);}

int main(int argc, char **argv)
{
	int fd;
	struct stat st;
	dcache_data data;
	stralloc sa=STRALLOC_INIT;

	bailout_progname(argv[0]);
	flag_bailout_fatal_begin=3;
	uogetopt_parse(&myenv,&argc,argv);
	if (-1==fstat(0,&st))
		xbailout(111,errno,"failed to fstat stdin",0,0,0);

	fd=open_readwrite(argv[1]);
	if (-1==fd) xbailout(111,errno,"failed to open ",argv[1],0,0);
	if (dcache_lck_excl(fd))
		xbailout(111,errno,"failed to lock cache",0,0,0);
	if (dcache_init(&c,fd,1))
		xbailout(111,errno,"failed to initialize cache",0,0,0);

	if (S_ISREG(st.st_mode)) {
		data.p=0;
		data.next=0;
		data.fd=0;
		data.len=st.st_size;
	} else {
		if (-1==readclose(0,&sa,4096))
			xbailout(111,errno,"failed to read data",0,0,0);
		data.p=sa.s;
		data.next=0;
		data.fd=0;
		data.len=sa.len;
	}
	if (opt_transaction)
		if (-1==dcache_trans_start(&c))
			xbailout(111,errno,"failed to start transaction",0,0,0);
	if (opt_delete) {
                while (1) {
                        int r;
                        dcache_lookupstart(&c);
                        r=dcache_lookupnext(&c,argv[2],str_len(argv[2]));
                        if (-1==r)
                                die_lookup();
                        if (!r)
                                break;
                        dcache_delete(&c);
		}
	}
	if (-1==dcache_enter(&c,argv[2],str_len(argv[2]),&data,0))
		xbailout(111,errno,"failed to enter record",0,0,0);
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
		if (-1==dcache_sync(&c))
			xbailout(100,errno,"failed to sync cache",0,0,0);
	} else {
		if (-1==dcache_flush(&c))
			xbailout(100,errno,"failed to flush cache",0,0,0);
	}

	return (0);
}
