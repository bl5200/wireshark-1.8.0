/* Do not modify this file.                                                   */
/* It is created automatically by the ASN.1 to Wireshark dissector compiler   */
/* packet-ocsp.h                                                              */
/* ../../tools/asn2wrs.py -b -p ocsp -c ./ocsp.cnf -s ./packet-ocsp-template -D . -O ../../epan/dissectors OCSP.asn */

/* Input file: packet-ocsp-template.h */

#line 1 "../../asn1/ocsp/packet-ocsp-template.h"
/* packet-ocsp.h
 * Routines for Online Certificate Status Protocol (RFC2560) packet dissection
 *  Ronnie Sahlberg 2004
 *
 * $Id: packet-ocsp.h 39427 2011-10-15 19:27:27Z wmeier $
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

#ifndef PACKET_OCSP_H
#define PACKET_OCSP_H

/*#include "packet-ocsp-exp.h"*/

extern int proto_ocsp;
int dissect_ocsp_OCSPResponse(gboolean implicit_tag, tvbuff_t *tvb, int offset, asn1_ctx_t *actx, proto_tree *tree, int hf_index);

#endif  /* PACKET_OCSP_H */

