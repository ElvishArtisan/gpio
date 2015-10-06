## automake.am
##
## utils Automake.am for gpio
##
## Use automake to process this into a Makefile.in
##
##   (C) Copyright 2007 Fred Gleason <fredg@paravelsystems.com>
##
##    $Id: Makefile.am,v 1.1 2007/11/07 20:36:32 fredg Exp $
##
##   This program is free software; you can redistribute it and/or modify
##   it under the terms of the GNU Library General Public License 
##   version 2 as published by the Free Software Foundation.
##
##   This program is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##   GNU General Public License for more details.
##
##   You should have received a copy of the GNU General Public
##   License along with this program; if not, write to the Free Software
##   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
##

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\" -DQTDIR=\"@QT_DIR@\" @QT_CXXFLAGS@
INCLUDES = -I$(top_srcdir)/drv
LIBS = @QT_LIBS@
MOC = @QT_MOC@

# QT's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	$(MOC) $< -o $@


if GPIO_AM
bin_PROGRAMS = gpitest gpotest 

dist_gpitest_SOURCES = gpitest.cpp gpitest.h\
                       rgpio.cpp rgpio.h\
                       rconf.cpp rconf.h
nodist_gpitest_SOURCES = moc_gpitest.cpp moc_rgpio.cpp
gpitest_LDADD = -lqui

dist_gpotest_SOURCES = gpotest.cpp gpotest.h\
                       rgpio.cpp rgpio.h\
                       rconf.cpp rconf.h
nodist_gpotest_SOURCES = moc_gpotest.cpp moc_rgpio.cpp
gpotest_LDADD = -lqui
else
bin_PROGRAMS = 
endif # GPIO_AM

CLEANFILES = *~ moc_* *.tar.gz *.obj *.exe *.idb *.pdb *.ilk *.qm
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 moc_* *.tar.gz
