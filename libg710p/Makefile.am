lib_LTLIBRARIES = \
	libg710p-hidraw.la \
	libg710p-libusb.la

include_HEADERS = \
	g710p.h

LIBG710P_SOURCES = \
	$(include_HEADERS) \
	g710p.c

libg710p_hidraw_la_CFLAGS = $(HIDAPI_HIDRAW_CFLAGS)
libg710p_hidraw_la_LIBADD = $(HIDAPI_HIDRAW_LIBS)
libg710p_hidraw_la_SOURCES = $(LIBG710P_SOURCES)

libg710p_libusb_la_CFLAGS = $(HIDAPI_LIBUSB_CFLAGS)
libg710p_libusb_la_LIBADD = $(HIDAPI_LIBUSB_LIBS)
libg710p_libusb_la_SOURCES = $(LIBG710P_SOURCES)

DISTCLEANFILES = \
	libg710p-hidraw.pc \
	libg710p-libusb.pc

EXTRA_DIST = \
	libg710p-hidraw.pc.in \
	libg710p-libusb.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	libg710p-hidraw.pc \
	libg710p-libusb.pc
