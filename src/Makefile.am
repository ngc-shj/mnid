bin_PROGRAMS = mnid

mnid_SOURCES = \
	defines.h \
	version.h \
	main.c main.h \
	mainwindow.c mainwindow.h \
	prefs.c prefs.h \
	prefs_common.c prefs_common.h \
	prefs_id.c prefs_id.h \
	prefs_pass.c prefs_pass.h \
	id.c id.h \
	pass.c pass.h \
	pass_list.c pass_list.h \
	about.c about.h \
	utils.c utils.h \
	gtkutils.c gtkutils.h \
	gnupg_crypt.c gnupg_crypt.h \
	gnupg_pass.c gnupg_pass.h \
	gnupg_select_key.c gnupg_select_key.h \
	blowfish_crypt.c blowfish_crypt.h \
	blowfish_key.c blowfish_key.h \
	blowfish_input_key.c blowfish_input_key.h \
	blowfish.c blowfish.h 

EXTRA_DIST = \
	version.h.in

INCLUDES = \
	$(GTK_CFLAGS) \
	$(GDK_PIXBUF_CFLAGS) \
	$(XML_CPPFLAGS) \
	$(GPGME_CFLAGS) \
	-I$(includedir)

mnid_LDADD = \
	$(INTLLIBS) \
	$(GTK_LIBS) \
	$(XML_LIBS) \
	$(GPGME_LIBS) \
	$(LIBICONV)

AM_CPPFLAGS = \
	-Wall \
	-DG_LOG_DOMAIN=\"Mnid\" \
	-DLOCALE_DIR=\""$(localedir)"\" \
	-DPIXMAPS_DIR=\""$(datadir)/pixmaps"\" \
	-DCRACKLIB_DICTPATH=\""$(CRACKLIB_DICTPATH)"\" \
	-DGTK_DISABLE_DEPRECATED=1 \
	-DG_DISABLE_DEPRECATED=1
