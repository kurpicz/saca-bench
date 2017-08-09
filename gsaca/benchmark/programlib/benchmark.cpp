#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#include <libgen.h>

//SACAs
#include "../divsufsort/divsufsort.h"
#include "../sais-lite-2.4.1/sais.h"
#include "../dc3/dc3.h"
#include "../ka/LinearSuffixSort.h"
#include "../gsaca/gsaca.h"

//Utility for cache miss rates (using PERF API)
#ifdef MEASURECMR
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>

bool init_cmr_measures();
bool start_cmr_measure();
double end_cmr_measure(); //returns cache miss ratio or -INF on problems
#endif

//abstract SACA interface for benchmark
struct SACA {
		virtual const char *name() = 0;
		virtual int prepare(int n) = 0;
		virtual int run(const unsigned char *S, int n) = 0;
		virtual void cleanup() = 0;
};

struct CSACA : public SACA { //classic c - styled sacas
	int *SA;
	int prepare(int n) {
		SA = new int[n];
		return SA == 0;
	};
	void cleanup() {
		delete[] SA;
	};
};

struct DivSufSort : public CSACA {
	const char *name() { return "dss"; };
	int run(const unsigned char *S, int n) {
		return divsufsort(S, SA, n);
	};
} saca_dss;

struct Sais : public CSACA {
	const char *name() { return "sais"; };
	int run(const unsigned char *S, int n) {
		return sais(S, SA, n);
	};
} saca_sais;

struct DistanceCover : public CSACA {
	const char *name() { return "dc3"; };
	int run(const unsigned char *S, int n) {
		return dc3(S, SA, n);
	};
} saca_dc3;

struct KoAluru : public SACA {
	int *SA;
	const char *name() { return "ka"; };
	int prepare(int n) {return 0;};
	int run(const unsigned char *S, int n) {
		SA = LinearSuffixSort( S, n ); //own allocation...
		return SA == 0;
	};
	void cleanup() {
		delete[] SA;
	};
} saca_ka;

struct GSaca : public CSACA {
	const char *name() { return "gsaca"; };
	int run(const unsigned char *S, int n) {
		return gsaca(S, SA, n);
	};
} saca_gsaca;

using namespace std;

void printUsage( const char *cmd, const char *message ) {
	cerr << cmd << " [OPTIONS] FILELIST" << endl;
	cerr << "Runs a SACA benchmark on a given list of files." << endl;
	cerr << "Sizes are in bytes, times in milliseconds, cache misses in ratio." << endl;
	cerr << "OPTIONS:" << endl;
	cerr << "\t-r RUNSPERFILE" << endl;
	cerr << "\t  run each algorithm RUNSPERFILE times and build average results" << endl;
	cerr << "FILELIST: list of files separated by whitespace" << endl;
	if (message)	cerr << endl << message << endl;
}

int main( int argc, char **argv ) {
	//all sacas with same interface
	SACA *saca[] = { &saca_dss, &saca_sais, &saca_dc3, &saca_ka, &saca_gsaca };
	size_t numsacas = sizeof(saca) / sizeof(saca[0]);

	//start processing arguments
	size_t numfiles = 0, runsperfile = 0;
	char **filename = argv + 1;
	if (argc < 2) {
		printUsage(argv[0], "need at least 1 input file");
		return 1;
	}
	//check runs per file
	if (strcmp(argv[1], "-r") == 0) {
		if (argc < 3) { //get second argument
			printUsage(argv[0], 0);
			return 1;
		}
		int r = atoi(argv[2]); //read argument
		if (r <= 0) {
			printUsage(argv[0], "runs per file must be greater 0");
			return 1;
		}
		runsperfile = (size_t)r;
		filename = filename + 2;
		argc -= 2;
	}
	if (runsperfile == 0)	runsperfile = 1; //default if necessary

	//check file list
	numfiles = (size_t)(argc - 1);
	if (numfiles == 0) {
		printUsage(argv[0], "need at least 1 input file");
		return 1;
	}

	//prepare cache miss measurements
#ifdef MEASURECMR
	if (!init_cmr_measures()) {
		printUsage(argv[0], "unable to initialise cache miss measurements using PERF");
		return 1;
	}
#endif

	//start benchmark
	//output header
	cout << "filename \tfilesize \tsigma";
	for (size_t i = 0; i < numsacas; i++) {
		cout << " \t" << saca[i]->name() << "time";
#ifdef MEASURECMR
		cout << " \t" << saca[i]->name() << "cmr";
#endif
	}
	cout << endl;
	//process file list
	for (size_t i = 0; i < numfiles; i++) {
		//read input from file
		unsigned char *S;
		int n;
		{
			ifstream infile;
			infile.open(filename[i], ifstream::binary);
			if (!infile.good()) {
				cerr << "Problem opening file " << filename[i] << endl;
				continue;
			}
			infile.seekg(0, infile.end);
			n = (int)infile.tellg();
			if (n <= 0) {
				cerr << "File " << filename[i] << " is empty" << endl;
				infile.close(); continue;
			}

			//read entire file to an array
			S = new unsigned char[n+1]; //cleaned automatically at end of program
			if (!S) {
				cerr << "File " << filename[i] << ": out of memory" << endl;
				infile.close(); continue;
			}
			infile.seekg(0, infile.beg);
			infile.read((char *)S, n);
			if (!infile.good()) {
				cerr << "Problem reading file " << filename[i] << endl;
				infile.close(); delete[] S; continue;
			}
			infile.close();
		}
		if (S[n-1] > 31) { //care for sentinel character
			S[n++] = 0;
		} else {
			S[n-1] = 0;
		}

		//check text format and get sigma
		size_t sigma = 1;
		{
			vector<bool> cdupl( std::numeric_limits<unsigned char>::max() + 1, false );
			for (int j = 0; j < n-1; j++) {
				if (S[j] == 0) {
					cerr << "File " << filename[i] << ":" << endl; 
					cerr << "termination byte is allowed only at end of input" << endl;
					delete[] S; S = 0; break;
				} else if (!cdupl[S[j]]) {
					cdupl[S[j]] = true;
					++sigma;
				}
			}
		}
		if (S != 0) {
			//output file stats
			string fname( basename(filename[i]) );
			replace( fname.begin(), fname.end(), '_', '~' ); //some latex escaping
			cout << fname << " \t" << n << " \t" << sigma;

			//and finally run benchmark
			for (size_t j = 0; j < numsacas; j++) {
				uint64_t time_ms = 0;
#ifdef MEASURECMR
				double cmr = 0;
#endif
				for (size_t k = 0; k < runsperfile; k++) {
					if (saca[j]->prepare(n) != 0) {
						cerr << "File " << filename[i] << ": Unable to prepare SACA " 
						     << saca[j]->name() << endl;
						break;
					}

#ifdef MEASURECMR
					if (!start_cmr_measure()) {
						cerr << "File " << filename[i] << ": Unable to start cache miss measure on SACA"
						     << saca[j]->name() << endl;
						break;
					}
#endif				
					auto start = chrono::high_resolution_clock::now();
					int r = saca[j]->run(S, n);
					auto stop = chrono::high_resolution_clock::now();
#ifdef MEASURECMR
					double ratio = end_cmr_measure();
					if (ratio < 0) {
						cerr << "File " << filename[i] << ": Problem measuring cache miss ratio on SACA " 
						     << saca[j]->name() << endl;
						break;
					}
					cmr += ratio;
#endif
					saca[j]->cleanup();
					if (r != 0) {
						cerr << "File " << filename[i] << ": Problem running SACA " 
						     << saca[j]->name() << endl;
						break;
					}
					time_ms += (uint64_t)chrono::duration_cast<chrono::milliseconds>
						                                  (stop-start).count();
				}
				cout << " \t" << (time_ms / runsperfile);
#ifdef MEASURECMR
				cout << " \t" << (cmr / (double)runsperfile);
#endif
			}
			delete[] S;
			cout << endl;
		}
	}
	return 0;
}

//// CACHE MISS RATIO UTILITY /////////////////////////////////////////////////

#ifdef MEASURECMR
static long
perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                int cpu, int group_fd, unsigned long flags)
{
    int ret;

    ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                   group_fd, flags);
    return ret;
}

int fd_cachereferences;
int fd_cachemisses;

bool init_cmr_measures() {
	struct perf_event_attr pe;
	//event for cache references
	memset(&pe, 0, sizeof(struct perf_event_attr));
	pe.type = PERF_TYPE_HARDWARE;
	pe.size = sizeof(struct perf_event_attr);
	pe.config = PERF_COUNT_HW_CACHE_REFERENCES;
	pe.disabled = 1;
	pe.exclude_kernel = 1;
	pe.exclude_hv = 1;
	fd_cachereferences = perf_event_open(&pe, 0, -1, -1, 0); //open as group leader
	if (fd_cachereferences == -1) {
		return false;
	}
	//event for cache misses (child event of cache references)
	memset(&pe, 0, sizeof(struct perf_event_attr));
	pe.type = PERF_TYPE_HARDWARE;
	pe.size = sizeof(struct perf_event_attr);
	pe.config = PERF_COUNT_HW_CACHE_MISSES;
	pe.disabled = 0; //because it is used as child
	pe.exclude_kernel = 1;
	pe.exclude_hv = 1;
	fd_cachemisses = perf_event_open(&pe, 0, -1, fd_cachereferences, 0);
	return fd_cachemisses != -1;
}

bool start_cmr_measure() {
	//reset counters
	if (ioctl(fd_cachereferences, PERF_EVENT_IOC_RESET, 0) == -1 ||
	    ioctl(fd_cachemisses, PERF_EVENT_IOC_RESET, 0) == -1) {
		return false;
	}
	//start all events by starting group leader
	return ioctl(fd_cachereferences, PERF_EVENT_IOC_ENABLE, 0) != -1;
}

double end_cmr_measure() {
	uint64_t rel_crefs, rel_cmisses; //both values need not to be absolute,
	//but are measured together, so an approximate rate can be computed

	//stop all events by stoping group leader
	if (ioctl(fd_cachereferences, PERF_EVENT_IOC_DISABLE, 0) == -1) {
		return -std::numeric_limits<double>::infinity();
	}
	//read both values from file descriptors
	if (read(fd_cachereferences, &rel_crefs, sizeof(uint64_t)) == -1 ||
	    read(fd_cachemisses, &rel_cmisses, sizeof(uint64_t)) == -1) {
		return -std::numeric_limits<double>::infinity();
	}
	return (double)rel_cmisses / (double)rel_crefs;
}
#endif
