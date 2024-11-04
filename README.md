              Mnid - a GTK+ based ID/Password management tool.

  Copyright(C) 2005 NOGUCHI Shoji <noguchi@org3.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

  For more details see the file COPYING.

What's mnid
============

Mnid (Mneme ID) is an ID/Password management tool based on GTK+ GUI toolkit,
and runs on X Window System.

Mnid is a free software distributed under the GNU GPL.

Installation
============

Read the 'INSTALL' file for more detailed directions.

Mnid uses the standard ./configure ; make. You need to use gmake, BSD
make probably won't work. Remember, run ./configure --help to see what
build options are available.

In order to compile Mnid, you need to have GTK+ 2.0, Libxml2 and OpenSSL
installed (as well as the development files!). The configure
script will fail if you don't. You can get it from ...

* GTK+ 2.0   (http://www.gtk.org/)
* Libxml2    (http://xmlsoft.org/)
* OpenSSL    (http://www.openssl.org/)

if you enable GnuPG support using GPGME
* gpgme      (http://www.gnupg.org/related_software/gpgme/index.html)


How to run
==========

You should run 'make install' as root to make sure plugins and other files
get installed into locations they want to be in. Once you've done that,
you only need to run 'mnid'.

Initial statup
--------------

When Mnid is executed for the first time, it automatically creates the
configuration files under ~/.mnid/.


Infomation
==========

You can check the newest version and infomation about Mnid at:

	http://www.org3.net/

Feedback
========

Comments, ideas and (most of all) bug reports (and especially patches) are
very welcome.

--
NOGUCHI Shoji <noguchi@org3.net>
