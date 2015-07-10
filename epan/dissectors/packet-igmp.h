/* packet-igmp.h   2001 Ronnie Sahlberg <See AUTHORS for email>
 * Declarations of routines for IGMP packet disassembly
 *
 * $Id: packet-igmp.h 18196 2006-05-21 04:49:01Z sahlberg $
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
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
 */

#ifndef __PACKET_IGMP_H__
#define __PACKET_IGMP_H__

void igmp_checksum(proto_tree *tree, tvbuff_t *tvb, int hf_index,
    int hf_index_bad, packet_info *pinfo, guint len);

#endif

