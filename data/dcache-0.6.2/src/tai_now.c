/* Reimplementation of Daniel J. Bernsteins tai library.
 * (C) 2001 Uwe Ohse, <uwe@ohse.de>.
 *   Report any bugs to <uwe@ohse.de>.
 * Placed in the public domain.
 */
/* @(#) $Id$ */
#include <time.h>
#include "tai.h"

void
tai_now (struct tai *target)
{
	time_t t=time((void *)0);
	tai_unix (target, t);
}
