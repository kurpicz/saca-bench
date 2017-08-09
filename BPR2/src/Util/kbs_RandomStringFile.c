/* Creating of Test Data   Version 0.1 from 12.11.01 12:15 Uhr */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "kbs_RandomStringFile.h"
#include "kbs_RandomString.h"
#include "randomlib.h"
#include "kbs_Types.h"
#include "kbs_Error.h"


/*--------------------------------------------------------------------------*/
void kbs_make_RandomStringFile(Kbs_Char *testdatafile) {
	filesize = 0;
	file_ptr = fopen(testdatafile, "wb+");
	if (file_ptr == NULL) {
		printf("kbs_make_RandomSeqFile - %s failed\n", testdatafile);
		exit(EXIT_FAILURE);
	}
}

/*--------------------------------------------------------------------------*/
void kbs_open_RandomStringFile(Kbs_Char *testdatafile) {
  file_ptr = fopen(testdatafile, "rb");
	if (file_ptr == NULL) {
		printf("kbs_open_RandomSeqFile - %s failed\n", testdatafile);
		exit(EXIT_FAILURE);
	}
}

/*--------------------------------------------------------------------------*/
void kbs_close_RandomStringFile(void){
	fclose(file_ptr);
}


/*--------------------------------------------------------------------------*/
void kbs_generate_RandomUStringFile(Kbs_Uint alphabetSize, Kbs_Char *filename, Kbs_Ulong seqSize) {
	Kbs_Ulong i;
	Kbs_Uchar *seq;
	kbs_make_RandomStringFile(filename);
	seq = (Kbs_Uchar *) malloc(seqSize * sizeof(Kbs_Uchar));
	kbsRandomize();
	for (i=0; i<seqSize; i++) {
		seq[i] = kbs_genRand_UChar(alphabetSize);
	}
	fwrite(seq, sizeof(Kbs_Uchar), seqSize, file_ptr);
	kbs_close_RandomStringFile();
	printf("Random file %s generated\n", filename);
}

/*--------------------------------------------------------------------------*/
Kbs_Char *kbs_generate_RandomStringFileName(Kbs_Uint alphabetSize, Kbs_Ulong fileSize) {
	Kbs_Char *filename;
	filename = (Kbs_Char *) malloc(sizeof(Kbs_Char) * 100);
	memset(filename, '\0', 100);
	sprintf(filename,"rand_a%d_s%lu", alphabetSize, fileSize);
	return filename;
}


/*--------------------------------------------------------------------------*/
Kbs_Char *kbs_generate_PeriodicStrFileName(const Kbs_Uint alphaSize, const Kbs_Ulong periodLen, const Kbs_Ulong strLen) {
	Kbs_Char *filename;
	filename = (Kbs_Char *) malloc(sizeof(Kbs_Char) * 120);
	memset(filename, '\0', 100);
	Kbs_Int offset = sprintf(filename,"periodic_a%u", (unsigned int )alphaSize);
	offset += sprintf(filename+offset,"_p%lu", (unsigned long )periodLen);
	offset += sprintf(filename+offset,"_s%lu", (unsigned long )strLen);
	return filename;
}


/*--------------------------------------------------------------------------*/
void kbs_generate_NURandomFiles(Kbs_Uint n, Kbs_Ulong atomarSize, Kbs_Uint alphabetSize){
	Kbs_Uint i;
	Kbs_Char *filename;
	for (i=1; i<=n; i++) {
		filename = kbs_generate_RandomStringFileName(alphabetSize, i*atomarSize);
		kbs_generate_RandomUStringFile(alphabetSize, filename, i*atomarSize);
		free(filename);
	}
}



