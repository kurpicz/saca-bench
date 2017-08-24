#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "utils.h"



/*******************************************************************/
int file_chdir(char* dir);

FILE* file_open(char *c_file, const char * c_mode);
int file_close(FILE* f_in);

size_t file_size(FILE* f_in);

int file_write(FILE* f_out, uint_t array);
uint_t file_read(FILE* f_in);

char* file_load(FILE* f_in) ;
char** file_load_multiple(char* c_file, int k, int_t* n) ;

int file_text_write(unsigned char *str, int_t n, char* c_file, const char* ext);
int file_text_int_write(int_t *str_int, int_t n, char* c_file, const char* ext);

int_t file_text_read(unsigned char** str, char* c_file, const char* ext);
int_t file_text_int_read(int_t** str, char* c_file, const char* ext);

int file_bwt_write(unsigned char *str, int_t *SA, int_t n, char* c_file, const char* ext);
int file_bwt_int_write(int_t *str_int, int *SA, int_t n, char* c_file, const char* ext);

/*******************************************************************/



#endif
