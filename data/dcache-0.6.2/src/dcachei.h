/*
 * Copyright (C) 2001-2002 Uwe Ohse, uwe@ohse.de
 * This is free software, licensed under the terms of the GNU Lesser
 * General Public License Version 2.1, of which a copy is stored at:
 *    http://www.ohse.de/uwe/licenses/LGPL-2.1
 * Later versions may or may not apply, see 
 *    http://www.ohse.de/uwe/licenses/
 * for information after a newer version has been published.
 */
#ifndef DCACHEI_H
#define DCACHEI_H

/* setting this too low might lead into problems: mmap and read interaction
 * is not well specified. So better safe than sorry */
#define DCACHE_MAX_PAGESIZE (1024*16)
#define DCACHE_RECLEN_MAX (0x7fffffff)
#define DCACHE_UINT32_MAX (0xffffffff)

#define DCACHE_MASTERTABLE 1024 /* offset of mastertable */
#define DCACHE_VERSION        1
#define DCACHE_MAGIC0       0x00444361
#define DCACHE_MAGIC1       0x63686581
#define DCACHE_OFF_MAGIC0   0
#define DCACHE_OFF_MAGIC1   (DCACHE_OFF_MAGIC0 + sizeof(dcache_uint32))
#define DCACHE_OFF_VERSION  (DCACHE_OFF_MAGIC1 + sizeof(dcache_uint32))
#define DCACHE_OFF_ELEMENTS (DCACHE_OFF_VERSION + sizeof(dcache_uint32))
#define DCACHE_OFF_SLOTUSE  (DCACHE_OFF_ELEMENTS + sizeof(dcache_uint32))
#define DCACHE_OFF_MAXSIZE  (DCACHE_OFF_SLOTUSE + sizeof(dcache_uint32))
#define DCACHE_OFF_NEWPTR   (DCACHE_OFF_MAXSIZE + sizeof(dcache_pos))
#define DCACHE_OFF_OLDPTR   (DCACHE_OFF_NEWPTR + sizeof(dcache_pos))
#define DCACHE_OFF_LASTPTR  (DCACHE_OFF_OLDPTR + sizeof(dcache_pos))
#define DCACHE_OFF_DATAOFF  (DCACHE_OFF_LASTPTR + sizeof(dcache_pos))
#define DCACHE_OFF_INUSE    (DCACHE_OFF_DATAOFF + sizeof(dcache_pos))

#define DCACHE_OFF_INSERTED (DCACHE_OFF_INUSE + sizeof(dcache_uint32))
#define DCACHE_OFF_DELETED  (DCACHE_OFF_INSERTED + sizeof(dcache_uint32))
#define DCACHE_OFF_MOVES    (DCACHE_OFF_DELETED + sizeof(dcache_uint32))
#define DCACHE_OFF_END      (DCACHE_OFF_MOVES + sizeof(dcache_uint32))

#define DC_OFF_KEYLEN   0
#define DC_OFF_DATALEN  4
#define DC_OFF_EXPIRE   8
#define DC_OFF_EXPIRE2 12 /* the second half of it */
#define DC_HEADLEN     16

void dcache_hash(dcache_uint32 *pcrc, const char *buf, dcache_reclen len);
void dcache_hash_init(dcache_uint32 *crc);
void dcache_hash_update(dcache_uint32 *pcrc,const char *buf, dcache_reclen len);
void dcache_hash_finish(dcache_uint32 *crc);


dcache_uint32 dcache_reclen_fromdisk (const char *buf, dcache_reclen *);
dcache_uint32 dcache_pos_fromdisk (const char *buf, dcache_pos *);
dcache_uint32 dcache_uint32_fromdisk (const char *buf, 
	dcache_uint32 *);
dcache_uint32 dcache_reclen_todisk (char *buf, dcache_reclen);
dcache_uint32 dcache_pos_todisk (char *buf, dcache_pos);
dcache_uint32 dcache_uint32_todisk (char *buf, dcache_uint32);
dcache_reclen dcache_write(int fd, const void *vbuf, dcache_reclen l);
dcache_reclen dcache_writev(int fd, dcache_data *);

int dcache_trans_enter(dcache *d, const void *key, dcache_reclen keylen,
        dcache_data *data, dcache_time timevalue);
int dcache_trans_delete(dcache *d, const void *key, dcache_reclen keylen,
        dcache_pos pos);

#endif



