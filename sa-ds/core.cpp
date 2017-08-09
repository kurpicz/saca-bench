// This is the sample code for the SA-DS algorithm presented in our article:
// G. Nong, S. Zhang and W. H. Chan, Two Efficient Algorithms for Linear Time Suffix Array Construction, 
// IEEE Transactions on Computers, Vol. 60, No. 10, Oct. 2011. 
// which draft can be retrieved at: http://code.google.com/p/ge-nong/

#include <iostream>
#include <stdlib.h>
using namespace std;
#include <memory.h>
#include <time.h>

unsigned char mask[]={0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
#define tget(i) ( (t[(i)/8]&mask[(i)%8]) ? 1 : 0 )
#define tset(i, b) t[(i)/8]=(b) ? (mask[(i)%8]|t[(i)/8]) : ((~mask[(i)%8])&t[(i)/8])

#define chr(i) (cs==sizeof(int)?((int*)s)[i]:((unsigned char *)s)[i])
#define weight1(x) ((int)(chr(x)*2+tget(x)))
#define weight(x) ((int)(chr(x)))

clock_t start, finish;
double  duration;

void timer_start() {
	start = clock();
}

void timer_finish(char *str) {
	finish = clock(); 
	duration = (double)(finish - start) / CLOCKS_PER_SEC; 
	fprintf( stderr, "\n%s: %.2lf seconds", str, duration);
}

// sort a[0..n-1] to b[0..n-1] according to the LS-types of characters from s
static void radixPassLS(int *a, int *b, unsigned char *s, unsigned char *t, int n1, int K, 
					  int n, int cs, int d) 
{ // count occurrences
  int i, j;
  int c[]={0, n1-1}; // counter array
  for (i = 0;  i < n1;  i++) {
	  j=a[i]+d; 
    if(j>n-1) j=n-1; 
	  if(tget(j)) b[c[1]--] = a[i]; // type-S
	  else b[c[0]++] = a[i]; // type-L
  }
}

// stably sort a[0..n-1] to b[0..n-1] with keys in 0..K from s
static void radixPass(int *a, int *b, unsigned char *s, unsigned char *t, int n1, int K, 
					  int n, int cs, int d) 
{ // count occurrences
  int i, j, sum, ch;
  int K2=K+2;
  int* c = new int[K2]; // counter array
  for (i = 0;  i < K2;  i++) c[i] = 0; // reset counters
  for (i = 0;  i < n1;  i++) {
	  j=a[i]+d; 
    ch=(j>=n-1) ? 0 : chr(j)+1;
 	  c[ch]++; // count occurences
  }
  for (i = 0, sum = 0;  i < K2;  i++) { // exclusive prefix sums
     int t = c[i];  c[i] = sum;  sum += t;
  }
  for (i = 0;  i < n1;  i++) {
	  j=a[i]+d; 
    ch=(j>=n-1) ? 0 : chr(j)+1;
    b[c[ch]++] = a[i]; // sort non-sentinel characters
  }

  delete [] c;
}

// stably sort a[0..n-1] to b[0..n-1] with keys in 0..K^2 from 
//   s with a virtual sentinel 0
static void radixPass2(int *a, int *b, unsigned char *s, unsigned char *t, int n1, int K, 
					  int n, int cs, int d) 
{ // count occurrences
  int i, j, j1, sum;
  int K2=K+2, K22=K2*K2; // count for the virtual sentinel
  int row, col;
  int* c = new int[K22]; // counter array
  for (i = 0;  i < K22;  i++) c[i] = 0; // reset counters

  for (i = 0;  i < n1;  i++) {
	  j=a[i]+d; j1=j+1;
    row=(j>=n-1) ? 0 : chr(j)+1;
    col=(j1>=n-1) ? 0 : chr(j1)+1;
 	  c[row*K2+col]++; // count occurences
  }
  for (i = 0, sum = 0;  i < K22;  i++) { // exclusive prefix sums
     int t = c[i];  c[i] = sum;  sum += t;
  }
  for (i = 0;  i < n1;  i++) {
	  j=a[i]+d; j1=j+1;
    row=(j>=n-1) ? 0 : chr(j)+1;
    col=(j1>=n-1) ? 0 : chr(j1)+1;
	  b[c[row*K2+col]++] = a[i]; // sort
  }
  delete [] c;
}

// store to p1 the pointers for all the critical characters in s[0..n-1]
int criticalChars(unsigned char *s, unsigned char *t, int *p1, int n, int cs) {
  int i, j;
  int d=3; // don't change!
  for(i=1, j=0; i<=n; i+=d, j++) {
	  //s[i-2] is type-L and s[i-1] is type-S
	  if(i>1 && tget(i-2)==0 && tget(i-1)==1) i--;
	  else
	    //s[i] is type-L and s[i+1] is type-S
		  if(tget(i)==0 && tget(i+1)==1) i++;
	  if(p1!=NULL) p1[j]=i;
  }
  return j;
}

// compute the start and end of each bucket, and init SA
void getBuckets(unsigned char *s, int *bkt, int n, int K, int cs, bool end) { 
  int i, sum=0;
  for(i=0; i<=K; i++) bkt[i]=0; // clear all buckets
  for(i=0; i<n; i++) bkt[chr(i)]++; // compute the size of each bucket
  for(i=0; i<=K; i++) { sum+=bkt[i]; bkt[i]=end ? sum : sum-bkt[i]; }
}

// build the suffix array SA of s[0..n-1] in {0..K}^n, n>=2;
// s[n-1]=0 is the VIRTUAL sentinel;
// use a space of at most 6.25n+(1) for a constant alphabet;
// d=3, each d-critical substring is of d+2=5 characters;
// SA[0, m-1], m>=n, is free for computing the suffix array of s
void SA_DS(unsigned char *s, int *SA, int n, int K, int m, int level) {
  static double redu_ratio=0;
  static long sum_n=0, sum_n1=0;
  int cs=(level==0) ? sizeof(char) : sizeof(int);

  fprintf(stderr, "\nLevel: %d", level);

  int i, j;
  unsigned char *t=new unsigned char[n/8+1]; // LS-type array in bits

  // stage 1: reduce the problem by at least 1/2

  timer_start();
  // Classify the type of each character
  tset(n-2, 0); tset(n-1, 1); //  the sentinel must be in s1, important!!!
  for(i=n-3; i>=0; i--) tset(i, (chr(i)<chr(i+1) || (chr(i)==chr(i+1) && tget(i+1)==1))?1:0);

  // 2*n1 must be not larger than n (proveable)
  int n1=criticalChars(s, t, SA, n, cs);
  int *SA1=SA;
  int *s1=SA+m-n1;

  // lsb radix sort critical 5-tuplets
  int *a=SA1, *b=SA1+n1;
  radixPassLS(a, b, s, t, n1, K, n, cs, 4);  	
  radixPass(b, a, s, t, n1, K, n, cs, 4);
  if(level==0) {
	    // speed-up bucket sorting on level 0, two characters per item
	    radixPass2(a, b, s, t, n1, K, n, cs, 2);
      radixPass2(b, a, s, t, n1, K, n, cs, 0); 
  }
  else {
	  // normal bucket sort, one character per item
      radixPass(a, b, s, t, n1, K, n, cs, 3);
      radixPass(b, a, s, t, n1, K, n, cs, 2);
      radixPass(a, b, s, t, n1, K, n, cs, 1);
      radixPass(b, a, s, t, n1, K, n, cs, 0); 	
  }
  timer_finish("Time for sorting all the d-critical substrings");

  // SA[0, n1-1] stores the sorted d-critical substrings of s now

  timer_start();

  // name the sorted substrings
  for(i=n1; i<=n1+(n-1)/2; i++) SA[i]=-1;
  // find lexicographic names of 5-tuplets
  int name = 0, c[] = {-1, -1, -1, -1, -1};
  for(i=0; i<n1; i++) {
	  int d, pos=SA[i]; bool diff=false;
    for(d=0; d<4; d++)
      if(weight(pos+d)!=c[d]) {diff=true; break;}
	  if(weight1(pos+4)!=c[4]) diff=true; 
    if(pos+4>=n-1 || diff) { 
      name++;
      for(d=0; d<4; d++)
        c[d]=(pos+d<n)?weight(pos+d):-1;
		  c[d]=(pos+d<n)?weight1(pos+d):-1;
    }
    SA[n1+pos/2]=name-1; 
  }
  for(i=n1+(n-1)/2, j=m-1; i>=n1; i--)
	  if(SA[i]>=0) SA[j--]=SA[i];

  timer_finish("Time for naming");

  // s1 is now stored in SA[m-n1, m-1]

  // stage 2: solve the reduced problem

  fprintf(stderr, "\nReduction ratio: %.2lf", (double)n1/n);
  redu_ratio += (double)n1/n;
  sum_n1+=n1; sum_n+=n;
  // recurse if names are not yet unique
  if(name<n1) {
    SA_DS((unsigned char*)s1, SA1, n1, name-1, m-n1, level+1);
  } else { // generate the suffix array of s1 directly
    for(i=0; i<n1; i++) SA1[s1[i]] = i;
	cerr << endl << "Recusion ends";
	fprintf(stderr, "\nMean reduction ratio over iterations: %.2lf", redu_ratio/(level+1));
	fprintf(stderr, "\nMean reduction ratio over characters: %.2lf", (double)sum_n1/sum_n);
  }

  fprintf(stderr, "\nLevel: %d", level);

  // stage 3: induce the result for the original problem

  timer_start();
  // compute suffix array for critical chars in s
  criticalChars(s, t, s1, n, cs); // get p1

  int *bkt = new int[K+1]; // bucket counters

 // put all left-most S characters into their buckets
  getBuckets(s, bkt, n, K, cs, true); // find ends of buckets
  for(i=0; i<n1; i++) SA1[i]=s1[SA1[i]]; // get index in s1
  for(i=n1; i<n; i++) SA[i]=-1; // init SA[n1..n-1]
  for(i=n1-1; i>0; i--) {
	  j=SA[i]; SA[i]=-1;
	  if(j>0 && tget(j) && !tget(j-1)) SA[--bkt[chr(j)]]=j;
  }

  // compute SAl
  getBuckets(s, bkt, n, K, cs, false); // find starts of buckets
  if(level==0)
    bkt[0]++; // handle the virtual sentinel
  for(i=0; i<n; i++) {
	  j=SA[i]-1; 
	  if(j>=0 && !tget(j)) SA[bkt[chr(j)]++]=j;
  }

  // compute SAs
  getBuckets(s, bkt, n, K, cs, true); // find ends of buckets
  for(i=n-1; i>0; i--) {
	  j=SA[i]-1; 
	  if(j>=0 && tget(j)) SA[--bkt[chr(j)]]=j;
  }

  timer_finish("Time for sorting all the suffixes");

  delete [] bkt; 
  delete [] t;
}
