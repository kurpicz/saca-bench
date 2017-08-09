
#ifndef KBS_SUFFIXARRAYCHECKER_H
#define KBS_SUFFIXARRAYCHECKER_H

#include <stdlib.h>
#include "kbs_Error.h"
#include "kbs_SuffixArray.h"
#include "kbs_SuffixArrayChecker.h"
#include "kbs_Types.h"




/**
 * straight forward checking of condition 1 (all suffix numbers are in range up to string length)
 * @param sa suffix array to be checked for correctness
 * @return suffix array is correct
 */
static Kbs_Bool check_Condition1(const Kbs_SuffixArray *const sa) {
	Kbs_Ulong i;
	for(i=0; i < sa->str->strLength; i++) {
		if (sa->posArray[i] >= sa->str->strLength) {
			printf("Condition 1 not satisfied: posArray[%d]= %d >= %d = string length\n", (int)(i), (int)sa->posArray[i], (int)sa->str->strLength);
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * straight forward checking of condition 2 ()
 * @param sa suffix array to be checked for correctness
 * @return suffix array is correct
 */
static Kbs_Bool check_Condition2(const Kbs_SuffixArray *const sa) {
	Kbs_Ulong i;
	Kbs_Ulong *pos = sa->posArray;
	Kbs_Uchar *s = sa->str->str;
	for(i=1; i < sa->str->strLength; i++) {
		if (s[pos[i-1]] > s[pos[i]]) {
			printf("Condition 2 not satisfied: s[pos[%d]]=%d > s[pos[%d]]=%d\n", (int)(i-1), (int)s[pos[i-1]], (int)i, (int)s[pos[i]]);
			printf("Condition 2 not satisfied: %d%d%d > %d%d%d \n", (int)s[pos[i-1]], (int)s[pos[i-1]+1], (int)s[pos[i-1]+2], (int)s[pos[i]],(int)s[pos[i]+1],(int)s[pos[i]+2]);
			printf("Condition 2 not satisfied: pos[%d]=%d > pos[%d]]=%d\n", (int)(i-1), (int)pos[i-1], (int)i, (int)pos[i]);
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * straight forward checking of condition 3
 * @param sa suffix array to be checked for correctness
 * @return suffix array is correct
 */
static Kbs_Bool check_Condition3(const Kbs_SuffixArray *const sa) {
	Kbs_Ulong i;
	Kbs_Ulong *pos = sa->posArray;
	Kbs_Long* invPos = (Kbs_Long*) malloc(sizeof(Kbs_Long) * (sa->str->strLength + 1));
	Kbs_Uchar *s = sa->str->str;
	for(i=0; i < sa->str->strLength; i++) {
		invPos[pos[i]] = i;
	}
	invPos[sa->str->strLength] = -1;
	Kbs_Long j, k;
	for(i=1; i < sa->str->strLength; i++) {
		if (s[pos[i-1]] == s[pos[i]]) { // && pos[i-1] != sa->str->strLength-1
			j = (Kbs_Long)invPos[pos[i-1] + 1];
			k = (Kbs_Long)invPos[pos[i] + 1];
			if (j>=k) {
				printf("Condition 3 not satisfied: invPos[pos[%d]+1]=%d >= invPos[pos[%d]+1]=%d\n", (int)(i-1), (int)j, (int)i, (int)k);
				free(invPos);
				return FALSE;
			}
		}
	}
	free(invPos);
	return TRUE;
}


/*---------------------------------------------------------------------*/
Kbs_Bool kbs_check_SuffixArray(const Kbs_SuffixArray *const sa) {
	if (sa == NULL || sa->str == NULL || sa->str->str == NULL || sa->posArray == NULL ) {
		KBS_ERROR(KBS_ERROR_NULLPOINTER);
		return FALSE;
	}
	return (check_Condition1(sa) && check_Condition2(sa) && check_Condition3(sa));
}


#endif
