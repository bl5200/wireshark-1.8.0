# Makefile.am
# Automake file for Wireshark plugins
#
# $Id: Makefile.am 42974 2012-06-01 15:57:05Z etxrab $
#
# Wireshark - Network traffic analyzer
# By Gerald Combs <gerald@wireshark.org>
# Copyright 1998 Gerald Combs
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

-include Custom.make
SUBDIRS = $(_CUSTOM_SUBDIRS_) \
	asn1 \
	docsis \
	ethercat \
	gryphon \
	irda \
	m2m \
	mate \
	opcua \
	profinet \
	stats_tree \
	unistim \
	wimax \
	wimaxasncp

plugindir = @plugindir@

EXTRA_DIST = \
	$(_CUSTOM_EXTRA_DIST_) \
	plugins.vcproj	\
	Makefile.nmake

MAINTAINERCLEANFILES = \
	Makefile.in

checkapi:
	for i in $(SUBDIRS); do \
		(cd "$$i"; $(MAKE) checkapi); \
	done

