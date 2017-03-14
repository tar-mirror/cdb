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
#include "dcache.h"
#include "bailout.h"
#include "uogetopt.h"
#include "getln.h"
#include "buffer.h"
#include "str.h"
#include "open.h"
#include "scan.h"
#include "fmt.h"
#include "buffer.h"
#include "close.h"
#include "attributes.h"
#include "common.h"
#include <stdlib.h>
#include <unistd.h>


static uogetopt2 myopts[] = {
  { 
  0,"examples",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,
  0,0,
  "Usage examples.",
  " openwrite t.db\n"
  " lockexcl\n"
  " start\n"
  " add key1 value1\n"
  " add key2 val2\n"
  " echo ---- try to find these entries before the commit (XFAIL)\n"
  " findall key1\n"
  " findall key2\n"
  " commit\n"
  " echo ---- try to find them after the commit, before the replay (XFAIL)\n"
  " findall key1\n"
  " findall key2\n"
  " replay\n"
  " sync\n"
  " echo ---- try to find them after the replay\n"
  " findall key1\n"
  " findall key2\n"
  ,0
  },
  { 
  0,"commands",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,
  0,0,
  "Show available Commands.",
  ""
  ,0
  },
  {  0,"see-also",uogo_print_help,UOGO_NOARG|UOGO_EXIT|UOGO_NOLHD,
    0,0,
    "Where to find related information.",
    COMMON_RELATED_INFO,0},
    COMMON_OPTIONS
  {0,0,0, 0, 0, 0, 0, 0, 0}
};
static uogetopt_env myenv={
"dcachesh",PACKAGE,VERSION,
"dcachesh",
"dcachesh is the dcache shell and allows for interactive dcache "
"manipulations.",
  0,
  "Report bugs to dcache@lists.ohse.de",
  1,1,0,0,uogetopt_out,myopts
};

dcache c;
stralloc cmd;
int in_transaction;
#define ISOPEN (c.fd!=-1)

#define CMDWRAP_RETURN(x,err) \
do { int ec=x; \
if (ec) warning(errno,err,0,0,0); \
else ok(); \
return ec; \
} while(0)
#define CMDWRAP_RETURN_ON_ERR(x,err) \
do { int ec=x; \
if (ec) warning(errno,err,0,0,0); \
else ok(); \
if (ec) return ec; \
} while(0)
static void ok(void) { if (isatty(0)) warning(0,"ok",0,0,0);}

static unsigned int out_pos;
static void out(const char *s, unsigned int l)
{
  if (-1==buffer_put(buffer_1,s,l)) 
    xbailout(111,errno,"failed to write to terminal",0,0,0);
  out_pos+=l;
}
static void outs(const char *s)
{
  unsigned int x=str_len(s);
  out(s,x);
}
static void outpos(dcache_pos u)
{
  char nb[FMT_ULONG];
  outs("0x");
  out(nb,fmt_xlong0(nb,u>>32,8));
  out(nb,fmt_xlong0(nb,u &0xffffffff,8));
}
static void outuintfill(unsigned int u, unsigned int len)
{
  unsigned int x;
  char nb[FMT_ULONG];
  x=fmt_ulong(nb,u);
  while (x++<len)
   outs(" ");
  out(nb,fmt_ulong(nb,u));
}
static void outreclenfill(unsigned int u, unsigned int len)
{
  outuintfill(u,len);
}
static void outln(void)
{
  if (-1==buffer_putflush(buffer_1,"\n",1l)) 
    xbailout(111,errno,"failed to write to terminal",0,0,0);
  out_pos=0;
}
static void outflush(void)
{
  if (-1==buffer_flush(buffer_1)) 
    xbailout(111,errno,"failed to write to terminal",0,0,0);
}

static int 
callback_fail(dcache *d)
{
  (void) d;
  warning(errno,"callback_fail now fails",0,0,0);
  return -1;
}

static int 
callback_print(dcache *d)
{
  dcache_pos kp,dp;
  dcache_reclen kl;
  char *k;

  kl=dcache_keylen(d);
  kp=dcache_keypos(d);
  dp=dcache_datapos(d);
  if (-1==dcache_seek(d->fd,kp)) {
    warning(errno,"callback_print failed to seek in database",0,0,0);
    return -1;
  }
  k=malloc(kl);
  if (!k) {
    warning(errno,"callback_print failed to allocate memory",0,0,0);
    return -1;
  }
  if (-1==dcache_read(d->fd,k,kl)) {
    warning(errno,"callback_print failed to read in database",0,0,0);
    free(k);
    return -1;
  }
  outs("dcache will delete ");
  out(k,kl);
  outs(" at ");
  outpos(dp);
  outln();
  free(k);
  return 0;
}
static int 
cmd_callback(char *s)
{
  if (str_equal(s,"none")) dcache_set_delete_callback(&c,0);
  else if (str_equal(s,"print")) dcache_set_delete_callback(&c,callback_print);
  else if (str_equal(s,"fail")) dcache_set_delete_callback(&c,callback_fail);
  else {
    warning(0,"usage: callback [none|print|fail]",0,0,0);
    return -1;
  }
  return 0;
}


static int cmd_quit(void)
{
  if (ISOPEN) { warning(0,"cache still is open",0,0,0); return -1; }
  xbailout(0,0,"bye.",0,0,0);
}
static int cmd_forcequit(void) attribute_noreturn;
static int cmd_forcequit(void)
{
  if (ISOPEN) warning(0,"cache still is open",0,0,0);
  xbailout(0,0,"bye.",0,0,0);
}

static int cmd_lock_shared(void)
{ CMDWRAP_RETURN(dcache_lck_share(c.fd),"failed to lock cache"); }
static int cmd_lock_excl(void)
{ CMDWRAP_RETURN(dcache_lck_excl(c.fd),"failed to lock cache"); }
static int cmd_unlock(void)
{ CMDWRAP_RETURN(dcache_lck_unlock(c.fd),"failed to unlock cache"); }
static int cmd_openread(char *s)
{	
  int fd;
  if (ISOPEN) {
    warning(0,"a cache is already open",0,0,0);
    return -1;
  }
  fd=open_read(s);
  if (-1==fd) { warning(errno,"failed to open ",s,0,0); return -1; }
  CMDWRAP_RETURN(dcache_init(&c,fd,0),"failed to initialize cache");
}
static int cmd_openwrite(char *s)
{	
  int fd;
  if (ISOPEN) {
    warning(0,"a cache is already open",0,0,0);
    return -1;
  }
  fd=open_readwrite(s);
  if (-1==fd) { warning(errno,"failed to open ",s,0,0); return -1; }
  CMDWRAP_RETURN(dcache_init(&c,fd,1),"failed to initialize cache");
}
static int cmd_trans_start(void)
{
#if 0
  if (in_transaction) {
    warning(0,"already in transaction",0,0,0); 
    return -1;
  }
#endif
  if (-1==dcache_trans_start(&c)) {
    warning(errno,"failed to start transaction",0,0,0); 
    return -1;
  }
  ok();
  in_transaction=1;
  return 0;
}
static int cmd_trans_cancel(void)
{
#if 0
  if (!in_transaction) {
    warning(0,"not in transaction",0,0,0); 
    return -1;
  }
#endif
  if (-1==dcache_trans_cancel(&c)) {
    warning(errno,"failed to cancel transaction",0,0,0); 
    return -1;
  }
  in_transaction=0;
  ok();
  return 0;
}
static int cmd_trans_commit(void)
{
#if 0
  if (!in_transaction) {
    warning(0,"not in transaction",0,0,0); 
    return -1;
  }
#endif
  if (-1==dcache_trans_commit(&c)) {
    warning(errno,"failed to commit transaction",0,0,0); 
    return -1;
  }
  ok();
  in_transaction=0;
  return 0;
}
static int cmd_trans_replay(void)
{
  if (in_transaction) {
    warning(0,"first commit, then replay",0,0,0);
    return -1;
  }
  if (!dcache_trans_need_replay(&c)) {
    warning(0,"no transaction needs replay",0,0,0);
    return -1;
  }
  if (-1==dcache_trans_replay(&c)) {
    warning(errno,"failed to replay transaction",0,0,0); 
    return -1;
  }
  ok();
  return 0;
}
static int cmd_add(char *key, char *val)
{
  dcache_data d;
  d.p=val;
  d.len=str_len(val);
  d.next=0;
  if (-1==dcache_enter(&c,key,str_len(key),&d,0)) {
    warning(errno,"failed to enter record into cache",0,0,0);
    return -1;
  }
  ok();
  return 0;
}
static int cmd_delete(char *key, char *occ)
{
  unsigned long x=0;
  unsigned long no=0;
  int all=0;
  if (str_equal(occ,"all"))
    all=1;
  else
    scan_ulong(occ,&x);

  dcache_lookupstart(&c);
  while (1) {
    switch(dcache_lookupnext(&c,key,str_len(key))) {
    case -1:
      warning(errno,"failed to search in cache",0,0,0);
      return -1;
    case 0:
      return 0;
    }
    if (all || x==no++) {
      if (-1==dcache_delete(&c)) {
	warning(errno,"failed to delete record",0,0,0);
	return -1;
      }
    }
  }
  ok();
}
static int cmd_close(void)
{
  if (in_transaction) {
    warning(0,"not closing: transaction is open",0,0,0);
    return -1;
  }
  dcache_free(&c);
  close(c.fd);
  c.fd=-1;
  ok();
  return 0;
}
static int cmd_flush(void)
{
  if (in_transaction) {
    warning(0,"not flushing: transaction is open",0,0,0);
    return -1;
  }
  CMDWRAP_RETURN(dcache_flush(&c),"failed to flush cache");
  return 0;
}
static int cmd_sync(void)
{
  if (in_transaction) {
    warning(0,"not syncing: transaction is open",0,0,0);
    return -1;
  }
  CMDWRAP_RETURN(dcache_sync(&c),"failed to sync cache");
  return 0;
}
static int cmd_autosync(char *s)
{
  if (str_equal(s,"on"))
    dcache_set_autosync(&c,1);
  else if (str_equal(s,"off"))
    dcache_set_autosync(&c,0);
  else {
    warning(0,"bad value for autosync: ",s,0,0);
    return -1;
  }
  ok();
  return 0;
}
static stralloc findsa;
static unsigned int findcount;
static int findverbose;
static int cmd_findnext(void)
{
  switch(dcache_lookupnext(&c,findsa.s,findsa.len)) {
  case -1:
    warning(errno,"failed to search cache",0,0,0);
    return -1;
  case 0:
    if (findverbose) {
      outs("no ");
      if (findcount) outs("further ");
      outs("record ");
      outs("with key ");
      out(findsa.s,findsa.len);
      outs(" found");
      outln();
    }
    return 0;
  }
  if (!findcount) {
    outs("Nr.    DataLen   DataPosition for key ");
    out(findsa.s,findsa.len);
    outln();
  }
  outuintfill(findcount++,4);
  outs(" ");
  outreclenfill(c.datalen,9);
  outs(" ");
  outpos(c.datapos);
  outln();
  return 1;
}
static int cmd_findfirst(char *s)
{
  if (!stralloc_copys(&findsa,s)) oom();
  dcache_lookupstart(&c);
  findcount=0;
  findverbose=1;
  return cmd_findnext();
}
static int cmd_findall(char *s)
{
  int x;
  x=cmd_findfirst(s);
  findverbose=0;
  while (1) {
    if (x!=1) return x;
    x=cmd_findnext();
  }
}
static int cmd_echo(char *s, char *t)
{
  outs(s);
  if (*t) outs(" ");
  outs(t);
  outln();
  return 0;
}

#define F_O 1
#define F_A 2
#define F_S 4
static int cmd_help(void);
static int cmd_help1(char *s);
struct cmdtab {
  const char *name;
  int flag;
  int (*fn0) (void);
  int (*fn1) (char *);
  int (*fn2) (char *, char *);
  const char *args;
  const char *hlp;
} cmdtab[]=
{
 {"OPEN/CLOSE", F_S, 0,0,0,   0,0},
 {"openread",        0, 0,cmd_openread,0,      "FILENAME", 
   "Open FILENAME for reading."},
 {"openwrite",       0, 0,cmd_openwrite,0,     "FILENAME", 
   "Open FILENAME for writing."},
 {"close",         F_O, cmd_close,0,0,         0, 
   "Close an open cache."},
 {"LOCKING", F_S, 0,0,0,   0,0},
 {"lockshared",    F_O, cmd_lock_shared,0,0,   0, 
   "Lock the cache so that other processes may still read from it.\n"
   "Use this if you will not change the content of the cache" },
 {"lockexcl",      F_O, cmd_lock_excl,0,0,     0, 
   "Lock the cache so that other processes may not read from it.\n"
   "Use this if you plan to change the content of the cache" },
 {"unlock",        F_O, cmd_unlock,0,0,        0, 
   "Release a lock."},
 {"SYNCHRONIZATION", F_S, 0,0,0,   0,0},
 {"flush",         F_O, cmd_flush,0,0,         0, 
   "Flush cache changes to operating system buffer."},
 {"sync",          F_O, cmd_sync,0,0,          0, 
   "Call flush and fsync() to force changes to disk."},
 {"autosync",      F_O, 0,cmd_autosync,0,      "[ON|OFF]", 
   "Set autosync to ON or OFF."},
 {"TRANSACTIONS", F_S, 0,0,0,   0,0},
 {"start",         F_O, cmd_trans_start,0,0,   0, 
   "Start a transaction."},
 {"cancel",        F_O, cmd_trans_cancel,0,0,  0, 
   "Cancel a transaction."},
 {"commit",        F_O, cmd_trans_commit,0,0,  0, 
   "Commit a transaction."},
 {"replay",        F_O, cmd_trans_replay,0,0,  0,
   "Replay unfinished transactions."},
 {"DATA OPERATIONS", F_S, 0,0,0,   0,0},
 {"add",           F_O, 0,0,cmd_add,           "KEY VALUE", 
   "Add a new record with key KEY and content VALUE.\n"
   "Note that this will not delete other records with KEY."},
 {"delete",        F_O, 0,0,cmd_delete,        "KEY NUMBER-or-ALL", 
  "Delete one or all records matching KEY. Usage:\n"
  "- delete KEY 47\n"
  "  delete the 47th record with key KEY.\n"
  "- delete KEY ALL\n"
  "  delete all records with key KEY."
  },
 {"findfirst",     F_O, 0,cmd_findfirst,0,     "KEY", 
  "Find the first record matching KEY."},
 {"findnext",      F_O, cmd_findnext,0,0,      0, 
  "Find the next record matching KEY.\n"
  "This may be used after a findfirst command."},
 {"findall",       F_O, 0,cmd_findall,0,       "KEY", 
  "Find all records matching KEY."},

 {"MISC", F_S, 0,0,0,   0,0},
 {"echo",            0, 0,0,cmd_echo,          "STRING STRING2", 
   "Print STRING and STRING2"},
 {"quit",            0, cmd_quit,0,0,          0, 
   "Leave the program if no cache file is open."},
 {"q",             F_A, cmd_quit,0,0,          0, 
   "Leave the program if no cache file is open."},
 {"exit",          F_A, cmd_quit,0,0,          0, 
   "Leave the program if no cache file is open."},
 {"forcequit",       0, cmd_forcequit,0,0,     0, 
   "Leave the program even if the a cache file is open."},
 {"callback",      F_O, 0,cmd_callback,0,      "[none|print|fail]", 
  "Set the delete callback to\n"
  "- none:  do nothing.\n"
  "- fail:  let the operation fail.\n"
  "- print: print the record just before it is deleted\n"
  "This is determines how this program handles automatical deletion."},
 {"help",            0, cmd_help,cmd_help1,0,  "[command]", 
  "Show help on all or one command."},
 {"?",             F_A, cmd_help,cmd_help1,0,  "[command]", 
  "Show help on all or one command."},
 {0,0,0,0,0,0, 0}
};


static int 
command(void)
{
  char *a;
  char *b;
  unsigned int x;
  if (cmd.s[0]=='#')
    return 0;
  x=str_chr(cmd.s,' ');
  while (cmd.s[x]==' ')
    cmd.s[x++]=0;
  a=cmd.s+x;
  b=a+str_chr(a,' ');
  while (*b==' ')
    *b++=0;
  for (x=0;cmdtab[x].name;x++) {
    if (!str_equal(cmdtab[x].name,cmd.s)) continue;
    if ((cmdtab[x].flag & F_O) && !ISOPEN) {
      warning(0,cmd.s, " can only be called if a dcache is open",0,0);
      return -1;
    }
    if (*b && cmdtab[x].fn2)
      return cmdtab[x].fn2(a,b);
    if (*a && cmdtab[x].fn1)
      return cmdtab[x].fn1(a);
    else if (!*a && cmdtab[x].fn0)
      return cmdtab[x].fn0();
    else
      warning(0,"usage: ",cmd.s," ",
	cmdtab[x].args ? cmdtab[x].args : "[no arguments allowed]");
      return -1;
  }
  warning(0,"unknown command: ",cmd.s,0,0);
  return -1;
}

int 
main(int argc, char **argv)
{
  static stralloc x;
  unsigned int i;
  bailout_progname(argv[0]);
  flag_bailout_fatal_begin = 3;
  for (i=0;cmdtab[i].name;i++) {
    const char *p;
    if (cmdtab[i].flag & F_S) {
      if (!stralloc_catb(&x,"\n========== ",12)) oom();
      if (!stralloc_catb(&x,cmdtab[i].name,str_len(cmdtab[i].name))) oom();
      if (!stralloc_catb(&x," ==========\n\n",13)) oom();
      continue;
    }
    if (cmdtab[i].flag & F_A) continue;
    if (!stralloc_catb(&x,cmdtab[i].name,str_len(cmdtab[i].name))) oom();
    if (cmdtab[i].args) {
      if (!stralloc_catb(&x," ",1)) oom();
      if (!stralloc_catb(&x,cmdtab[i].args,str_len(cmdtab[i].args))) oom();
    }
    if (cmdtab[i].flag & F_O)
      if (!stralloc_catb(&x," [needs open cache]",19)) oom();
    if (!stralloc_catb(&x,"\n  ",3)) oom();
    for (p=cmdtab[i].hlp;*p;p++) {
      if (!stralloc_catb(&x,p,1)) oom();
      if (*p=='\n' && p[1])
	if (!stralloc_catb(&x,"  ",2)) oom();
    }
    if (p[-1]!='\n')
      if (!stralloc_catb(&x,"\n",1)) oom();
  }
  if (!stralloc_catb(&x,"",1)) oom();
  for(i=0;myopts[i].longname;i++)
    if (str_equal(myopts[i].longname,"commands")) {
      myopts[i].longhelp=x.s;
    }

/*
  const char *name;
  int flag;
  int (*fn0) (void);
  int (*fn1) (char *);
  int (*fn2) (char *, char *);
  const char *args;
  const char *hlp;
*/

  uogetopt_parse(&myenv,&argc,argv);
  c.fd=-1;
  if (isatty(0)) {
    outs("dcachesh (");
    outs(PACKAGE);
    outs(" ");
    outs(VERSION);
    outs(")");
    outln();
    outs("Use the 'help' command if needed.");
    outln();
  }
  while (1)  {
    int r;
    int gotlf;
    if (isatty(0)) {
      outs(">");
      outflush();
    }
    r=getln(buffer_0,&cmd,&gotlf,'\n');
    if (-1==r) xbailout(111,errno,"failed to read input",0,0,0); 
    if (!cmd.len) break;
    if (!gotlf) xbailout(111,0,"last line not terminated",0,0,0);
    cmd.s[cmd.len-1]=0;
    out_pos=0; /* got lf */
    command();
  }
  if (c.fd!=-1) 
  	dcache_free(&c);
  return 0;
}

static int cmd_help(void)
{ 
  int i;
  for (i=0;cmdtab[i].name;i++) {
    if (cmdtab[i].flag & (F_A|F_S)) 
      continue;
    outs(cmdtab[i].name);
    while (out_pos<13)
      outs(" ");
    outs(cmdtab[i].args ?  cmdtab[i].args : "");
    while (out_pos<54)
      outs(" ");
    if (cmdtab[i].flag & F_O)
      outs("[needs open cache]");
    outln();
  }
  return 0;
}
static int cmd_help1(char *s)
{ 
  int i;
  for (i=0;cmdtab[i].name;i++) {
    if (str_equal(cmdtab[i].name,s)) {
      if (cmdtab[i].hlp)
	outs(cmdtab[i].hlp);
      else	
	outs("no help available");
      outln();
      return 0;
    }
  }
  warning(0,"no such command",0,0,0);
  return -1;
}
