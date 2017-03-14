/*
 * Copyright (C) 2001-2002 Uwe Ohse, uwe@ohse.de
 * This is free software, licensed under the terms of the GNU Lesser
 * General Public License Version 2.1, of which a copy is stored at:
 *    http://www.ohse.de/uwe/licenses/LGPL-2.1
 * Later versions may or may not apply, see 
 *    http://www.ohse.de/uwe/licenses/
 * for information after a newer version has been published.
 */
#ifndef DCACHE_H
#define DCACHE_H

#include "typesize.h" /* does support autoconf.h */
typedef uo_int64_t dcache_pos;
typedef uo_int32_t dcache_reclen;
typedef uo_uint32_t dcache_uint32;
typedef uo_uint64_t dcache_time;

typedef struct dcache_data_struct {
	int fd; /* ignored if p != NULL */
	const char *p;
	dcache_reclen len;
	struct dcache_data_struct *next;
} dcache_data;

struct dcache_struct;
typedef int (*dcache_delete_callback) (struct dcache_struct *);

typedef struct dcache_struct {
	/* flags, options */
	int flag_autosync; /* defaults to one after dcache_init */
	dcache_delete_callback dcallback;
	dcache_pos datapos;
	dcache_reclen datalen;
	dcache_pos keypos;
	dcache_reclen keylen;
	dcache_time datatime;

	/* internal stuff, do not change */
	char *mapped;
	dcache_pos mapsize;
	dcache_pos loop;
	dcache_uint32 hash; /* during searches */
	dcache_pos *ep; /* pointer to elements pointers */
	dcache_uint32 *hp; /* pointer to elements hashes */

	dcache_pos walkpos;
	int walking; /* flag */
	int fd;
	dcache_pos size;

	/* copies of the constants in the mapped area */
	dcache_pos    dataoffset;
	dcache_uint32 slotuselimit;
	dcache_uint32 elements;
	dcache_pos    maxsize;

	/* copies of the variables in the mapped area */
	dcache_uint32 inuse;
	dcache_pos    n;
	dcache_pos    o;
	dcache_pos    l;
	dcache_uint32 inserted;
	dcache_uint32 deleted;
	dcache_uint32 moves;

	/* transaction support */
	char *t_buf;
	dcache_uint32 t_len;
	dcache_uint32 t_used;
} dcache;

#define dcache_keylen(x) ((x)->keylen)
#define dcache_keypos(x) ((x)->keypos)
#define dcache_datalen(x) ((x)->datalen)
#define dcache_datapos(x) ((x)->datapos)
#define dcache_datatime(x) ((x)->datatime)
#define dcache_fd(x) ((x)->fd)

int dcache_create_fd(int fd,dcache_pos maxsize, 
	dcache_uint32 elements, int do_hole);
int dcache_create_name(const char *fname, int mode,dcache_pos maxsize, 
	dcache_uint32 elements, int do_hole);


int dcache_init(dcache *, int fd, int forwrite);
int dcache_reload_vars(dcache *);
int dcache_store_vars(dcache *);
void dcache_free(dcache *);

int dcache_trans_start(dcache *);
int dcache_trans_commit(dcache *);
int dcache_trans_cancel(dcache *);
int dcache_trans_replay(dcache *d);
int dcache_trans_need_replay(dcache *d);


int dcache_enter(dcache *, const void *key, dcache_reclen keylen,
	dcache_data *data, dcache_time timevalue);

void dcache_lookupstart(dcache *);
int dcache_lookupnext(dcache *, const char *key, dcache_reclen keylen);
int dcache_lookup(dcache *, const char *key, dcache_reclen keylen);

int dcache_walk(dcache *);
void dcache_walkstart(dcache *);

int dcache_delete(dcache *);

int dcache_lck_share(int fd);
int dcache_lck_excl(int fd);
int dcache_lck_tryshare(int fd);
int dcache_lck_tryexcl(int fd);
int dcache_lck_unlock(int fd);
int dcache_flush(dcache *c);
int dcache_sync(dcache *c);
dcache_reclen dcache_read(int rfd,void *vbuf,dcache_reclen len);
int dcache_seek(int fd, dcache_pos l);
void dcache_set_delete_callback(dcache *, dcache_delete_callback);
void dcache_set_autosync(dcache *, int onoff);

#endif
