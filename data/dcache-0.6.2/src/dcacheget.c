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
#include "readwrite.h"
#include "buffer.h"
#include "dcache.h"
#include "bailout.h"
#include "uogetopt.h"
#include "str.h"
#include "common.h"

static const char *opt_escape="\\";
static const char *opt_separator="";

static buffer b;
static char bspace[BUFFER_INSIZE];
static void
prt(int fd, dcache_reclen dl)
{
	buffer_init(&b,(buffer_op)read,fd,bspace,sizeof(bspace));
	while (dl) {
		int x;
		char *p;
		x=buffer_feed(&b);
		if (-1==x) xbailout(111,errno,"failed to read from cache",0,0,0);
		if (!x) 
			xbailout(111,0,"unexpected end of file while reading cache",
				0,0,0);
		p=buffer_peek(&b);
		if (x>dl)
			x=dl;
		if (opt_separator && opt_separator[0] && opt_escape && 
			opt_escape[0]) {
			unsigned int i,j;
			for (i=0,j=0;i< (unsigned int) x;i++) {
				if (p[i]==opt_separator[0]) {
					if (i-j) 
						if (-1==buffer_put(buffer_1,p+j,i-j))
							xbailout(111,errno,"failed to write",0,0,0);
					j=i;
					if (-1==buffer_put(buffer_1,opt_escape,str_len(opt_escape)))
						xbailout(111,errno,"failed to write",0,0,0);
				} else if (p[i]==opt_escape[0]) {
					if (i-j) 
						if (-1==buffer_put(buffer_1,p+j,i-j))
							xbailout(111,errno,"failed to write",0,0,0);
					j=i;
					if (-1==buffer_put(buffer_1,opt_escape,str_len(opt_escape)))
						xbailout(111,errno,"failed to write",0,0,0);
				}
			}
			if (i-j) 
				if (-1==buffer_put(buffer_1,p+j,i-j))
					xbailout(111,errno,"failed to write",0,0,0);
		} else {
			if (-1==buffer_put(buffer_1,p,x))
				xbailout(111,errno,"failed to write",0,0,0);
		}
		buffer_seek(&b,x);
		if (opt_separator && opt_separator[0]) {
			if (-1==buffer_put(buffer_1,opt_separator,str_len(opt_separator)))
				xbailout(111,errno,"failed to write",0,0,0);
		}
		dl-=x;
	}
	if (-1==buffer_flush(buffer_1))
		xbailout(111,errno,"failed to write",0,0,0);
}

static unsigned long opt_recno=1;
static int opt_all=0;
static uogetopt2 myopts[]={
{'a',"all",uogo_flag,UOGO_NOARG ,&opt_all, 1 , 
  "Print all records with the same key.",
  "Use the --separator option to select a record separator\n"
  "sequence, and the --escape option to select an escape\n"
  "character.\n",0},
{'e',"escape", uogo_string,0, &opt_escape, 0, 
  "Select a escape character.",
  "The escape character is used to escape the starting\n"
  "character of the separator sequence and defaults to\n"
  "a backslash (\\). If the key contains the escape\n"
  "character it will be duplicated.","CHAR"},
{'n',"record-number",uogo_ulong,0 ,&opt_recno, 1 , 
  "Find the n'th record with the key.",
  "Use this to find all records with the same key.",  "NUMBER"},
{'s',"separator", uogo_string,0, &opt_separator, 0, 
  "Select a sepatator sequence.",
  "The sequence is used to separate multiple records\n"
  "and may be as long as you want.\n"
  "The default sequence is the empty string.\n", "STRING"},
COMMON_OPTIONS
{
0,"see-also",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,
0,0,
"Where to find related information.",
"dcachedump(1) dumps the content of a dcache.\n"
COMMON_RELATED_INFO,0,
},
{
0,"examples",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,
0,0,
"Usage examples.",
"dcacheget '4711' <cache\n"
"  Prints the first record associoated with 4711 to the standard output.\n"
"dcacheget -n 3 '4711' <cache\n"
"  Prints the third record associoated with 4711 to the standard output.\n"
"dcacheget -a '4711' <cache\n"
"  Print all records associoated with 4711 to the standard output.\n"
,0
},
{0,0,0,0,0,0,0,0,0}
};
static struct uogetopt_env myenv={
  "dcacheget",PACKAGE,VERSION,
  "dcacheget KEY <CACHE",
  "print selected records of CACHE to the standard output.",
  0,
  "Report bugs to dcache@lists.ohse.de",
  2,2,0,0,uogetopt_out,myopts
};
dcache c;


int main(int argc, char **argv)
{
	unsigned long recno=0;
	int retcode=1;
	bailout_progname(argv[0]);
	flag_bailout_fatal_begin=3;
	uogetopt_parse(&myenv,&argc,argv);

	if (dcache_lck_share(0))
		xbailout(100,errno,"failed to lock cache",0,0,0);
	if (dcache_init(&c,0,0))
		xbailout(100,errno,"failed to initialize cache",0,0,0);
	dcache_lookupstart(&c);
	while (1) {
		int r;
		r=dcache_lookupnext(&c,argv[1],str_len(argv[1]));
		if (0==r)
			break;
		if (-1==r)
			xbailout(111,errno,"failed to search in cache",0,0,0);
		recno++;
		if (opt_all) {
			prt(dcache_fd(&c), dcache_datalen(&c));
			retcode=0;
		} else if (recno==opt_recno) {
			prt(dcache_fd(&c), dcache_datalen(&c));
			return (0);
		}
	}
	return (retcode);
}
