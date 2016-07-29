AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\"

bin_PROGRAMS = garu
garu_SOURCES = \
	garu.c \
	garu-application.c \
	garu-window.c \
	garu-song-box.c \
	garu-headerbar.c \
	garu-player.c \
	garu-playbin.c \
	garu-tagger.c \
	garu-playlist.c \
	garu-library.c \
	garu-folder-browser.c \
	garu-utils.c \
	garu-preferences.c \
	garu-effects.c \
	garu-about-dialog.c
garu_CFLAGS = \
	$(GTK_CFLAGS) \
	$(GSTREAMER_CFLAGS) \
	$(GSTREAMERUTILS_CFLAGS) \
	$(TAGLIB_C_CFLAGS)

garu_LDADD = \
	$(GTK_LIBS) \
	$(GSTREAMER_LIBS) \
	$(GSTREAMERUTILS_LIBS) \
	$(TAGLIB_C_LIBS) \
	$(LIBINTL)
