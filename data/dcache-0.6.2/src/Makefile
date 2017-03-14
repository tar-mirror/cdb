VERSION=0.6.2
PACKAGE=dcache
CLASS=data
INCLUDES=-I.



CC=gcc
L=./auto-link.sh
C=./auto-compile.sh
W=./auto-compilew.sh
MYMAKE=make
PACKAGE_VERSION=$(PACKAGE)-$(VERSION)
PREMAKE_DEFS=-DPACKAGE_VERSION=\"$(PACKAGE)-$(VERSION)\" \
 -DPACKAGE=\"$(PACKAGE)\" -DVERSION=\"$(VERSION)\" -DPACKAGE_CLASS=\"$(CLASS)\"

TARGETS=$(EXTRA_TARGETS) $(PREMAKE_TARGETS)
CLEANFILES=$(EXTRA_CLEANFILES) $(PREMAKE_CLEANFILES)
MAINTAINER_CLEANFILES=$(EXTRA_MAINTAINER_CLEANFILES) $(PREMAKE_MAINTAINER_CLEANFILES)
PREMAKE_DIST=$(PACKAGE_VERSION).tar.gz
#
#
# the dependency of socket_ux on ipv6 is here because premake2 can't 
# handle it otherwise.



PREMAKE_TARGETS = dcachemake dcachedump dcachelist dcachestats \
dcachedel dcacheadd dcacheget dcachereplay dcachesh dcache_intro.7 \
libdcache.3 dcacheadd.1 dcachedel.1 dcachedump.1 dcacheget.1 \
dcachelist.1 dcachemake.1 dcachereplay.1 dcachesh.1 dcachestats.1 \
dcacheadd.html dcachedel.html dcachedump.html dcacheget.html \
dcachelist.html dcachemake.html dcachereplay.html dcachesh.html \
dcachestats.html
 
PREMAKE_CLEANFILES = check.out t.* auto-gettimeofday.lib fsync.o \
auto-fsync.lib auto-have_ftruncate.h auto-version.tmac \
dcache_intro.html.t install.html.t libdcache.html.t INSTALL.t \
dcache_intro.7 dcache_intro.7.t libdcache.3 libdcache.3.t dcacheadd.1 \
dcacheadd.1.t dcachedel.1 dcachedel.1.t dcachedump.1 dcachedump.1.t \
dcacheget.1 dcacheget.1.t dcachelist.1 dcachelist.1.t dcachemake.1 \
dcachemake.1.t dcachereplay.1 dcachereplay.1.t dcachesh.1 dcachesh.1.t \
dcachestats.1 dcachestats.1.t dcacheadd.html dcacheadd.html.t \
dcachedel.html dcachedel.html.t dcachedump.html dcachedump.html.t \
dcacheget.html dcacheget.html.t dcachelist.html dcachelist.html.t \
dcachemake.html dcachemake.html.t dcachereplay.html \
dcachereplay.html.t dcachesh.html dcachesh.html.t dcachestats.html \
dcachestats.html.t auto-typesize.h auto-typesize.stamp *.stamp
 
PREMAKE_MAINTAINER_CLEANFILES = dcache_intro.html install.html \
libdcache.html INSTALL
 

libbyte_a_OBJECTS=byte_chr.o byte_copy.o byte_cr.o fmt_ulong.o \
fmt_xlong.o fmt_xlong0.o scan_long.o scan_sign.o scan_ulong.o \
str_chr.o str_diff.o str_len.o
 
libdcache_a_OBJECTS=dcache_cb.o dcache_create.o dcache_del.o \
dcache_enter.o dcache_fdisk.o dcache_flush.o dcache_free.o \
dcache_hash.o dcache_init.o dcache_lock.o dcache_look.o dcache_osync.o \
dcache_read.o dcache_ritev.o dcache_seek.o dcache_tdisk.o \
dcache_trans.o dcache_walk.o dcache_write.o
 
libtai_a_OBJECTS=tai_add.o tai_now.o tai_uint.o
 
libunix_a_OBJECTS=alloc.o alloc_re.o buffer.o buffer_0.o buffer_1.o \
buffer_2.o buffer_get.o buffer_put.o env_get.o error.o error_str.o \
gen_alloc_ady.o gen_alloc_lus.o getln.o open_read.o open_readwri.o \
readclose.o readclose_app.o stralloc_catb.o stralloc_eady.o \
stralloc_opyb.o stralloc_opys.o stralloc_plus.o
 
libuomisc_a_OBJECTS=bailout.o bailout_name.o uogetopt.o
 
PREMAKE_EXTRA_SYSTYPE =  auto-gettimeofday.lib auto-have_fsync.h \
auto-have_ftruncate.h auto-typesize.h
 
all:  auto-have_ftruncate.h auto-systype $(TARGETS) $(EXTRA_TARGETS)
 
auto-gettimeofday.lib: gettimeofday.sh auto-compile.sh auto-link.sh
	sh gettimeofday.sh >gettimeofday.t && mv gettimeofday.t $@
dcachemake:  dcachemake.o libdcache.a libuomisc.a libtai.a libunix.a libbyte.a auto-link.sh
	$L dcachemake  dcachemake.o libdcache.a libuomisc.a libtai.a libunix.a libbyte.a 
dcachedump:  dcachedump.o libdcache.a libuomisc.a libtai.a libunix.a libbyte.a auto-link.sh
	$L dcachedump  dcachedump.o libdcache.a libuomisc.a libtai.a libunix.a libbyte.a 
dcachelist:  dcachelist.o libdcache.a libuomisc.a libtai.a libunix.a libbyte.a auto-link.sh
	$L dcachelist  dcachelist.o libdcache.a libuomisc.a libtai.a libunix.a libbyte.a 
dcachestats:  dcachestats.o libdcache.a libuomisc.a libtai.a libunix.a libbyte.a auto-link.sh
	$L dcachestats  dcachestats.o libdcache.a libuomisc.a libtai.a libunix.a libbyte.a 
dcachedel:  dcachedel.o libdcache.a libuomisc.a libtai.a libunix.a libbyte.a auto-link.sh
	$L dcachedel  dcachedel.o libdcache.a libuomisc.a libtai.a libunix.a libbyte.a 
dcacheadd:  dcacheadd.o libdcache.a libuomisc.a libtai.a libunix.a libbyte.a auto-link.sh
	$L dcacheadd  dcacheadd.o libdcache.a libuomisc.a libtai.a libunix.a libbyte.a 
dcacheget:  dcacheget.o libdcache.a libuomisc.a libtai.a libunix.a libbyte.a auto-link.sh
	$L dcacheget  dcacheget.o libdcache.a libuomisc.a libtai.a libunix.a libbyte.a 
dcachereplay:  dcachereplay.o libdcache.a libuomisc.a libtai.a libunix.a libbyte.a auto-link.sh
	$L dcachereplay  dcachereplay.o libdcache.a libuomisc.a libtai.a libunix.a libbyte.a 
dcachesh:  dcachesh.o libdcache.a libuomisc.a libtai.a libunix.a libbyte.a auto-link.sh
	$L dcachesh  dcachesh.o libdcache.a libuomisc.a libtai.a libunix.a libbyte.a 
auto-have_fsync.h: have_func.sh
	sh have_func.sh fsync "#include <unistd.h>" "fsync(0)" "" "" >$@.t && mv $@.t $@
auto-fsync.lib: fsync.sh auto-compile.sh auto-link.sh auto-have_fsync.h
	sh fsync.sh >fsync.t && mv fsync.t $@
auto-have_ftruncate.h: have_func.sh auto-compile.sh auto-link.sh
	sh have_func.sh ftruncate "#include <unistd.h>" "ftruncate(0,0)" "" "" >$@.t && mv $@.t $@
auto-typesize.h: auto-typesize.stamp
auto-typesize.stamp: typesize.sh auto-compile.sh auto-link.sh
	sh typesize.sh >typesize.t
	@if cmp typesize.t auto-typesize.h 2>/dev/null >/dev/null ; then : ; else mv typesize.t auto-typesize.h ; fi
	rm -f typesize.t
	touch $@
libbyte.a: $(libbyte_a_OBJECTS) auto-makelib.sh
	./auto-makelib.sh $@ $(libbyte_a_OBJECTS)
libdcache.a: $(libdcache_a_OBJECTS) auto-makelib.sh
	./auto-makelib.sh $@ $(libdcache_a_OBJECTS)
libtai.a: $(libtai_a_OBJECTS) auto-makelib.sh
	./auto-makelib.sh $@ $(libtai_a_OBJECTS)
libunix.a: $(libunix_a_OBJECTS) auto-makelib.sh
	./auto-makelib.sh $@ $(libunix_a_OBJECTS)
libuomisc.a: $(libuomisc_a_OBJECTS) auto-makelib.sh
	./auto-makelib.sh $@ $(libuomisc_a_OBJECTS)
dcache_intro.html: dcache_intro.7.in groff.tmac \
auto-version.tmac
	groff -mwww -Thtml -P-l groff.tmac auto-version.tmac \
dcache_intro.7.in\
	 |sed -e 's%<title></title>%<title>An introduction to \
dcache</title><meta name="Description" content="The dcache persistant \
caching library and tools - an overview" /><meta name="Keywords" \
content="dcache,cache,caching,persistence,persistent,persisting,library,tools" \
/><!-- @head@ -->%' -e 's%<body>%<body><!-- @top@ -->%' -e \
's%</body>%<!-- @bottom@ --></body>%' >dcache_intro.html.t && mv \
dcache_intro.html.t dcache_intro.html
install.html: dcacheinstall.0.in groff.tmac auto-version.tmac
	groff -mwww -Thtml -P-l groff.tmac auto-version.tmac \
dcacheinstall.0.in\
	 |sed -e 's%<title></title>%<title>The dcache library</title><meta \
name="Description" content="The dcache persistant caching library - \
API description" /><meta name="Keywords" \
content="dcache,install,installation" /><!-- @head@ -->%' -e \
's%<body>%<body><!-- @top@ -->%' -e 's%</body>%<!-- @bottom@ \
--></body>%' >install.html.t && mv install.html.t \
install.html
libdcache.html: libdcache.3.in groff.tmac auto-version.tmac
	groff -mwww -Thtml -P-l groff.tmac auto-version.tmac libdcache.3.in\
	 |sed -e 's%<title></title>%<title>The dcache library</title><meta \
name="Description" content="The dcache persistant caching library - \
API description" /><meta name="Keywords" \
content="dcache,cache,caching,persistence,persistent,persisting,library,API,functions" \
/><!-- @head@ -->%' -e 's%<body>%<body><!-- @top@ -->%' -e \
's%</body>%<!-- @bottom@ --></body>%' >libdcache.html.t && mv \
libdcache.html.t libdcache.html
INSTALL: dcacheinstall.0.in groff.tmac auto-version.tmac
	groff -man -Tascii groff.tmac auto-version.tmac dcacheinstall.0.in \
|sed 's/\o33\[[0-9]*m//g ' |col -b >INSTALL.t && mv INSTALL.t \
INSTALL
dcache_intro.7: dcache_intro.7.in groff.tmac auto-version.tmac
	cat groff.tmac auto-version.tmac dcache_intro.7.in \
 | sed 's/^.section/.SH/' >dcache_intro.7.t \
 && mv dcache_intro.7.t dcache_intro.7
libdcache.3: libdcache.3.in groff.tmac auto-version.tmac
	cat groff.tmac auto-version.tmac libdcache.3.in \
 | sed 's/^.section/.SH/' >libdcache.3.t \
 && mv libdcache.3.t libdcache.3
dcacheadd.1: dcacheadd uogo2man
	perl uogo2man dcache ./dcacheadd 1 man keywords \
"dcache,record,records,add,adding" >$@.t &&mv $@.t $@
dcachedel.1: dcachedel uogo2man
	perl uogo2man dcache ./dcachedel 1 man keywords \
"dcache,record,records,delete,deleting" >$@.t &&mv $@.t \
$@
dcachedump.1: dcachedump uogo2man
	perl uogo2man dcache ./dcachedump 1 man "--output-format-help" "OUTPUT \
FORMAT" keywords "dcache,dump,snapshot" >$@.t &&mv $@.t \
$@
dcacheget.1: dcacheget uogo2man
	perl uogo2man dcache ./dcacheget 1 man keywords \
"dcache,record,get,getting" >$@.t &&mv $@.t $@
dcachelist.1: dcachelist uogo2man
	perl uogo2man dcache ./dcachelist 1 man "--output-format-help" "OUTPUT \
FORMAT" "--caveats" "CAVEATS" "keywords" \
"dcache,list,listing,content,contents" >$@.t &&mv $@.t \
$@
dcachemake.1: dcachemake uogo2man
	perl uogo2man dcache ./dcachemake 1 man "--input-format-help" "INPUT \
FORMAT" keywords "dcache,create,creating,creation" >$@.t &&mv $@.t \
$@
dcachereplay.1: dcachereplay uogo2man
	perl uogo2man dcache ./dcachereplay 1 man keywords \
"dcache,transaction,replay" >$@.t &&mv $@.t $@
dcachesh.1: dcachesh uogo2man
	perl uogo2man dcache ./dcachesh 1 man keywords \
"dcache,tool,interactive,shell" "--commands" "COMMANDS" >$@.t &&mv \
$@.t $@
dcachestats.1: dcachestats uogo2man
	perl uogo2man dcache ./dcachestats 1 man "--output-format" "OUTPUT \
FORMAT" keywords "dcache,statistics,stats" >$@.t &&mv $@.t \
$@
dcacheadd.html: dcacheadd uogo2man doc-html-header doc-html-tail \
doc-highlights doc-innerlinks
	perl uogo2man dcache ./dcacheadd - html keywords \
"dcache,record,records,add,adding" >$@.t &&mv $@.t $@
dcachedel.html: dcachedel uogo2man doc-html-header doc-html-tail \
doc-highlights doc-innerlinks
	perl uogo2man dcache ./dcachedel - html keywords \
"dcache,record,records,delete,deleting" >$@.t &&mv $@.t \
$@
dcachedump.html: dcachedump uogo2man doc-html-header doc-html-tail \
doc-highlights doc-innerlinks
	perl uogo2man dcache ./dcachedump - html "--output-format-help" \
"OUTPUT FORMAT" keywords "dcache,dump,snapshot" >$@.t &&mv $@.t \
$@
dcacheget.html: dcacheget uogo2man doc-html-header doc-html-tail \
doc-highlights doc-innerlinks
	perl uogo2man dcache ./dcacheget - html keywords \
"dcache,record,get,getting" >$@.t &&mv $@.t $@
dcachelist.html: dcachelist uogo2man doc-html-header doc-html-tail \
doc-highlights doc-innerlinks
	perl uogo2man dcache ./dcachelist - html "--output-format-help" \
"OUTPUT FORMAT" "--caveats" "CAVEATS" "keywords" \
"dcache,list,listing,content,contents" >$@.t &&mv $@.t \
$@
dcachemake.html: dcachemake uogo2man doc-html-header doc-html-tail \
doc-highlights doc-innerlinks
	perl uogo2man dcache ./dcachemake - html "--input-format-help" "INPUT \
FORMAT" keywords "dcache,create,creating,creation" >$@.t &&mv $@.t \
$@
dcachereplay.html: dcachereplay uogo2man doc-html-header doc-html-tail \
doc-highlights doc-innerlinks
	perl uogo2man dcache ./dcachereplay - html keywords \
"dcache,transaction,replay" >$@.t &&mv $@.t $@
dcachesh.html: dcachesh uogo2man doc-html-header doc-html-tail \
doc-highlights doc-innerlinks
	perl uogo2man dcache ./dcachesh - html keywords \
"dcache,tool,interactive,shell" "--commands" "COMMANDS" >$@.t &&mv \
$@.t $@
dcachestats.html: dcachestats uogo2man doc-html-header doc-html-tail \
doc-highlights doc-innerlinks
	perl uogo2man dcache ./dcachestats - html "--output-format" "OUTPUT \
FORMAT" keywords "dcache,statistics,stats" >$@.t &&mv $@.t \
$@
auto-version.tmac: version.tmac.stamp
version.tmac.stamp: Makefile
	(echo .ds PA $(PACKAGE) ;\
	 echo .ds VE $(VERSION) ;\
	 echo .ds CL $(CLASS) ;\
	) >auto-version.tmac.t
	cmp auto-version.tmac auto-version.tmac.t \
	 || cp auto-version.tmac.t auto-version.tmac
	touch $@ && rm auto-version.tmac.t
html: dcache_intro.html install.html libdcache.html INSTALL \
dcacheadd.html dcachedel.html dcachedump.html dcacheget.html \
dcachelist.html dcachemake.html dcachereplay.html dcachesh.html \
dcachestats.html

 
alloc.o: alloc.c alloc.h error.h auto-compilew.sh
	$W  -c alloc.c
alloc_re.o: alloc_re.c alloc.h byte.h auto-compilew.sh
	$W  -c alloc_re.c
bailout.o: bailout.c error.h buffer.h bailout.h fmt.h auto-compilew.sh
	$W  -c bailout.c
bailout_name.o: bailout_name.c bailout.h buffer.h auto-compilew.sh
	$W  -c bailout_name.c
buffer.o: buffer.c buffer.h auto-compile.sh
	$C  -c buffer.c
buffer_0.o: buffer_0.c readwrite.h buffer.h auto-compile.sh
	$C  -c buffer_0.c
buffer_1.o: buffer_1.c readwrite.h buffer.h auto-compile.sh
	$C  -c buffer_1.c
buffer_2.o: buffer_2.c readwrite.h buffer.h auto-compile.sh
	$C  -c buffer_2.c
buffer_get.o: buffer_get.c buffer.h byte.h error.h auto-compile.sh
	$C  -c buffer_get.c
buffer_put.o: buffer_put.c buffer.h str.h byte.h error.h auto-compile.sh
	$C  -c buffer_put.c
byte_chr.o: byte_chr.c byte.h auto-compilew.sh
	$W  -c byte_chr.c
byte_copy.o: byte_copy.c byte.h auto-compilew.sh
	$W  -c byte_copy.c
byte_cr.o: byte_cr.c byte.h auto-compilew.sh
	$W  -c byte_cr.c
dcache_cb.o: dcache_cb.c dcache.h typesize.h auto-typesize.h auto-compilew.sh
	$W  -c dcache_cb.c
dcache_create.o: dcache_create.c dcache.h typesize.h auto-typesize.h \
  dcachei.h auto-have_ftruncate.h auto-compilew.sh
	$W  -c dcache_create.c
dcache_del.o: dcache_del.c dcache.h typesize.h auto-typesize.h dcachei.h auto-compilew.sh
	$W  -c dcache_del.c
dcache_enter.o: dcache_enter.c dcache.h typesize.h auto-typesize.h \
  dcachei.h auto-compilew.sh
	$W  -c dcache_enter.c
dcache_fdisk.o: dcache_fdisk.c dcache.h typesize.h auto-typesize.h \
  dcachei.h auto-compilew.sh
	$W  -c dcache_fdisk.c
dcache_flush.o: dcache_flush.c dcache.h typesize.h auto-typesize.h auto-compilew.sh
	$W  -c dcache_flush.c
dcache_free.o: dcache_free.c dcache.h typesize.h auto-typesize.h auto-compilew.sh
	$W  -c dcache_free.c
dcache_hash.o: dcache_hash.c dcache.h typesize.h auto-typesize.h \
  dcachei.h auto-compilew.sh
	$W  -c dcache_hash.c
dcache_init.o: dcache_init.c dcache.h typesize.h auto-typesize.h \
  dcachei.h auto-compilew.sh
	$W  -c dcache_init.c
dcache_lock.o: dcache_lock.c dcache.h typesize.h auto-typesize.h auto-compilew.sh
	$W  -c dcache_lock.c
dcache_look.o: dcache_look.c dcache.h typesize.h auto-typesize.h \
  dcachei.h auto-compilew.sh
	$W  -c dcache_look.c
dcache_osync.o: dcache_osync.c dcache.h typesize.h auto-typesize.h auto-compilew.sh
	$W  -c dcache_osync.c
dcache_read.o: dcache_read.c dcache.h typesize.h auto-typesize.h auto-compilew.sh
	$W  -c dcache_read.c
dcache_ritev.o: dcache_ritev.c dcache.h typesize.h auto-typesize.h \
  dcachei.h auto-compilew.sh
	$W  -c dcache_ritev.c
dcache_seek.o: dcache_seek.c dcache.h typesize.h auto-typesize.h auto-compilew.sh
	$W  -c dcache_seek.c
dcache_tdisk.o: dcache_tdisk.c dcache.h typesize.h auto-typesize.h \
  dcachei.h auto-compilew.sh
	$W  -c dcache_tdisk.c
dcache_trans.o: dcache_trans.c dcache.h typesize.h auto-typesize.h \
  dcachei.h auto-have_ftruncate.h auto-compilew.sh
	$W  -c dcache_trans.c
dcache_walk.o: dcache_walk.c dcache.h typesize.h auto-typesize.h \
  dcachei.h auto-compilew.sh
	$W  -c dcache_walk.c
dcache_write.o: dcache_write.c dcache.h typesize.h auto-typesize.h \
  dcachei.h auto-compilew.sh
	$W  -c dcache_write.c
dcacheadd.o: dcacheadd.c error.h buffer.h stralloc.h gen_alloc.h dcache.h \
  typesize.h auto-typesize.h readclose.h bailout.h open.h uogetopt.h \
  attributes.h str.h common.h auto-compilew.sh
	$W  -c dcacheadd.c
dcachedel.o: dcachedel.c error.h attributes.h dcache.h typesize.h \
  auto-typesize.h bailout.h buffer.h open.h uogetopt.h str.h common.h auto-compilew.sh
	$W  -c dcachedel.c
dcachedump.o: dcachedump.c dcache.h typesize.h auto-typesize.h fmt.h \
  error.h readwrite.h buffer.h bailout.h uogetopt.h attributes.h str.h \
  common.h tai.h uint64.h auto-compilew.sh
	$W  -c dcachedump.c
dcacheget.o: dcacheget.c error.h readwrite.h buffer.h dcache.h typesize.h \
  auto-typesize.h bailout.h uogetopt.h attributes.h str.h common.h auto-compilew.sh
	$W  -c dcacheget.c
dcachelist.o: dcachelist.c dcache.h typesize.h auto-typesize.h error.h \
  dcachei.h fmt.h readwrite.h buffer.h bailout.h uogetopt.h attributes.h \
  str.h common.h tai.h uint64.h auto-compilew.sh
	$W  -c dcachelist.c
dcachemake.o: dcachemake.c error.h buffer.h stralloc.h gen_alloc.h \
  dcache.h typesize.h auto-typesize.h bailout.h fmt.h uogetopt.h \
  attributes.h str.h open.h scan.h common.h auto-compilew.sh
	$W  -c dcachemake.c
dcachereplay.o: dcachereplay.c error.h attributes.h dcache.h typesize.h \
  auto-typesize.h bailout.h buffer.h open.h uogetopt.h str.h common.h auto-compilew.sh
	$W  -c dcachereplay.c
dcachesh.o: dcachesh.c error.h buffer.h dcache.h typesize.h \
  auto-typesize.h bailout.h uogetopt.h attributes.h getln.h stralloc.h \
  gen_alloc.h str.h open.h scan.h fmt.h close.h common.h auto-compilew.sh
	$W  -c dcachesh.c
dcachestats.o: dcachestats.c attributes.h error.h dcache.h typesize.h \
  auto-typesize.h dcachei.h bailout.h buffer.h uogetopt.h fmt.h common.h auto-compilew.sh
	$W  -c dcachestats.c
env_get.o: env_get.c env.h auto-compilew.sh
	$W  -c env_get.c
error.o: error.c error.h auto-compilew.sh
	$W  -c error.c
error_str.o: error_str.c error.h auto-compilew.sh
	$W  -c error_str.c
fmt_ulong.o: fmt_ulong.c gen_fmt.h fmt.h auto-compilew.sh
	$W  -c fmt_ulong.c
fmt_xlong.o: fmt_xlong.c gen_fmt.h fmt.h auto-compilew.sh
	$W  -c fmt_xlong.c
fmt_xlong0.o: fmt_xlong0.c gen_fmt.h fmt.h auto-compilew.sh
	$W  -c fmt_xlong0.c
gen_alloc_ady.o: gen_alloc_ady.c alloc.h gen_alloci.h auto-compilew.sh
	$W  -c gen_alloc_ady.c
gen_alloc_lus.o: gen_alloc_lus.c gen_alloci.h auto-compilew.sh
	$W  -c gen_alloc_lus.c
getln.o: getln.c byte.h getln.h buffer.h stralloc.h gen_alloc.h auto-compilew.sh
	$W  -c getln.c
open_read.o: open_read.c open.h auto-compilew.sh
	$W  -c open_read.c
open_readwri.o: open_readwri.c open.h auto-compilew.sh
	$W  -c open_readwri.c
readclose.o: readclose.c readclose.h stralloc.h gen_alloc.h auto-compilew.sh
	$W  -c readclose.c
readclose_app.o: readclose_app.c readwrite.h error.h readclose.h \
  stralloc.h gen_alloc.h close.h auto-compilew.sh
	$W  -c readclose_app.c
scan_long.o: scan_long.c gen_scan.h scan.h case.h auto-compilew.sh
	$W  -c scan_long.c
scan_sign.o: scan_sign.c scan.h auto-compilew.sh
	$W  -c scan_sign.c
scan_ulong.o: scan_ulong.c gen_scan.h scan.h case.h auto-compilew.sh
	$W  -c scan_ulong.c
str_chr.o: str_chr.c str.h auto-compilew.sh
	$W  -c str_chr.c
str_diff.o: str_diff.c str.h auto-compilew.sh
	$W  -c str_diff.c
str_len.o: str_len.c str.h auto-compilew.sh
	$W  -c str_len.c
stralloc_catb.o: stralloc_catb.c stralloc.h gen_alloc.h byte.h auto-compilew.sh
	$W  -c stralloc_catb.c
stralloc_eady.o: stralloc_eady.c stralloc.h gen_alloc.h gen_alloci.h auto-compilew.sh
	$W  -c stralloc_eady.c
stralloc_opyb.o: stralloc_opyb.c stralloc.h gen_alloc.h byte.h auto-compilew.sh
	$W  -c stralloc_opyb.c
stralloc_opys.o: stralloc_opys.c str.h stralloc.h gen_alloc.h auto-compilew.sh
	$W  -c stralloc_opys.c
stralloc_plus.o: stralloc_plus.c stralloc.h gen_alloc.h gen_alloci.h auto-compilew.sh
	$W  -c stralloc_plus.c
tai_add.o: tai_add.c tai.h uint64.h typesize.h auto-typesize.h auto-compilew.sh
	$W  -c tai_add.c
tai_now.o: tai_now.c tai.h uint64.h typesize.h auto-typesize.h auto-compile.sh
	$C  -c tai_now.c
tai_uint.o: tai_uint.c tai.h uint64.h typesize.h auto-typesize.h auto-compilew.sh
	$W  -c tai_uint.c
uogetopt.o: uogetopt.c str.h env.h scan.h uogetopt.h attributes.h auto-compilew.sh
	$W  -c uogetopt.c
rts check: rts-clean  rts-resolve 
rts-clean:
	rm -f rts-failures 
rts-resolve: 
	@if test -f rts-failures ; then echo The following tests failed: ; \
cat rts-failures ; exit 1; fi 
.PRECIOUS: Makefile
remake: clean
	rm -f auto-compile.sh auto-link.sh auto-compilew.sh auto-makelib.sh
packageinfo: 
	@echo version="$(VERSION)"
	@echo package="$(PACKAGE)"
	@echo class="$(CLASS)"

auto-compile.sh auto-compilew.sh auto-makelib.sh auto-link.sh \
auto-systype.lfs: guess-compiler.sh auto-compiler-config
auto-compiler-config: auto-compiler-config.stamp
auto-config: force
	@( for i in conf-cc conf-ld conf-ldflags conf-cflags conf-cflagsw \
	  conf-ar conf-ranlib conf-libs auto-compile.sh auto-compilew.sh \
	  auto-link.sh auto-makelib.sh ; do \
	  echo $$i ; if test -f $$i ; then cat $$i ; fi ;  \
	done ) >$@.t 
	@mv $@.t $@
auto-compiler-config.stamp: auto-config
	@if cmp auto-config $@ ; then : ; else \
	  echo configuration has changed ; \
	  DEFS='$(DEFS)' INCLUDES='$(INCLUDES)' \
	  PREMAKE_DEFS='$(PREMAKE_DEFS)' \
	  sh guess-compiler.sh ; \
	  rm auto-config ; \
	  $(MYMAKE) auto-config && mv auto-config $@ ; \
	fi 

maintainer-clean: distclean
	rm -f Makefile $(MAINTAINER_CLEANFILES)
distclean: clean
	rm -f $(DISTCLEANFILES) auto-dummy # to avoid "no arguments" error
clean spotless: mostlyclean
	rm -f $(TARGETS) auto-*
	rm -rf deps
mostlyclean:
	rm -f *.o *.a $(CLEANFILES)

NEWS-check:
	@if sed 1q NEWS | grep -e "$(VERSION)" > /dev/null; then :; else \
		echo "NEWS not updated; not releasing" 1>&2; \
		exit 1;                                   \
	fi
	@if grep -e "$(VERSION)" ChangeLog > /dev/null; then :; else \
		echo "ChangeLog not updated; not releasing" 1>&2; \
		exit 1;                                   \
	fi
dist:
	rm -f $(PREMAKE_DIST)
	package
bk-dist: NEWS-check
	bk tag `echo "Release-$(VERSION)" | sed 's/\./-/g'`
	$(MYMAKE) dist
gpg-dist: dist
	gpg -b --armor $(PREMAKE_DIST)
force:	

auto-systype: auto-typesize.h $(EXTRA_SYSTYPE) $(PREMAKE_EXTRA_SYSTYPE)
	rm -f $@.t
	@echo  >>$@.t
	@echo "PACKAGE: $(PACKAGE) $(VERSION)" >>$@.t
	@echo "REALNAME: $(REALNAME)" >>$@.t
	@echo "uname-a:" `uname -a` >>$@.t
	@echo "uname-s:" `uname -s` >>$@.t
	@echo "uname-r:" `uname -r` >>$@.t
	@echo "uname-v:" `uname -v` >>$@.t
	@echo "uname-m:" `uname -m` >>$@.t
	@echo "linux-ld.so:" `ls /lib/ld-*.so` >>$@.t
	@echo "linux-libc.so:" `ls /lib/libc-*.so` >>$@.t
	cat auto-typesize.h >>$@.t
	if test "x$(PREMAKE_EXTRA_SYSTYPE)$(EXTRA_SYSTYPE)" = x ; then true ; else \
	  grep -h systype-info $(PREMAKE_EXTRA_SYSTYPE) $(EXTRA_SYSTYPE) >>$@.t \
	      || exit 1 ; \
	fi
	mv $@.t $@

check: docheck check.exp auto-compile.sh auto-link.sh auto-compilew.sh \
		$(TARGETS)
	sh ./docheck 2>&1 | cat -v | tee check.out | diff -u check.exp -
