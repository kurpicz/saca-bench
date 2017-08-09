#include <stdlib.h>
#include <stdio.h>
#include "kbs_Alphabet.h"
#include "kbs_Error.h"
#include "kbs_Limits.h"
#include "kbs_Math.h"
#include "kbs_String.h"
#include "kbs_SuffixArray.h"
#include "kbs_SuffixArrayConstDStepAndPre.h"
#include "kbs_Time.h"
#include "kbs_Types.h"






/*---------------------------------------------------------------------------------------*/
void kbs_delete_SA_WithoutString(Kbs_SuffixArray *oldSA) {
	if (oldSA == NULL) {
		return;
	}
	if (oldSA->posArray != NULL) {
		free(oldSA->posArray);
	}
	if (oldSA->invPosArray != NULL) {
		free(oldSA->invPosArray);
	}
	if (oldSA->buckets != NULL) {
		free(oldSA->buckets);
	}
  if (oldSA->lcpArray != NULL) {
    free(oldSA->lcpArray);
  }
	free(oldSA);
}

/*---------------------------------------------------------------------------------------*/
void kbs_delete_SA_IncludingString(Kbs_SuffixArray *oldSA) {
	if (oldSA == NULL) {
		return;
	}
	kbs_delete_Ustring(oldSA->str);
	kbs_delete_SA_WithoutString(oldSA);
}


/*---------------------------------------------------------------------------------------*/
void kbs_show_SA(const Kbs_SuffixArray *const sa){
	Kbs_Ulong i;
	Kbs_Char *tempCharArray;
	if (sa == NULL) {
		return;
	}
	if (sa->str->alphabet->alphaSize < ('Z'-'A')*2)
	for(i=0; i<sa->str->strLength; i++) {
		sa->str->str[i] += 'A';
	}
	for(i=0; i<sa->str->strLength; i++) {
		tempCharArray = sa->str->str + sa->posArray[i];
		printf("i:%lu - \t%lu\t%s\n", i, sa->posArray[i], tempCharArray);
	}
}


/*---------------------------------------------------------------------------------------*/
Kbs_Bool kbs_isEqual_SA(const Kbs_SuffixArray *const sa1, const Kbs_SuffixArray *const sa2) {
	Kbs_Ulong i;
	Kbs_Ulong size;
	if (sa1->str->strLength != sa2->str->strLength) {
		printf("size of SA1 %lu != size of SA2 %lu\n", sa1->str->strLength, sa2->str->strLength);
		fflush(stdout);
		return FALSE;
	}
	size = sa1->str->strLength;
	for (i=0; i<size; i++) {
		if (sa1->posArray[i] != sa2->posArray[i]) {
			printf("Mismatch at position %lu: %lu != %lu\n", i, sa1->posArray[i], sa2->posArray[i]);
			fflush(stdout);
		}
	}
	fflush(stdout);
	return TRUE;
}



/*---------------------------------------------------------------------------------------*/
void kbs_show_LongArray(Kbs_Long *longArray, Kbs_Ulong length) {
	Kbs_Ulong i;
	if(longArray == NULL) {
		return;
	}
	printf("Long Array:\n");
	for(i=0; i<length;i++) {
		printf("%d: %d\t", (int)i, (int)longArray[i]);
		printf("\n");
		fflush(stdout);
	}
}

/*---------------------------------------------------------------------------------------*/
static void kbs_show_UcharArray(Kbs_Uchar *ucharArray, Kbs_Ulong length){
	Kbs_Ulong i;
	if(ucharArray == NULL) {
		return;
	}
	for(i=0; i<length;i++) {
		printf("%lu:\t%lu\n", i, (Kbs_Ulong)ucharArray[i]);
		fflush(stdout);
	}
}
