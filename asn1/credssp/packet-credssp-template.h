/* packet-credssp.h
 * Routines for CredSSP (Credential Security Support Provider) packet dissection
 * Graeme Lunt 2011
 *
 * $Id: packet-credssp-template.h 39196 2011-09-30 15:21:16Z gal $
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

#ifndef PACKET_CREDSSP_H
#define PACKET_CREDSSP_H

#include "packet-credssp-val.h"

void proto_reg_handoff_credssp(void);
void proto_register_credssp(void);

#endif  /* PACKET_CREDSSP_H */
