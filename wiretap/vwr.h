/* vwr.h
 *
 * $Id: vwr.h 42156 2012-04-20 12:09:16Z martinm $
 *
 * Wiretap Library
 * Copyright (c) 1998-2010 by Tom Alexander <talexander@ixiacom.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef __VWR_H__
#define __VWR_H__

int vwr_open(wtap *wth, int *err, gchar **err_info);

#endif
