#include <fstream>
#include <iostream>
#include <string.h>
#include <stdexcept>

//SACAs
#include "../divsufsort/divsufsort.h"
#include "../sais-lite-2.4.1/sais.h"
#include "../dc3/dc3.h"
#include "../ka/LinearSuffixSort.h"
#include "../gsaca/gsaca.h"

//prototypes for object-oriented sacas
int koaluru( const unsigned char *S, int *SA, int n);

using namespace std;

void printUsage( const char *cmd, const char *message ) {
	cerr << cmd << " [OPTIONS] INPUT" << endl;
	cerr << "Computes the suffix array of given input." << endl;
	cerr << "OPTIONS:" << endl;
	cerr << "\t-a ALGORITHM" << endl;
	cerr << "\t  use a specific algorithm for construction:" << endl;
	cerr << "\t  DSS\tdivsufsort" << endl;
	cerr << "\t  SAIS\tsuffix array induced sort" << endl;
	cerr << "\t  DC3\tskew algorithm" << endl;
	cerr << "\t  KA\tKo Aluru algorithm" << endl;
	cerr << "\t  GSACA\tGrouping SACA" << endl;
	cerr << "\t  Default: DSS" << endl;
	cerr << "\t-x" << endl;
	cerr << "\t  output suffix array in computer-readable format" << endl;
	cerr << "\t  (only suffix start positions seperated by whitespace)" << endl;
	cerr << "\t-f" << endl;
	cerr << "\t  treat INPUT as file path" << endl;
	cerr << "INPUT: either a string, or path to a file if -f option is enabled" << endl;
	if (message)	cerr << endl << message << endl;
}

int main( int argc, char **argv ) {
	//read options
	int (*saca)(const unsigned char *, int *, int) = 0;
	bool xoutput = false;
	bool finput = false;
	for (int i = 1; i < argc-1; i++) {
		if (strcmp(argv[i], "-a") == 0) {
			++i; //get second command
			if (i >= argc-1) {
				printUsage(argv[0], 0);
				return 1;
			} else if (saca != 0) {
				printUsage(argv[0], "only 1 algorithm for construction can be used");
				return 1;
			} else {
				//choose algorithm
				if      (strcmp(argv[i],"DSS") == 0)	saca = divsufsort;
				else if (strcmp(argv[i],"SAIS") == 0)	saca = sais;
				else if (strcmp(argv[i],"DC3") == 0)	saca = dc3;
				else if (strcmp(argv[i],"KA") == 0)	saca = koaluru;
				else if (strcmp(argv[i],"GSACA") == 0)	saca = gsaca;
				else {
					printUsage(argv[0], "unknown algorithm");
					return 1;
				}
			}
		} else if (strcmp(argv[i], "-x") == 0) {
			xoutput = true;
		} else if (strcmp(argv[i], "-f") == 0) {
			finput = true;
		} else {
			printUsage(argv[0], 0);
			return 1;
		}
	}
	if (saca == 0)	saca = divsufsort;

	//read input
	unsigned char *S;
	int n;
	if (argc < 2) {
		printUsage(argv[0], "no input given");
		return 1;
	}
	if (!finput) { //use input directly as text
		S = (unsigned char *)argv[argc-1];
		n = strlen(argv[argc-1]) + 1;
	} else { //read input from file
		ifstream infile;
		infile.open(argv[argc-1], ifstream::binary);
		if (!infile.good()) {
			printUsage(argv[0], "Problem opening file");
			return 1;
		}
		infile.seekg(0, infile.end);
		n = (int)infile.tellg();
		if (n <= 0) {
			printUsage(argv[0], "Input file is empty");
			return 1;
		}

		//read entire file to an array
		S = new unsigned char[n+1]; //cleaned automatically at end of program
		if (!S) {
			printUsage(argv[0], "Out of Memory");
			return 1;
		}
		infile.seekg(0, infile.beg);
		infile.read((char *)S, n);
		if (!infile.good()) {
			printUsage(argv[0], "Problem reading file");
			return 1;
		}
		infile.close();

		if (S[n-1] > 31) { //care for sentinel character
			S[n++] = 0;
		} else {
			S[n-1] = 0;
		}
	}
	for (int i = 0; i < n-1; i++) { //check text format
		if (S[i] == 0) {
			printUsage(argv[0], "termination byte is allowed only at end of input");
			return 1;
		}
	}

	//construct suffix array from T
	int *SA = new int[n];
	if (!SA) {
		printUsage(argv[0], "Out of Memory");
		return 1;
	}
	if (saca(S, SA, n) != 0) {
		printUsage(argv[0], "Out of Memory");
		return 1;
	}

	//output suffix array
	if (xoutput) { //machine - readable output
		cout << SA[0];
		for (int i = 1; i < n; i++) {
			cout << ' ' << SA[i];
		}
	} else { 	//human - readable output
		cout << "i\tSA[i]\tINPUT[SA[i]..n)" << endl;
		for (int i = 0; i < n; i++) {
			if (SA[i] >= n || SA[i] < 0) {
				cerr << "Invalid suffix array construction" << endl;
				return 1;
			}
			cout << i << '\t' << SA[i] << '\t' << (char *)(S+SA[i]) << endl;
		}
	}
	return 0;
}

int koaluru( const unsigned char *S, int *SA, int n) {
	int *SA_tmp = LinearSuffixSort( S, n );
	if (!SA_tmp)	return -1;
	memcpy(SA, SA_tmp, n*sizeof(int));
	delete[] SA_tmp;
	return 0;
}
