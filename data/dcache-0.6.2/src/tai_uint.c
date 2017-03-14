/* Reimplementation of Daniel J. Bernsteins tai library.
 * (C) 2001 Uwe Ohse, <uwe@ohse.de>.
 *   Report any bugs to <uwe@ohse.de>.
 * Placed in the public domain.
 */
/* @(#) $Id$ */
#include "tai.h"

void
tai_uint (struct tai *target, unsigned int ui)
{
	target->x = ui;
}
