/*
 * Copyright (C) 2001-2002 Uwe Ohse, uwe@ohse.de
 * This is free software, licensed under the terms of the GNU Lesser
 * General Public License Version 2.1, of which a copy is stored at:
 *    http://www.ohse.de/uwe/licenses/LGPL-2.1
 * Later versions may or may not apply, see 
 *    http://www.ohse.de/uwe/licenses/
 * for information after a newer version has been published.
 */
#include <unistd.h>
#include <fcntl.h>
#include "dcache.h"

static int
lock_region (int fd, int cmd, int typ, off_t start, int whence, off_t len)
{
	struct flock lock;
	lock.l_type = typ;
	lock.l_start = start;
	lock.l_len = len;
	lock.l_whence = whence;
	return fcntl (fd, cmd, &lock);
}

int dcache_lck_share(int fd)
{ return lock_region(fd,F_SETLKW, F_RDLCK, 0, SEEK_SET, 0); }
int dcache_lck_excl(int fd)
{ return lock_region(fd,F_SETLKW, F_WRLCK, 0, SEEK_SET, 0); }
int dcache_lck_tryshare(int fd)
{ return lock_region(fd,F_SETLK, F_RDLCK, 0, SEEK_SET, 0); }
int dcache_lck_tryexcl(int fd)
{ return lock_region(fd,F_SETLK, F_WRLCK, 0, SEEK_SET, 0); }
int dcache_lck_unlock(int fd)
{ return lock_region(fd,F_SETLK, F_UNLCK, 0, SEEK_SET, 0); }

