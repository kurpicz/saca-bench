
utildir = $(includedir)/kbs_include

util_headers = \
	kbs_FibonacciString.h \
	kbs_RandomString.h \
	kbs_RandomStringFile.h 

util_modules = \
	kbs_FibonacciString.c \
	kbs_RandomString.c \
	kbs_RandomStringFile.c

noinst_LTLIBRARIES = \
	libutil.la

libutil_la_SOURCES = \
	$(util_headers) \
	$(util_modules)

libutil_la_LIBADD = \
	@top_srcdir@/src/Extern/libextern.la \
	@top_srcdir@/src/Standard/libstandard.la

libutil_la_CFLAGS = \
	-I@top_srcdir@/src/Extern \
	-I@top_srcdir@/src/Standard
