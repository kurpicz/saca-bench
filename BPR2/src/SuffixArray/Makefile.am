SUBDIRS = .

suffixarrayconstdir = $(includedir)/kbs_include

suffixarrayconst_headers = \
	kbs_SuffixArray.h \
	kbs_SuffixArrayAnnotated.h \
	kbs_SuffixArrayChecker.h \
	kbs_SuffixArrayConstDStepAndPre.h



suffixarrayconst_modules = \
	kbs_SuffixArray.c \
	kbs_SuffixArrayAnnotated.c \
	kbs_SuffixArrayChecker.c \
	kbs_SuffixArrayConstDStepAndPre.c

noinst_LTLIBRARIES = libsuffixarray.la

libsuffixarray_la_SOURCES = \
	$(suffixarrayconst_headers) \
	$(suffixarrayconst_modules)


libsuffixarray_la_LIBADD = \
	@top_srcdir@/src/Standard/libstandard.la \
	@top_srcdir@/src/Util/libutil.la

libsuffixarray_la_CFLAGS = \
	-I@top_srcdir@/src/Standard \
	-I@top_srcdir@/src/Util 

libsuffixarray_la_LDFLAGS = -lm



