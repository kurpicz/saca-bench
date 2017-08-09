#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "kbs_Types.h"
#include "kbs_Error.h"
#include "kbs_String.h"
#include "kbs_Alphabet.h"


/*--------------------------------------------------------------------------*/
void kbs_sort_Alphabet(Kbs_Alphabet *const alphabet){
	Kbs_Ulong i, k;
	if (alphabet == NULL) {
		fprintf(stdout, "Try to sort Null-alphabet\n");
		KBS_ERROR(KBS_ERROR_NULLPOINTER);
		return;
	}
	if (alphabet->charArray == NULL) {
		fprintf(stdout, "Alphabet is not complete\n");
		KBS_ERROR(KBS_ERROR_NULLPOINTER);
		return;
	}
	memset(alphabet->charArray, '\0', sizeof(Kbs_Uchar) * (alphabet->alphaSize + 1));
	k = 0;
	for (i=0; i<KBS_MAX_ALPHABET_SIZE; i++) {
		if (alphabet->alphaMapping[i] >= 0) {
			alphabet->charArray[k] = i;
			alphabet->alphaMapping[i] = k;
			k++;
		}
	}
	if (k != alphabet->alphaSize) {
		printf("k ist ungleich alphabet size");
	}
}


/*--------------------------------------------------------------------------*/
void kbs_delete_Alphabet(Kbs_Alphabet* alphabet) {
	if (alphabet == NULL) {
		return;
	}
	if (alphabet->charArray != NULL) {
		free(alphabet->charArray);
	}
	free(alphabet);
}

