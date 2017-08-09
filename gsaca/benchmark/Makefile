ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
SACA_HEADERS := divsufsort/divsufsort.h \
		sais-lite-2.4.1/sais.h \
		dc3/dc3.h \
		ka/LinearSuffixSort.h \
		gsaca/gsaca.h
SACA_IMPL :=	divsufsort/divsufsort.c divsufsort/sssort.c divsufsort/trsort.c divsufsort/utils.c \
		sais-lite-2.4.1/sais.c \
		dc3/dc3.C \
		ka/LinearSuffixSort.cpp ka/BooleanString.cpp \
		gsaca/gsaca.c
all:	constructsa benchmark

constructsa:	constructsa.x
benchmark:	benchmark.x	corpusbenchmark.sh

constructsa.x:	programlib/constructsa.cpp $(SACA_HEADERS) $(SACA_IMPL)
	g++ -std=c++11 -DNDEBUG -Wall -Wextra -O3 -ffast-math -funroll-loops \
		-o constructsa.x programlib/constructsa.cpp $(SACA_IMPL)

benchmark.x:	programlib/benchmark.cpp $(SACA_HEADERS) $(SACA_IMPL)
ifneq ("$(wildcard /proc/sys/kernel/perf_event_paranoid)","")
	$(eval DCMR := -DMEASURECMR)
else
	$(eval DCMR := )
	@echo "Warning: It seems like your kernel has no PERF_EVENT support"
	@echo "(file /proc/sys/kernel/perf_event_paranoid is not available)"
	@echo "To measure cache miss ratios, please install PERF"
endif
	g++ -std=c++11 -DNDEBUG $(DCMR) -Wall -Wextra -O3 -ffast-math -funroll-loops \
		-o benchmark.x programlib/benchmark.cpp $(SACA_IMPL)

corpusbenchmark.sh:	programlib/corpusbenchmark.sh
	cp programlib/corpusbenchmark.sh corpusbenchmark.sh
	chmod a+x corpusbenchmark.sh
