/* capture_info_dialog.h
 *
 * $Id: capture_info_dialog.h 40378 2012-01-04 22:13:01Z gerald $
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

#ifndef CAPTURE_INFO_DIALOG_H
#define CAPTURE_INFO_DIALOG_H

#include <QDialog>

class CaptureInfoDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CaptureInfoDialog(QWidget *parent = 0);

signals:

public slots:

};

#endif // CAPTURE_INFO_DIALOG_H
