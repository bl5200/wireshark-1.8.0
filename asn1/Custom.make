# Automake file for Wireshark
#
# $Id: Custom.make 39340 2011-10-10 18:26:33Z etxrab $
#
# Wireshark - Network traffic analyzer
# By Gerald Combs <gerald@wireshark.org>
# Copyright 2006 Gerald Combs
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# Add custom asn1 directories here, the file is included by Makefile.am
# Note you need to add a Cutom.m4 file too.
#
CUSTOM_SUBDIRS = 

# Add custom dirs here for BER dissectors on Windows
CUSTOM_BER_LIST = 

# Add custom dirs here for PER dissectors on Windows
CUSTOM_PER_LIST =