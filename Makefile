#   Rock Dodger! Avoid the rocks as long as you can!
#   Copyright (C) 2001- Paul Holt <pcholt@gmail.com>,
#                 2010- Robert P Krawczyk <rpkrawczyk@gmail.com>

#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.

#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

profile=0
debug=0

VMAJOR=1
VMINOR=1
VRELEASE=4
VERSION=$(VMAJOR).$(VMINOR).$(VRELEASE)
PACKAGENAME=rockdodger

prefix = $(DESTDIR)/usr/local
INSTALL=/usr/bin/install
TMP=/tmp

exec_prefix = $(prefix)
datarootdir = $(prefix)/share
datadir = $(datarootdir)
bindir = $(exec_prefix)/bin
docdir = $(datarootdir)/doc/$(PACKAGENAME)
mandir = $(datarootdir)/man
localstatedir = $(prefix)/var
gamesdir = $(localstatedir)/games

NEWD=$(PACKAGENAME)-$(VERSION)
COMPILEDATE=$(shell date --date=@$${SOURCE_DATE_EPOCH:-$$(date +%s)} '+%Y-%m-%d')
TARPKGNAME=$(NEWD).pkg.tgz
OPTIONS=-DVERSION=\"$(VERSION)\" -DCOMPILEDATE=\"$(COMPILEDATE)\" -Wall -I../SDL/h
EXENAME=rockdodger.exe

SOUNDLIBRARIES=-lSDL_mixer
SDL_CONFIG=sdl-config

LIBRARIES=-L../SDL/lib -lSDL_image -lsdl12 $(SOUNDLIBRARIES) -lm
OBJECTS=SFont.o guru_meditation.o signal_handling.o random_gen.o datafun.o sound.o input_functions.o scroller.o display_subsystem.o \
	game_state.o highscore_io.o sprite.o \
	blubats.o greeblies.o powerup.o rocks.o spacedots.o ship.o engine_exhaust.o laser.o \
	mood_item.o sparkles.o yellifish.o infoscreen.o \
	u-iff.o sekrit_lab.o\
	globals.o intro.o main.o
MANPAGE=rockdodger.6

ifeq ($(profile),1)
	OPTIONS+=-pg
endif
ifeq ($(debug),0)
	OPTIONS+=-O2 -DNDEBUG
	ifneq ($(profile),1)
		OPTIONS+=-fomit-frame-pointer
	endif
else
	OPTIONS+=-O1 -DDEBUG
endif
CFLAGS=-MMD -g $(OPTIONS) $(MOREOPTS)

all:	$(EXENAME) config.h

config.h: config.h.in
	sed -e 's:@gamesdir@:$(gamesdir):' -e 's/@PACKAGENAME@/$(PACKAGENAME)/' -e 's:@datadir@:$(datadir):' $^ > $@

$(EXENAME):	config.h $(OBJECTS)
ifeq ($(profile),1)
	$(CC) -pg -o $(EXENAME) $(OBJECTS) $(LIBRARIES)
else
	$(CC) $(LDFLAGS) -Zomf -o $(EXENAME) $(OBJECTS) $(LIBRARIES)
endif

clean:
	rm -f *.o *.d $(EXENAME) config.h TAGS

dist:
	$(MAKE) clean
	mkdir -p $(TMP)/$(NEWD)/data
	cp *.c *.h *.in Makefile COPYING *.xpm $(MANPAGE) rockdodger.desktop $(TMP)/$(NEWD)
	cp -r data/* $(TMP)/$(NEWD)/data
	( cd $(TMP);  tar zcf $(NEWD).tar.gz $(NEWD) )
	rm -rf $(TMP)/$(NEWD)
	mv $(TMP)/$(NEWD).tar.gz .

pkg:	$(EXENAME)
	(DTEMP=`mktemp -d $(TMP)/rock.XXXXX`; $(MAKE) install DESTDIR=$$DTEMP; echo $$DTEMP; \
	tar -C $$DTEMP -cvzf $(TARPKGNAME) . && rm -r $$DTEMP)

install:	all
	$(INSTALL) -d $(bindir) $(gamesdir)
	$(INSTALL) $(EXENAME) $(bindir)
	-chgrp games $(bindir)/$(EXENAME)
	chmod g+s $(bindir)/$(EXENAME)
	$(MAKE) -C data install
	touch $(gamesdir)/rockdodger.scores
	-chgrp games $(gamesdir)/rockdodger.scores
	chmod g+rw $(gamesdir)/rockdodger.scores
	mkdir -p $(mandir)/man6
	gzip -9n < $(MANPAGE) > $(mandir)/man6/$(MANPAGE).gz

uninstall:
	rm -f $(gamesdir)/rockdodger.scores
	rm -f $(bindir)/$(EXENAME)
	rm -f $(mandir)/man6/$(MANPAGE).gz
	$(MAKE) -C data uninstall

tags:
	etags *.c *.h >TAGS

.PHONY: all clean install uninstall dist pkg tags
-include $(OBJECTS:.o=.d)
