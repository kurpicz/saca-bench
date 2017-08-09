/*
 * suftest.cpp
 */

#include "LinearSuffixSort.h"
#include <iostream>
#include <fstream>

using namespace std;

static
int
sufcheck(const unsigned char *T, const int *SA,
         int n, int verbose) {
  int C[256];
  int i = 0, p, t = 0;
  int c;
  int err = 0;

  if(1 <= verbose) { ::fprintf(stderr, "sufchecker: "); }

  /* Check arguments. */
  if((T == NULL) || (SA == NULL) || (n < 0)) { err = -1; }

  /* ranges. */
  if(err == 0) {
    for(i = 0; i <= n; ++i) {
      if((SA[i] < 0) || (n < SA[i])) {
        err = -2;
        break;
      }
    }
  }

  /* first characters. */
  if(err == 0) {
    for(i = 1; i < n; ++i) {
      if(T[SA[i]] > T[SA[i + 1]]) {
        err = -3;
        break;
      }
    }
  }

  /* suffixes. */
  if(err == 0) {
    for(i = 0; i < 256; ++i) { C[i] = 0; }
    for(i = 0; i < n; ++i) { ++C[T[i]]; }
    for(i = 0, p = 1; i < 256; ++i) {
      t = C[i];
      C[i] = p;
      p += t;
    }

    for(i = 0; i <= n; ++i) {
      p = SA[i];
      if(0 < p) {
        c = T[--p];
        t = C[c];
      } else {
        p = n;
        c = -1;
        t = 0;
      }
      if(p != SA[t]) {
        err = -4;
        break;
      }
      if(0 <= c) {
        ++C[c];
        if((n < C[c]) || (T[SA[C[c]]] != c)) { C[c] = -1; }
      }
    }
  }

  if(1 <= verbose) {
    if(err == 0) {
      ::fprintf(stderr, "Done.\n");
    } else if(verbose == 1) {
      ::fprintf(stderr, "Error.\n");
    } else if(err == -1) {
      ::fprintf(stderr, "Invalid arguments.\n");
    } else if(err == -2) {
      ::fprintf(stderr, "Out of the range [0,%d].\n  SA[%d]=%d\n",
        (int)n, (int)i, (int)SA[i]);
    } else if(err == -3) {
      ::fprintf(stderr, "Suffixes in wrong order.\n"
                      "  T[SA[%d]=%d]=%d > T[SA[%d]=%d]=%d\n",
        i, SA[i], T[SA[i]],
        i + 1, SA[i + 1], T[SA[i + 1]]);
    } else if(err == -4) {
      ::fprintf(stderr, "Suffix in wrong position.\n");
      if(0 <= t) { ::fprintf(stderr, "  SA[%d]=%d or\n", (int)t, (int)SA[t]); }
      ::fprintf(stderr, "  SA[%d]=%d\n", (int)i, (int)SA[i]);
    }
  }

  return err;
}

int
main(int argc, const char *argv[]) {
  unsigned char *T;
  int *A;
  int n;

  /* Check arguments. */
  if(argc != 2) {
    ::fprintf(stderr, "usage: %s FILE\n", argv[0]);
    ::exit(EXIT_FAILURE);
  }

  /* Open a file for reading. */
  ifstream fs(argv[1], ios_base::in | ios_base::binary);

  /* Get the file size. */
  fs.seekg(0, ios_base::end);
  n = fs.tellg();
  fs.seekg(0, ios_base::beg);

  /* Allocate n bytes of memory. */
  T = new unsigned char[n + 1];

  /* Read n bytes of data. */
  fs.read((char *)T, n);
  T[n] = 0;
  fs.close();

  /* Construct the suffix array. */
  A = LinearSuffixSort(T, n + 1);

  /* Check the suffix array. */
  if(sufcheck(T, A, n, 3) != 0) { ::exit(1); }

  /* Deallocate memory. */
  delete[] A;
  delete[] T;

  return 0;
}
