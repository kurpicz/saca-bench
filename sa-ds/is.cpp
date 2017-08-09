#include <iostream>
#include <math.h>
#include <stdlib.h>
using namespace std;
#define DEBUGLEVEL 1
#include <time.h>
#if !defined( unix )
#include <io.h>
#include <fcntl.h>
#endif

// comment to disable verifying the result SA
#define _verify_sa

void SA_DS(unsigned char *s, int *SA, int n, int K, int m, int level);

// output values:
//  1: s1<s2
//  0: s1=s2
// -1: s1>s2
int sless(unsigned char *s1, unsigned char *s2, unsigned int n) {
	for(unsigned int i=0; i<n; i++) {
		if (s1[i] < s2[i]) return 1;
		if (s1[i] > s2[i]) return -1;
	}
	return 0;
} 

// test if SA is sorted for the input string s
bool isSorted(unsigned int *SA, unsigned char *s, unsigned int n) {
	for(unsigned int i = 0;  i < n-1;  i++) {
	  unsigned int d=SA[i]<SA[i+1]?n-SA[i+1]:n-SA[i];
		int rel=sless(s+SA[i], s+SA[i+1], d);
		if(rel==-1 || rel==0 && SA[i+1]>SA[i])
			return false;
	}
	return true;  
}

int main(int argc, char **argv) {
	fprintf( stderr, "\nComputing suffix array by SA-DS (d=3) on " );
	if ( argc > 1 ) {
		freopen( argv[ 1 ], "rb", stdin );
		fprintf( stderr, "%s", argv[ 1 ] );
	} else
		fprintf( stderr, "stdin" );
	fprintf( stderr, " to " );
	if ( argc > 2 ) {
		freopen( argv[ 2 ], "wb", stdout );
		fprintf( stderr, "%s", argv[ 2 ] );
	} else
		fprintf( stderr, "stdout" );
	fprintf( stderr, "\n" );
	#if !defined( unix )
	setmode( fileno( stdin ), O_BINARY );
	setmode( fileno( stdout ), O_BINARY );
	#endif

	// Allocate 5(n+1) bytes memory for input string and output suffix array
	fseek(stdin, 0, SEEK_END);
	int n=ftell(stdin);
  if(n<=0) {
    fprintf(stderr, "Empty file, nothing to sort, exit!");
    return 0;
  }

  n++; // count for the virtual sentinel
	fprintf(stderr, "Allocating input and output space: %d bytes = %.3lf MB", 5*n, (double)5*n/1024/1024);
	unsigned char *s_ch=new unsigned char[n];
	int *SA = new int[n];
	if(s_ch==NULL || SA==NULL) {
		delete [] s_ch; delete [] SA; 
		fprintf(stderr, "\nInsufficient memory, exit!");
		return 0;
	}

	// read the string into buffer.
	fprintf(stderr, "\nReading input string...");
	fseek(stdin, 0, SEEK_SET );
	fread((unsigned char *) s_ch, 1, n-1, stdin);
  s_ch[n-1]=0; // append the virtual sentinel

	clock_t start, finish;
	double  duration;
	start = clock();

	SA_DS(s_ch, SA, n, 255, n, 0);

	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;

	fprintf( stderr, "\nSize: %d bytes, Time: %5.4f seconds\n", n-1, duration);

#ifdef _verify_sa
	fprintf( stderr, "\nVerifying the suffix array...");
	fprintf( stderr, "\nSorted: %d", (int)isSorted((unsigned int *)SA, (unsigned char *)s_ch, (unsigned int)n));
#endif

	fprintf( stderr, "\nOutputing the suffix array...");
	for(int i=1; i<n; i++)
		fwrite( (unsigned char *)SA+i, 1, sizeof( int ), stdout );

	delete [] SA;
	delete [] s_ch;
	return 0;
}
