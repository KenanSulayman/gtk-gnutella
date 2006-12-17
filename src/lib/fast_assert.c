/*
 * $Id$
 *
 * Copyright (c) 2006, Christian Biere
 *
 *----------------------------------------------------------------------
 * This file is part of gtk-gnutella.
 *
 *  gtk-gnutella is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  gtk-gnutella is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gtk-gnutella; if not, write to the Free Software
 *  Foundation, Inc.:
 *      59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *----------------------------------------------------------------------
 */

/**
 * @ingroup lib
 * @file
 *
 * Fast assertions.
 *
 * @author Christian Biere
 * @date 2006
 */

#include "common.h"

RCSID("$Id$")

#include "lib/fast_assert.h"
#include "lib/override.h"			/* Must be the last header included */

/**
 * @note For maximum safety this is kept signal-safe, so that we can
 *       even use assertions in signal handlers. See also:
 * http://www.opengroup.org/onlinepubs/009695399/functions/xsh_chap02_04.html
 */
void NON_NULL_PARAM((1)) REGPARM(1)
assertion_warning(const assertion_data * const data)
{
	char line_buf[22], *line_ptr = &line_buf[sizeof line_buf - 1];
	struct iovec iov[16];
	guint n = 0;

	{
		unsigned line_no = data->line;

		*line_ptr = '\0';
		do {
			*--line_ptr = (line_no % 10) + '0';
			line_no /= 10;
		} while (line_no && line_ptr != line_buf);
	}

#define print_str(x) \
G_STMT_START { \
	if (n < G_N_ELEMENTS(iov)) { \
		const char *ptr = (x); \
		iov[n].iov_base = (char *) ptr; \
		iov[n].iov_len = strlen(ptr); \
		n++; \
	} \
} G_STMT_END

	if (data->expr) {
		print_str("Assertion failure (");
	} else {
		print_str("Code should not have been reached (");
	}
	print_str(data->file);
	print_str(":");
	print_str(line_ptr);
	print_str(")");
	if (data->expr) {
		print_str(" \"");
		print_str(data->expr);
		print_str("\"");
	}
	print_str("\n");
	writev(STDERR_FILENO, iov, n);
}

void G_GNUC_NORETURN NON_NULL_PARAM((1)) REGPARM(1)
assertion_failure(const assertion_data * const data)
{
	assertion_warning(data);
	abort();
}

/* vi: set ts=4 sw=4 cindent: */
