/* packet-imap.c
 * Routines for imap packet dissection
 * Copyright 1999, Richard Sharpe <rsharpe@ns.aus.com>
 *
 * $Id: packet-imap.c 39551 2011-10-25 01:28:00Z morriss $
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * Copied from packet-tftp.c
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <epan/packet.h>
#include <epan/strutil.h>
#include "packet-ssl.h"

static int proto_imap = -1;
static int hf_imap_isrequest = -1;
static int hf_imap_line = -1;
static int hf_imap_request = -1;
static int hf_imap_request_tag = -1;
static int hf_imap_response = -1;
static int hf_imap_response_tag = -1;

static gint ett_imap = -1;
static gint ett_imap_reqresp = -1;

#define TCP_PORT_IMAP			143
#define TCP_PORT_SSL_IMAP		993

static void
dissect_imap(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree)
{
	gboolean        is_request;
	proto_tree      *imap_tree, *reqresp_tree;
	proto_item      *ti, *hidden_item;
	gint		offset = 0;
	const guchar	*line;
	gint		next_offset;
	int		linelen;
	int		tokenlen;
	const guchar	*next_token;

	col_set_str(pinfo->cinfo, COL_PROTOCOL, "IMAP");


	if (pinfo->match_uint == pinfo->destport)
		is_request = TRUE;
	else
		is_request = FALSE;

	if (check_col(pinfo->cinfo, COL_INFO)) {
		/*
		 * Put the first line from the buffer into the summary
		 * (but leave out the line terminator).
		 */
		linelen = tvb_find_line_end(tvb, offset, -1, &next_offset, FALSE);
		line = tvb_get_ptr(tvb, offset, linelen);

		col_add_fstr(pinfo->cinfo, COL_INFO, "%s: %s",
			     is_request ? "Request" : "Response",
			     format_text(line, linelen));
	}

	if (tree) {
		ti = proto_tree_add_item(tree, proto_imap, tvb, offset, -1, ENC_NA);
		imap_tree = proto_item_add_subtree(ti, ett_imap);

		hidden_item = proto_tree_add_boolean(imap_tree, hf_imap_isrequest, tvb, 0, 0, is_request);
		PROTO_ITEM_SET_HIDDEN(hidden_item);

		while(tvb_length_remaining(tvb, offset) > 2) {

			/*
			 * Find the end of each line
			 *
			 * Note that "tvb_find_line_end()" will return a value that is
			 * not longer than what's in the buffer, so the "tvb_get_ptr()"
			 * call won't throw an exception.
			 */
			linelen = tvb_find_line_end(tvb, offset, -1, &next_offset, FALSE);
			line = tvb_get_ptr(tvb, offset, linelen);

			/*
			 * Put the line into the protocol tree.
			 */
			ti = proto_tree_add_item(imap_tree, hf_imap_line, tvb, offset,
					next_offset - offset, ENC_ASCII|ENC_NA);

			reqresp_tree = proto_item_add_subtree(ti, ett_imap_reqresp);

			/*
			 * Show each line as tags + requests or replies.
			 */

			/*
			 * Extract the first token, and, if there is a first
			 * token, add it as the request or reply tag.
			 */
			tokenlen = get_token_len(line, line + linelen, &next_token);
			if (tokenlen != 0) {
				proto_tree_add_item(reqresp_tree, (is_request) ? hf_imap_request_tag : hf_imap_response_tag,
									tvb, offset, tokenlen, ENC_ASCII|ENC_NA);

				offset += (gint) (next_token - line);
				linelen -= (int) (next_token - line);
				line = next_token;
			}

			/*
			 * Add the rest of the line as request or reply data.
			 */
			if (linelen != 0) {
				proto_tree_add_item(reqresp_tree, (is_request) ? hf_imap_request : hf_imap_response,
									tvb, offset, linelen, ENC_ASCII|ENC_NA);
			}

			offset += linelen+2; /* Skip over last line and \r\n at the end of it */

		}
	}
}

void
proto_register_imap(void)
{
	static hf_register_info hf[] = {
		{ &hf_imap_isrequest, { "Request", "imap.isrequest", FT_BOOLEAN, BASE_NONE, NULL, 0x0, "TRUE if IMAP request, FALSE otherwise", HFILL }},
		{ &hf_imap_line, { "Line", "imap.line", FT_STRINGZ, BASE_NONE, NULL, 0x0, "A line of an IMAP message", HFILL }},
		{ &hf_imap_request, { "Request", "imap.request", FT_STRINGZ, BASE_NONE, NULL, 0x0, "Remainder of request line", HFILL }},
		{ &hf_imap_request_tag, { "Request Tag", "imap.request_tag", FT_STRINGZ, BASE_NONE, NULL, 0x0, "First token of request line", HFILL }},
		{ &hf_imap_response, { "Response", "imap.response", FT_STRINGZ, BASE_NONE, NULL, 0x0, "Remainder of response line", HFILL }},
		{ &hf_imap_response_tag, { "Response Tag", "imap.response_tag", FT_STRINGZ, BASE_NONE, NULL, 0x0, "First token of response line", HFILL }}
	};

	static gint *ett[] = {
	&ett_imap,
	&ett_imap_reqresp,
	};

	proto_imap = proto_register_protocol("Internet Message Access Protocol",
					   "IMAP", "imap");
	register_dissector("imap", dissect_imap, proto_imap);
	proto_register_field_array(proto_imap, hf, array_length(hf));
	proto_register_subtree_array(ett, array_length(ett));
}

void
proto_reg_handoff_imap(void)
{
	dissector_handle_t imap_handle;

	imap_handle = create_dissector_handle(dissect_imap, proto_imap);
	dissector_add_uint("tcp.port", TCP_PORT_IMAP, imap_handle);
	ssl_dissector_add(TCP_PORT_SSL_IMAP, "imap", TRUE);
}
