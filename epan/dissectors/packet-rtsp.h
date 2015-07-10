/* packet-rtsp.h
 *
 * $Id: packet-rtsp.h 37741 2011-06-21 12:45:37Z etxrab $
 * Liberally copied from packet-http.h, 
 * by Stephane GORSE (Orange Labs / France Telecom)
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

#ifndef __PACKET_RTSP_H__
#define __PACKET_RTSP_H__

/* Used for RTSP statistics */
typedef struct _rtsp_info_value_t {
	guint32 framenum;
	gchar	*request_method;
	guint	 response_code;
	gchar   *rtsp_host;
	gchar   *request_uri;
} rtsp_info_value_t;

WS_VAR_IMPORT const value_string rtsp_status_code_vals[];

#endif /* __PACKET_RTSP_H__ */
