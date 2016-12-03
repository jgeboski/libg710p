if ENABLE_TOOLS

bin_PROGRAMS = g710p-keys

LIBG710P_CFLAGS = \
	-I$(top_builddir)/libg710p

LIBG710P_HIDRAW_LDADD = \
	$(top_builddir)/libg710p/libg710p-hidraw.la

G710P_TOOLS_COMMON_SOURCES = \
	g710p-tools-common.c \
	g710p-tools-common.h

g710p_keys_CFLAGS = $(LIBG710P_CFLAGS)
g710p_keys_LDADD = $(LIBG710P_HIDRAW_LDADD)
g710p_keys_SOURCES = \
	$(G710P_TOOLS_COMMON_SOURCES) \
	g710p-keys.c

if ENABLE_PULSEAUDIO_TOOL

bin_PROGRAMS += g710p-pulseaudio

g710p_pulseaudio_CFLAGS = \
	$(LIBG710P_CFLAGS) \
	$(LIBPULSE_CFLAGS)

g710p_pulseaudio_LDADD = \
	$(LIBG710P_HIDRAW_LDADD)

g710p_pulseaudio_LDFLAGS = $(LIBPULSE_LIBS)
g710p_pulseaudio_SOURCES = \
	$(G710P_TOOLS_COMMON_SOURCES) \
	g710p-pulseaudio.c

endif # ENABLE_PULSEAUDIO_TOOL

endif # ENABLE_TOOLS
