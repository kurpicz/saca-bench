#!/bin/bash
#check args
if [ $# -ne 1 ]; then
	echo "USAGE: $0 CORPUS"
	echo "CORPUS: name of corpus to be benchmarked (data/CORPUS must be a directory)"
	exit 1
fi
#check if corpus exists
cdir="data/$1"
if [ ! -d $cdir ]; then
	echo "Corpus $1 does not exist (data/$1 is not a directory)"
	exit 1
fi
#create data of corpus
if [ -e "$cdir/Makefile" ]; then
	curdir=$(pwd)
	cd $cdir
	make
	cd $curdir
fi
#create benchmark if not done yet
make benchmark.x
#run benchmark (10 repeats) and output results to file
./benchmark.x -r 10 $(find $cdir -type f ! -name Makefile) | tee "results/$1.dat"
