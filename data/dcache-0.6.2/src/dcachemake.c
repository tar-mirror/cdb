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
#include "buffer.h"
#include "stralloc.h"
#include "dcache.h"
#include "bailout.h"
#include "fmt.h"
#include "uogetopt.h"
#include "str.h"
#include "open.h"
#include "scan.h"
#include "common.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h> /* rename */
#include "attributes.h"

#define TWOGIGMINUSONE (0x7fffffffUL)  

const char *tmpfname;
static int lineno;
static void die_lineno (const char *s, const char *t) attribute_noreturn;
static void die_bad_length(void) attribute_noreturn;
static void die_bad_tai(void) attribute_noreturn;
static void die_unchar(char c) attribute_noreturn;
static void die_eof(void) attribute_noreturn;
static void die_read(void) attribute_noreturn;
static void die_enter(void) attribute_noreturn;

static void die_lineno (const char *s, const char *t)
{
  char nb[FMT_ULONG];
  nb[fmt_ulong(nb,lineno)]=0;
  if (tmpfname)
    unlink(tmpfname);
  xbailout(100,0,s,t," in input line ",nb);
}
static void die_bad_length(void) { die_lineno("","bad length"); }
static void die_bad_tai(void) { die_lineno("","bad expire date"); }
static void die_unchar(char c)
{
  char nb[FMT_ULONG];
  nb[fmt_xlong(nb,(unsigned long)(unsigned char)c)]=0;
  die_lineno("unexpected character 0x",nb);
}

static void die_eof(void)
{ die_lineno("","unexpected end of file"); }
static void die_read(void)
{ int e=errno; if (tmpfname) unlink(tmpfname); 
 xbailout(111,e,"failed to read",0,0,0); }

static void die_enter(void)
{ int e=errno; if (tmpfname) unlink(tmpfname); 
  xbailout(111,e,"failed to enter record. dcache_enter",0,0,0); }

static char 
getcha(void)
{
  int l;
  char c;
  l = buffer_get(buffer_0,&c,1);
  if (-1==l) die_read();
  if ( 0==l) die_eof();
  return c;
}

static char 
getexpect1(const char c1)
{
  char c;
  c=getcha();
  if (c1==c) return c;
  die_unchar(c);
}

static char 
getexpect2(const char c1, const char c2)
{
  char c;
  c=getcha();
  if (c1==c) return c;
  if (c2==c) return c;
  die_unchar(c);
}

static unsigned char unhex[256];
static void init_chartab(void)
{
  unsigned int i;
  for (i=0;i<256;i++)
    unhex[i]=255;
  unhex['0']=0;
  unhex['1']=1;
  unhex['2']=2;
  unhex['3']=3;
  unhex['4']=4;
  unhex['5']=5;
  unhex['6']=6;
  unhex['7']=7;
  unhex['8']=8;
  unhex['9']=9;
  unhex['a']=10; unhex['A']=10;
  unhex['b']=11; unhex['B']=11;
  unhex['c']=12; unhex['C']=12;
  unhex['d']=13; unhex['D']=13;
  unhex['e']=14; unhex['E']=14;
  unhex['f']=15; unhex['F']=15;
}

static unsigned long 
getnum(char *termchar)
{
  unsigned long ul=0;
  static int init_done;
  if (!init_done) {
    init_done=1;
    init_chartab();
  }
  while (1) {
    unsigned char c;
    c=getcha();
    if (255==unhex[c] || unhex[c] > 9)  {
      *termchar=c;
      return ul;
    }
    if (ul > TWOGIGMINUSONE/10)
      die_bad_length();
    ul*=10;
    ul+=unhex[c];
    if (ul > TWOGIGMINUSONE)
      die_bad_length();
  }
}
static dcache_time
gettai(void)
{
  dcache_time t=0;
  unsigned int l=0;
  static int init_done;
  if (!init_done) {
    init_done=1;
    init_chartab();
  }
  while (1) {
    unsigned char c;
    c=getcha();
    if (255==unhex[c])
      die_bad_tai();
    if (16==l)
	die_bad_tai();
    t=16*t;
    t+=unhex[(unsigned char)c];
    l++;
    if (16==l)
      return t;
  }
}

static unsigned long opt_elements=   0;
static unsigned long opt_maxsize =10000000;
static unsigned long opt_max_mb  =0;
static dcache_pos reallength  =0;
static int opt_hole=0;
static int opt_add=0;
static int opt_print_deleted=0;
static int opt_sync = 1;
static int opt_transaction = 0;

static uogetopt2 myopts[]={
{'a',"add",uogo_flag,UOGO_NOARG, &opt_add, 1,
"Add to an existing cache.",
"Do not create a new one. INPUT will be appended to the existing cache CACHE. "
"TMPFILE is not used.",0},
{'d',"print-deleted",uogo_flag,UOGO_NOARG, &opt_print_deleted, 1,
"Print the keys of deleted records.",
"The keys will be formatted as +KEYLEN:KEY\n",0},
{'e',"elements",uogo_ulong,0 , &opt_elements, 0,
"Set the maximum number of records.",
"The hash tables in the cache will be sized to allow operation without too "
"many hash collisions for this number of records, allowing for 50% more "
"records to be used with slightly worse performance.\n"
"If the hash table is full then the oldest record will be deleted.\n"
"Note: this value cannot be changed after the cache creation.\n"
"Default: one slot per 1024 bytes of data.", "NUMBER"},
{'h',"hole",uogo_flag,UOGO_NOARG , &opt_hole, 1,
"Create a cache containing holes.",
"The default is to reserve the space during the creation of the file.\n"
"This option is unsafe as cache corruption may occur if the operating "
"system at the wrong time detects that no free space is left on the device.\n"
 ,0},
{'m',"maxsize",uogo_ulong,0 , &opt_maxsize , 0,
 "Set the maximum cache data size in bytes.",
 "The data space in the cache will be limited to that many bytes. If a new "
 "record is added and doesn't fit into the cache then the oldest records "
 "will be deleted until enough space is free.\n"
 "Use this option for smaller cache sizes, as it is limited to sizes less "
 "than 4 GB.\n"
 "Note: this value cannot be changed after the cache creation.\n"
 "Note, too: if both --maxsize and --max-mb are given then --max-mb is "
 "preferred.\n"
 "Default: 10 MB","NUMBER"},
{'M',"max-mb",uogo_ulong,0 , &opt_max_mb , 0,
 "Set the maximum cache data size in megabytes.",
 "See above.\n"
 "Use this option for caches larger than 4GB.","NUMBER"},
{'N',"no-sync",uogo_flag,UOGO_NOARG ,&opt_sync, 0 ,
  "Do not fsync the cache to disk.",
  "The default is to call fsync, which waits until the data has been "
  "written to disk. This is costly but safe.  Using this option allows "
  "the operating system to schedule the writing to a later time, thus "
  "making it impossible to detect any error due to, for example, a bad "
  "hard disk.\n"
  "The use of this option is not recommended unless speed is more "
  "important than correctness. Note that fsync costs about 5 to 10 "
  "percent of a second.",0},
{'T',"transaction",uogo_flag,UOGO_NOARG ,&opt_transaction, 0 ,
  "Add all new records in one transaction.",
  "This costs a lot of memory and disk space, and makes only a difference "
  "to the normal mode of operation in that it is sure that, if the "
  "operation is stopped in between, the new records are either all "
  "visible or not visible at all.",0},
{  0,"input-format-help", uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD, 
0,1,
"An description of the input format.",
"dcachemake expects the standard input to be a set of records plus an additional "
"newline.\n"
"A record is encoded like this:\n\n"
"  +KEYLEN,DATALEN[@TAIA]:KEY->DATA\n\n"
"followed by a newline character. This format is similar to the format "
"used by cdbmake and cdbdump (http://cr.yp.to/cdb.html), dyndbmake and "
"dyndbdump (http://www.ohse.de/uwe/dyndb.html).\n"
"\n"
" KEYLEN is the length of the KEY, in bytes.\n"
" DATALEN is the length of the DATA, in bytes.\n"
" [@TAIA] is an `at' (@) character followed by tai64 label.\n"
" KEY stands for the key. The key needs to fit into the memory.\n"
" DATA is the data.\n\n"
"The tai64 label is encoded as a stream of 16 hey digits (see "
"http://cr.yp.to/libtai/tai64.html for more information) and is optional. "
"The tools in the package interpret it as an expiry value, meaning that "
"one cannot retrieve the record after that point in time, but this "
"behaviour can be turned off.\n\n"
"An example for valid input:\n"
"  +1,2:a->bc\n"
"  +2,0@400000003b1bc751042d7f04:d->\n"
"  \n"
"Two records, one with two bytes data, no expiry date, the second with "
"expiry date, but without data. The records are followed by an empty line.",0},
{  0,"see-also",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,
  0,0,
"Where to find related information.",
"dcachedump(1) dumps a dcache.\n"
COMMON_RELATED_INFO,0},
COMMON_OPTIONS
{ 
  0,"examples",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,
  0,0,
  "Usage examples.",
  "echo | dcachemake -M 48 dcache\n"
  "  creates an empty dcache with 48 MB size.\n"
  "dcachedump <old-cache | dcachemake -M 1024 new-cache\n"
  "  copies the content of one cache to a new one with 1GB of space.\n"
  ,0
},

{0,0,0,0,0,0,0,0,0}
};
static dcache cache;

static uogetopt_env myoptenv={
"dcachemake",PACKAGE,VERSION,
"dcachemake DCACHE [TMPFILE] <INPUT",
"create a DCACHE from the standard input.",
"dcachemake creates a cache file CACHE.\n\n"
"The default data size is 10 MB, "
"the default number of records is one per 1024 bytes of data size.\n\n"
"TMPFILE is the name of the file during the creation of the cache. TMPFILE "
"will be renamed to CACHE atomically as soon as the input has been read. "
"TMPFILE and CACHE have to be located on the same file system.\n\n"
"If TMPFILE is not given then the cache will be created as CACHE. This "
"might interfere with other programs accessing CACHE during this time.",
COMMON_BUGREPORT_INFO,
2,3,0,0,uogetopt_out,myopts
};


static int delete_cb(dcache *dc)
{
  dcache_pos p;
  dcache_reclen kl;
  char nb[FMT_ULONG+1];
  kl=dcache_keylen(dc);
  p=dcache_keypos(dc);
  if (-1==dcache_seek(dcache_fd(dc),p)) 
    return -1;
  if (-1==buffer_put(buffer_1,"+",1))
    return -1;
  if (-1==buffer_put(buffer_1,nb,fmt_ulong(nb,kl)))
    return -1;
  if (-1==buffer_put(buffer_1,":",1))
    return -1;
  while (kl) {
    int r;
    char b[128];
    dcache_reclen want=sizeof(b);
    if (want > kl) 
      want=kl;
    r=read(dcache_fd(dc),b,want);
    if (r==-1) {
      if (errno==error_intr) continue;
      return -1;
    }
    if (r==0) {
      errno=error_io;
      return -1;
    }
    if (-1==buffer_put(buffer_1,b,r))
      return -1;
    kl-=r;
  }
  if (-1==buffer_put(buffer_1,"\n",1))
    return -1;
  return 0;
}

int 
main(int argc, char **argv)
{
  int fd;
  stralloc buf=STRALLOC_INIT;

  bailout_progname(argv[0]);
  flag_bailout_fatal_begin=3;
  myoptenv.program=flag_bailout_log_name;
  uogetopt_parse(&myoptenv,&argc,argv);
  if (opt_max_mb) {
    reallength=opt_max_mb;
    reallength*=1024;
    reallength*=1024;
  } else 
	  reallength=opt_maxsize;

  if (0==opt_elements)
    opt_elements=reallength/1024;
  if (0==opt_elements)
    opt_elements=2;
  if (argv[2])
    tmpfname=argv[2];
  if (opt_add) {
    fd=open_readwrite(argv[2] ? argv[2] : argv[1]);
    if (-1==fd) 
      xbailout(111,errno,"failed to open cache. open",
	      0,0,0);
  } else {
    if (!tmpfname)
      tmpfname=argv[1];
    fd=dcache_create_name(argv[2]? argv[2] : argv[1],0644, reallength, 
	opt_elements*2, opt_hole);
    if (-1==fd) 
      xbailout(111,errno,"failed to create cache. dcache_create", 0,0,0);
  }
  if (dcache_lck_excl(fd))
    xbailout(111,errno,"failed to lock cache. dcache_lck_excl",0,0,0);
  if (dcache_init(&cache,fd,1))
    xbailout(111,errno,"failed to initialize cache. dcache_init",0,0,0);
  dcache_set_autosync(&cache,0);
  if (opt_print_deleted)
    dcache_set_delete_callback(&cache,delete_cb);
  if (opt_transaction)
    if (-1==dcache_trans_start(&cache))
      xbailout(111,errno,"failed to start transaction",0,0,0);

  lineno=0;
  while (1) {
    int avail;
    int klen,dlen,skip;
    dcache_time x=0; /* keep gcc quiet */
    char gotchar;
    char c;
    char *p;
    dcache_data data[2];
    lineno++;

    c=getexpect2('+','\n');
    if ('\n'==c) break;
    klen=getnum(&gotchar);
    if (gotchar!=',') die_unchar(gotchar);
    dlen=getnum(&gotchar);
    if ('@'==gotchar) {
      x=gettai();
      c=getexpect1(':');
    } else if (':'==gotchar)
      x=0;
    else
      die_unchar(gotchar);
    buf.len=0;

    /* read key */

    while (klen) {
      int l;
      l = buffer_feed(buffer_0);
      if (-1==l) die_read();
      if (0==l) die_eof();
      if (l>klen) l=klen;
      p = buffer_peek(buffer_0);
      if (!stralloc_catb(&buf,p,l)) oom();
      buffer_seek(buffer_0,l);
      klen-=l;
    }

    getexpect1('-');
    getexpect1('>');

    avail = buffer_feed(buffer_0);
    if (-1==avail) die_read();
    if ( 0==avail) die_eof();

    data[0].p=buffer_peek(buffer_0);
    data[0].fd=-1;
    data[0].len = (avail >= dlen) ? dlen : avail;
    skip=data[0].len; 
    dlen-=data[0].len;

    if (dlen) {
      data[0].next=data+1;
      data[1].fd=0;
      data[1].p=0;
      data[1].next=0;
      data[1].len=dlen;
    } else
      data[0].next=0;

    if (-1==dcache_enter(&cache,buf.s,buf.len,data,x))
	    die_enter();
    buffer_seek(buffer_0,skip);
    getexpect1('\n');
  }
  if (opt_transaction) {
    if (-1==dcache_trans_commit(&cache))
      xbailout (100, errno, "failed to commit transaction", 0, 0, 0);
    if (-1==dcache_trans_replay(&cache)) {
      warning (errno, "failed to replay transaction", 0, 0, 0);
      xbailout(111, 0, "use dcachereplay to continue operation",0,0,0);
    }
  }
  if (opt_sync) {
    if (-1 == dcache_sync (&cache))
      xbailout (100, errno, "failed to sync cache", 0, 0, 0);
  } else {
    if (-1 == dcache_flush (&cache))
      xbailout (100, errno, "failed to flush cache", 0, 0, 0);
  }
  if (argv[2]) {
    if (-1==rename(argv[2],argv[1]))
      xbailout(111,errno,"failed to rename ",argv[2], " to ", argv[1]);
  }
  if (-1==buffer_flush(buffer_1))
    xbailout(111,errno,"failed to write to standard output",0,0,0);
  return (0);
}
