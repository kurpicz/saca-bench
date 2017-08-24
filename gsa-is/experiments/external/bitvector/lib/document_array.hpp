#ifndef DAH
#define DAH

#include <cstring>
#include <cstdio>
#include <climits>

#include "utils.hpp"


int document_array_LF(unsigned char* T, int_t* SA, int_t* DA, uint_t n, unsigned int SIGMA, int cs, unsigned char separator, int_t k);
int document_array_LF_int(int_t* T, int_t* SA, int_t* DA, uint_t n, unsigned int SIGMA, int cs, unsigned char separator, int_t k);

/**/

int document_array_check(unsigned char *T, int_t *SA, int_t *DA, uint_t n, int cs, unsigned char separator, int_t k);

int document_array_check_int(int_t *T, int_t *SA, int_t *DA, uint_t n, int cs, int_t k);

int document_array_print(unsigned char *T, int_t *SA, int_t *DA, size_t n, int cs);
int document_array_write(int_t *DA, int_t n, char* c_file, const char* ext);
int_t document_array_read(int_t** DA, char* c_file, const char* ext);

/**/

#endif
