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
#include "randomlib.h"
#include "kbs_Types.h"
#include "kbs_Error.h"
#include "kbs_String.h"





/*--------------------------------------------------------------------------*/
Kbs_Uchar kbs_genRand_UChar(Kbs_Uint alphabetSize) {
	Kbs_Uchar letter;
	Kbs_Int lower, upper;
	if (alphabetSize>250) {
		printf("alphabetSize %du exceed maximum\n", alphabetSize);
	}
	lower = 1;
	upper = lower + alphabetSize - 1;
	letter = RandomInt(lower,upper);
	if (alphabetSize < 60) {
		letter += 'A'-1;
	}
	return letter;
}

/*--------------------------------------------------------------------------*/
Kbs_Ustring *kbs_genRand_UString(Kbs_Uint alphabetSize, Kbs_Ulong seqSize) {
	if (seqSize == 0) {
		return NULL;
	}
	Kbs_Ustring *string;
	string = (Kbs_Ustring *) malloc(sizeof(Kbs_Ustring));
	if (string == NULL) {
		KBS_ERROR(KBS_ERROR_MALLOC);
	}
	string->strLength = seqSize;
	string->str = (Kbs_Uchar *) malloc(seqSize+1 * sizeof(Kbs_Uchar));
	if (string->str == NULL) {
		KBS_ERROR(KBS_ERROR_MALLOC);
	}
	kbsRandomize();
	Kbs_Ulong i;
	for (i=0; i<seqSize; i++) {
		string->str[i] = kbs_genRand_UChar(alphabetSize);
	}
	string->str[seqSize] = 0;
	return string;
}

/*--------------------------------------------------------------------------*/
Kbs_Ustring *kbs_genRandPeriodic_UString(Kbs_Uint alphabetSize, Kbs_Ulong lengthPeriod, Kbs_Ulong strLength) {
	if (strLength == 0) {
		return NULL;
	}
	Kbs_Ustring *string;
	string = (Kbs_Ustring *) malloc(sizeof(Kbs_Ustring));
	if (string == NULL) {
		KBS_ERROR(KBS_ERROR_MALLOC);
	}
	string->strLength = strLength;
	string->str = (Kbs_Uchar *) malloc((strLength + 1) * sizeof(Kbs_Uchar));
	if (string->str == NULL) {
		KBS_ERROR(KBS_ERROR_MALLOC);
	}
	kbsRandomize();
	if (lengthPeriod > strLength || lengthPeriod == 0) {
		return kbs_genRand_UString(alphabetSize, strLength);
	}
	Kbs_Ulong i;
	for (i=0; i<lengthPeriod; i++) {
		string->str[i] = kbs_genRand_UChar(alphabetSize);
	}
	for (i=lengthPeriod; i<strLength; i++) {
		string->str[i] = string->str[i - lengthPeriod];
	}
	string->str[strLength] = 0;
	return string;
}




