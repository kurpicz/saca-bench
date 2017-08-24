#ifndef SUFFIXARRAYH
#define SUFFIXARRAYH

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "utils.h"
#include "file.h"

int suffix_array_write(int_t *SA, int_t n, char* c_file, const char* ext);
int_t suffix_array_read(int_t** SA, char* c_file, const char* ext);

int_t suffix_array_print(unsigned char *T, int_t *SA, size_t len, int cs);
int_t suffix_array_check(unsigned char *T, int_t *SA, size_t len, int cs, unsigned char sentinel);

#endif
