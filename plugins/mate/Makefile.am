# Makefile.am
# Automake file for MATE Wireshark plugin
#
# $Id: Makefile.am 42972 2012-06-01 14:44:35Z wmeier $
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
#

include $(top_srcdir)/Makefile.am.inc

INCLUDES = -I$(top_srcdir)

include Makefile.common

plugindir = @plugindir@

plugin_LTLIBRARIES = mate.la

mate_la_SOURCES = \
	plugin.c	\
	moduleinfo.h	\
	$(DISSECTOR_SRC)	\
	$(DISSECTOR_SUPPORT_SRC)	\
	$(DISSECTOR_INCLUDES)

mate_la_LDFLAGS = -module -avoid-version
mate_la_LIBADD = @PLUGIN_LIBS@

# Libs must be cleared, or else libtool won't create a shared module.
# If your module needs to be linked against any particular libraries,
# add them here.
LIBS =

#
# Build plugin.c, which contains the plugin version[] string, a
# function plugin_register() that calls the register routines for all
# protocols, and a function plugin_reg_handoff() that calls the handoff
# registration routines for all protocols.
#
# We do this by scanning sources.  If that turns out to be too slow,
# maybe we could just require every .o file to have an register routine
# of a given name (packet-aarp.o -> proto_register_aarp, etc.).
#
# Formatting conventions:  The name of the proto_register_* routines an
# proto_reg_handoff_* routines must start in column zero, or must be
# preceded only by "void " starting in column zero, and must not be
# inside #if.
#
# DISSECTOR_SRC is assumed to have all the files that need to be scanned.
#
# For some unknown reason, having a big "for" loop in the Makefile
# to scan all the files doesn't work with some "make"s; they seem to
# pass only the first few names in the list to the shell, for some
# reason.
#
# Therefore, we have a script to generate the plugin.c file.
# The shell script runs slowly, as multiple greps and seds are run
# for each input file; this is especially slow on Windows.  Therefore,
# if Python is present (as indicated by PYTHON being defined), we run
# a faster Python script to do that work instead.
#
# The first argument is the directory in which the source files live.
# The second argument is "plugin", to indicate that we should build
# a plugin.c file for a plugin.
# All subsequent arguments are the files to scan.
#
plugin.c: $(DISSECTOR_SRC) Makefile.common $(top_srcdir)/tools/make-dissector-reg \
    $(top_srcdir)/tools/make-dissector-reg.py
	@if test -n "$(PYTHON)"; then \
		echo Making plugin.c with python ; \
		$(PYTHON) $(top_srcdir)/tools/make-dissector-reg.py $(srcdir) \
		    plugin $(DISSECTOR_SRC) ; \
	else \
		echo Making plugin.c with shell script ; \
		$(top_srcdir)/tools/make-dissector-reg $(srcdir) \
		    $(plugin_src) plugin $(DISSECTOR_SRC) ; \
	fi

#
# Currently plugin.c can be included in the distribution because
# we always build all protocol dissectors. We used to have to check
# whether or not to build the snmp dissector. If we again need to
# variably build something, making plugin.c non-portable, uncomment
# the dist-hook line below.
#
# Oh, yuk.  We don't want to include "plugin.c" in the distribution, as
# its contents depend on the configuration, and therefore we want it
# to be built when the first "make" is done; however, Automake insists
# on putting *all* source into the distribution.
#
# We work around this by having a "dist-hook" rule that deletes
# "plugin.c", so that "dist" won't pick it up.
#
#dist-hook:
#	@rm -f $(distdir)/plugin.c

CLEANFILES = \
	mate \
	*~

DISTCLEANFILES = \
	mate_grammar.out	\
	mate_grammar.c		\
	mate_grammar.h

MAINTAINERCLEANFILES = \
	Makefile.in		\
	mate_parser.c		\
	mate_parser_lex.h	\
	plugin.c

EXTRA_DIST = \
	mate_grammar.lemon	\
	mate_parser.l		\
	Makefile.nmake		\
	Makefile.common		\
	moduleinfo.nmake	\
	plugin.rc.in		\
	CMakeLists.txt

RUNLEX = $(top_srcdir)/tools/runlex.sh

mate_parser_lex.h : mate_parser.c

LEMON = $(top_builddir)/tools/lemon

mate_grammar.h : mate_grammar.c
mate_grammar.c : mate_grammar.lemon mate.h mate_util.h $(LEMON)/lemon$(EXEEXT)
	$(LEMON)/lemon$(EXEEXT) t=$(srcdir)/$(LEMON)/lempar.c $(srcdir)/mate_grammar.lemon || \
		(rm -f grammar.c grammar.h ; false)

checkapi:
	$(PERL) $(top_srcdir)/tools/checkAPIs.pl -g abort -g termoutput -build $(DISSECTOR_SRC) $(DISSECTOR_INCLUDES)
