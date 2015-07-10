/* main_toolbar.h
 * Definitions for toolbar utility routines
 * Copyright 2003, Ulf Lamping <ulf.lamping@web.de>
 *
 * $Id: main_toolbar.h 43283 2012-06-16 18:17:12Z guy $
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

#ifndef __TOOLBAR_H__
#define __TOOLBAR_H__


/** @file
 *  The main toolbar.
 *  @ingroup main_window_group
 */

/** Create the main toolbar.
 * @return the new toolbar
 */
GtkWidget *toolbar_new(void);

/** Redraw the main toolbar. Used, when user changes preferences. */
void toolbar_redraw_all(void);

/** We have (or don't have) a capture in progress now.
 *
 * @param have_capture_file TRUE, if we have a capture in progress file
 */
void set_toolbar_for_capture_in_progress(gboolean have_capture_file);

/** The capture is in the process of being stopped.
 */
void set_toolbar_for_capture_stopping(void);

/** We have (or don't have) captured packets now.
 *
 * @param have_captured_packets TRUE, if we have captured packets
 */
void set_toolbar_for_captured_packets(gboolean have_captured_packets);

/** The packet history has changed, we need to update the menu.
 *
 * @param back_history some back history entries available
 * @param forward_history some forward history entries available
 */
void set_toolbar_for_packet_history(gboolean back_history, gboolean forward_history);

/** Set object data of some buttons (where needed). It's needed so callback 
 *  functions can read back their required data. Acts like g_object_set_data() 
 *  on multiple buttons.
 *
 * @param key the key
 * @param data the data to set
 */
void set_toolbar_object_data(gchar *key, gpointer data);

#endif /* __TOOLBAR_H__ */
