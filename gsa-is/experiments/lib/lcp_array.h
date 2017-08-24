#ifndef LCPH
#define LCPH

#include <string.h>
#include <stdio.h>
#include <limits.h>

#include "utils.h"
#include "file.h"


/**
 * Constructs the lcp array of a given suffix array.
 * @param T[0..n-1] The input string.
 * @param SA[0..n-1] The input suffix array.
 * @param n The length of the given string.
 * @return 0 if no error occurred, -1 otherwise.
 */
int lcp_kasai(char* T, int_t* SA, uint_t n, int_t* LCP);

/**
 * Constructs the lcp array of a given suffix array.
 * @param T[0..n-1] The input string.
 * @param SA[0..n-1] The input suffix array.
 * @param n The length of the given string.
*/
int lcp_PHI(unsigned char* T, int_t* SA, int_t* LCP, uint_t n, int cs, unsigned char separator);
int lcp_PHI_int(int_t* T, int_t* SA, int_t* LCP, uint_t n, int cs);

int lcp_array_check(unsigned char *T, int_t *SA, int_t *LCP, uint_t n, int cs, unsigned char separator);
int lcp_array_check_phi(unsigned char *T, int_t *SA, int_t *LCP, uint_t n, int cs, unsigned char separator);

int lcp_array_check_lcp(unsigned char *T, int_t *SA, int_t *LCP, uint_t n, int cs, unsigned char separator);

int lcp_array_print(unsigned char *T, int_t *SA, int_t *LCP, size_t n, int cs); 

int lcp_array_write(int_t *LCP, int_t n, char* c_file, const char* ext);
int_t lcp_array_read(int_t** LCP, char* c_file, const char* ext);

#endif
