lib_LTLIBRARIES = libiso-codes-locale-resolver.la

libiso_codes_locale_resolver_la_CFLAGS = \
	$(DEP_CFLAGS) -DISO_CODES_PREFIX=\"$(ISO_CODES_PREFIX)\"

libiso_codes_locale_resolver_la_LDFLAGS = -Wl,--as-needed -Wl,--no-undefined 
libiso_codes_locale_resolver_la_LIBADD = $(DEP_LIBS)

libiso_codes_locale_resolver_la_SOURCES = locale-resolver.c

MAINTAINERCLEANFILES = Makefile.in
