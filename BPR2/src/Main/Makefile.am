SUBDIRS =

bprtime_headers =
bprtime_modules = \
	bprtime.main.c

fibstring_headers =
fibstring_modules = \
	fib.main.c

periodicstr_headers =
periodicstr_modules = \
	periodicStrGen.main.c


bin_PROGRAMS = bprtime fibstring periodicstr


INCLUDES =

bprtime_SOURCES = \
	$(bprtime_header) \
	$(bprtime_modules)

bprtime_CFLAGS = \
	-I@top_srcdir@/src/Extern \
	-I@top_srcdir@/src/Standard \
	-I@top_srcdir@/src/SuffixArray

bprtime_LDADD = \
	@top_srcdir@/src/Extern/libextern.la \
	@top_srcdir@/src/Standard/libstandard.la \
	@top_srcdir@/src/SuffixArray/libsuffixarray.la

bprtime_LDFLAGS = -lm


fibstring_SOURCES = \
	$(fibstring_header) \
	$(fibstring_modules)

fibstring_CFLAGS = \
	-I@top_srcdir@/src/Extern \
	-I@top_srcdir@/src/Standard \
	-I@top_srcdir@/src/Util \
	-I@top_srcdir@/src/SuffixArray

fibstring_LDADD = \
	@top_srcdir@/src/Extern/libextern.la \
	@top_srcdir@/src/Standard/libstandard.la \
	@top_srcdir@/src/Util/libutil.la \
	@top_srcdir@/src/SuffixArray/libsuffixarray.la

fibstring_LDFLAGS = -lm



periodicstr_SOURCES = \
	$(periodicstr_header) \
	$(periodicstr_modules)

periodicstr_CFLAGS = \
	-I@top_srcdir@/src/Extern \
	-I@top_srcdir@/src/Standard \
	-I@top_srcdir@/src/Util \
	-I@top_srcdir@/src/SuffixArray

periodicstr_LDADD = \
	@top_srcdir@/src/Extern/libextern.la \
	@top_srcdir@/src/Standard/libstandard.la \
	@top_srcdir@/src/Util/libutil.la \
	@top_srcdir@/src/SuffixArray/libsuffixarray.la


