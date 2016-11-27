if ENABLE_TOOLS

bin_PROGRAMS = g710p-keys

g710p_keys_CFLAGS = \
	-I$(top_builddir)/libg710p

g710p_keys_LDADD = \
	$(top_builddir)/libg710p/libg710p-hidraw.la

g710p_keys_SOURCES = \
	g710p-keys.c

endif # ENABLE_TOOLS
