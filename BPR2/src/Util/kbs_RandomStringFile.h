#ifndef KBS_RANDOMSTRINGFILE_H
#define KBS_RANDOMSTRINGFILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "kbs_Types.h"

/* Constant Definitions */

#define RAND_ALPHABET_SIZE		4
#define RAND_NUMBERFILES			10
#define RAND_ATOMAR_SIZE			10000


/* directories */
#define RANDOM_SEQ_DIR	"./RandomStrings\0"





/*--------------------------------------------------------------------------*/
/* Definition of the Testdatafilename */
#define bool        	int      /* Definition of the datatype  boolean */
#define true        	1        /* Definition of true value */
#define false       	0        /* Definition of false value */




/* Global Variables */

FILE  				*file_ptr;
unsigned long filesize;

/* Prototypes */

void kbs_make_RandomStringFile(Kbs_Char *testdatafile);
void kbs_open_RandomStringFile(Kbs_Char *testdatafile);
void kbs_close_RandomStringFile(void);
/*static void kbs_appendUChar_RandomStringFile(Kbs_Uchar letter, Kbs_Int address );*/

void kbs_generate_RandomUStringFile(Kbs_Uint alphabetSize, Kbs_Char *filename, Kbs_Ulong seqSize);

Kbs_Char *kbs_generate_RandomStringFileName(Kbs_Uint alphabetSize, Kbs_Ulong fileSize);


Kbs_Char *kbs_generate_PeriodicStrFileName(const Kbs_Uint alphaSize, const Kbs_Ulong periodLen, const Kbs_Ulong strLen);

/*--------------------------------------------------------------------------*/
void kbs_generate_NURandomFiles(Kbs_Uint n, Kbs_Ulong atomarSize, Kbs_Uint alphabetSize);

#endif
