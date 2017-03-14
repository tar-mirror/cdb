/*
 * Copyright (C) 2000-2002 Uwe Ohse, uwe@ohse.de
 * This is free software, licensed under the terms of the GNU Lesser
 * General Public License Version 2.1, of which a copy is stored at:
 *    http://www.ohse.de/uwe/licenses/LGPL-2.1
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

static int opt_sync = 1;
static uogetopt2 myopts[]={
{'N',"no-sync",uogo_flag,UOGO_NOARG ,&opt_sync, 0 ,
  "Do not fsync cache to disk.",
  "The default is to call fsync, which waits until the\n"
  "data has been written to disk. This is costly but safe.\n"
  "Using this option is not recommended unless speed is the\n"
  "premium.",0},
COMMON_OPTIONS
{
0,"see-also",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,
0,0,
"Where to find related information.",
"dcachestats(1) shows whether a replay is needed.\n"
COMMON_RELATED_INFO,0,
},
{
0,"examples",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,
0,0,
"Usage examples.",
"dcachereplay cache\n"
"  The only recommended usage.\n"
,0
},
{0,0,0,0,0,0,0,0,0}
};
static struct uogetopt_env myenv={
  "dcachereplay",PACKAGE,VERSION,
  "dcachereplay CACHE",
  "execute committed transactions on a dcache",
  "dcachereplay executes committed, but not yet finished, transactions "
  "on CACHE. It may be called even if there are no unfinished transactions.\n\n"
  "dcachereplay exits with 0 if there was no unfinished transaction or if "
  "it was able to finish it.",
  "Report bugs to dcache@lists.ohse.de",
  2,2,0,0,uogetopt_out,myopts
};
dcache c;

int main(int argc, char **argv)
{
  int fd;
  bailout_progname(argv[0]);
  flag_bailout_fatal_begin=3;
  uogetopt_parse(&myenv,&argc,argv);

  fd=open_readwrite(argv[1]);
  if (-1==fd) xbailout(100,errno,"failed to open ",argv[1],0,0);
  if (dcache_lck_excl(fd))
    xbailout(111,errno,"failed to lock cache",0,0,0);
  if (dcache_init(&c,fd,1))
    xbailout(111,errno,"failed to initialize cache",0,0,0);
  switch(dcache_trans_need_replay(&c)) {
  case 0: break;
  case -1: xbailout(111,errno,"failed to get transaction information",0,0,0);
  default:
    if (-1==dcache_trans_replay(&c))
      xbailout(111,errno,"failed to replay transaction",0,0,0);
    if (opt_sync) {
      if (-1 == dcache_sync (&c))
	xbailout (100, errno, "failed to sync cache", 0, 0, 0);
    } else {
      if (-1 == dcache_flush (&c))
	xbailout (100, errno, "failed to flush cache", 0, 0, 0);
    }
    return 0;
  }
  warning(0,"no open transaction found",0,0,0);
  return 0;
}
