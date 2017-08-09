
standarddir = $(includedir)/kbs_include

standard_headers = \
	kbs_Alphabet.h \
	kbs_Error.h \
	kbs_Limits.h \
	kbs_Math.h \
	kbs_String.h \
	kbs_Time.h \
	kbs_Types.h

standard_modules = \
	kbs_Alphabet.c \
	kbs_Error.c \
	kbs_Math.c \
	kbs_String.c \
	kbs_Time.c

noinst_LTLIBRARIES = libstandard.la

libstandard_la_SOURCES = \
	$(standard_headers) \
	$(standard_modules)

libstandard_la_LIBADD = \
	@top_srcdir@/src/Extern/libextern.la

libstandard_la_CFLAGS = \
	-I@top_srcdir@/src/Extern
