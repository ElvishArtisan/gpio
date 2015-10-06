## Makefile.am
##
##    GPIO Driver for MeasurementComputing Cards
##    Copyright (C) 2002-2003  Fred Gleason <fredg@paravelsystems.com>
##
##    This program is free software; you can redistribute it and/or modify
##    it under the terms of version 2 of the GNU General Public License as
##    published by the Free Software Foundation;
##
##    This program is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU General Public License for more details.
##
##    You should have received a copy of the GNU General Public License
##    along with this program; if not, write to the Free Software
##    Foundation, Inc., 59 Temple Place, Suite 330, 
##    Boston, MA  02111-1307  USA
##
## Use automake to process this into a Makefile.in
##

KERNELDIR = /lib/modules/`uname -r`/build/include
INCLUDE_DIR = /usr/include/linux
INIT_DIR = /etc/init.d

CFLAGS = -O
KFLAGS = -D__KERNEL__ -DMODULE -I$(KERNELDIR)

OBJS = gpio.o
all:	$(OBJS)
gpio.o:	gpio.c gpio.h
	$(CC) $(CFLAGS) $(KFLAGS) @KERNELFLAG@ -o gpio.o -c gpio.c

install-exec-local:	gpio.o
	cp gpio.o /lib/modules/`uname -r`/extra/
	cp gpio.h $(INCLUDE_DIR)/
	mkdir -p $(INIT_DIR)
	cp gpio $(INIT_DIR)
	chmod 755 $(INIT_DIR)/gpio

uninstall:	
	rm -f /lib/modules/`uname -r`/extra/gpio.o
	rm -f $(INCLUDE_DIR)/gpio.h
	rm -f $(INIT_DIR)/gpio

clean:
	rm -f $(OBJS) core *~

rpm:	dist
	cp gpio-$(VERSION).tar.gz $(RPM_ROOT)/SOURCES/
	$(RPMBUILD) -ba --target $(ARCH)-$(VENDOR)-linux gpio.spec
	mv @RPM_ROOT@/RPMS/$(ARCH)/gpio-@VERSION@-@RPM_RELEASE@.$(ARCH).rpm ./gpio-@VERSION@_`uname -r`-@RPM_RELEASE@.$(ARCH).rpm
	mv @RPM_ROOT@/SRPMS/gpio-@VERSION@-@RPM_RELEASE@.src.rpm .
	rm @RPM_ROOT@/SOURCES/gpio-@VERSION@.tar.gz
	rm -rf @RPM_ROOT@/BUILD/gpio-@VERSION@

slack:	all
	./make_slack

slax:	all
	rm -f gpio-$(VERSION)-$(ARCH)-$(RPM_RELEASE).mo
	./make_slack
	tgz2mo gpio-$(VERSION)-$(ARCH)-$(RPM_RELEASE).tgz gpio-$(VERSION)-$(ARCH)-$(RPM_RELEASE).mo

EXTRA_DIST = gpio.h gpio.c Makefile-2.6.in

CLEANFILES = *~ *.o .tmp_versions *.ko *.mod.* .gpio* *.tar.gz *.tgz *.mo\
             *.rpm
DISTCLEANFILES = Makefile-2.6
MAINTAINERCLEANFILES = *~ Makefile.in
