#include <stdlib.h>
#include "kbs_Limits.h"
#include "kbs_Math.h"
#include "kbs_String.h"
#include "kbs_SuffixArray.h"
#include "kbs_SuffixArrayAnnotated.h"
#include "kbs_Types.h"




/*---------------------------------------------------------------------*/
Kbs_Ulong *kbs_lcpArrayFromSuffixArray(Kbs_SuffixArray *const suffixarray) {
	if (suffixarray == NULL || suffixarray->str == NULL || suffixarray->str->str == NULL || suffixarray->posArray == NULL) {
		KBS_ERROR(KBS_ERROR_NULLPOINTER);
		return NULL;
	}
	Kbs_Ulong strLen = suffixarray->str->strLength;
	Kbs_Uchar *charArray = suffixarray->str->str;
	Kbs_Ulong *sa = suffixarray->posArray;
	Kbs_Ulong **invPosArray = suffixarray->invPosArray;
	Kbs_Ulong i;
	if (invPosArray == NULL) {
		invPosArray = (Kbs_Ulong *)malloc(sizeof(Kbs_Ulong)*strLen);
		if (invPosArray == NULL) {
			KBS_ERROR(KBS_ERROR_MALLOC);
		}
		for (i=0; i<strLen; i++) {
      invPosArray[sa[i]] = sa+i;
		}
	}

  Kbs_Ulong *lcparray = (Kbs_Ulong *)malloc(sizeof(Kbs_Ulong) * (strLen+1));
  if (lcparray == NULL) {
    KBS_ERROR(KBS_ERROR_MALLOC);
  }
	Kbs_Ulong h = 0;
	for (i=0; i<strLen; i++) {
		if (invPosArray[i] != sa) {
			register Kbs_Ulong j = *(invPosArray[i]-1);
			while (i+h < strLen && j+h < strLen && charArray[i+h] == charArray[j+h]) {
				h++;
			}
			lcparray[invPosArray[i] - sa] = h;
			if (h > 0) {
				h--;
			}
		}
	}
	lcparray[0] = 0;
	lcparray[strLen] = 0;
	if (suffixarray->invPosArray == NULL) {
		suffixarray->invPosArray = invPosArray;
	}
  suffixarray->lcpArray = lcparray;
	return lcparray;
}


/*---------------------------------------------------------------------*/
Kbs_Ulong kbs_MaxLcpFromSuffixArray(Kbs_SuffixArray *const suffixarray) {
	if (suffixarray == NULL || suffixarray->str == NULL || suffixarray->str->str == NULL || suffixarray->posArray == NULL) {
		KBS_ERROR(KBS_ERROR_NULLPOINTER);
		return (-1);
	}
  if (suffixarray->lcpArray == NULL) {
    kbs_lcpArrayFromSuffixArray(suffixarray);
  }
  if (suffixarray->lcpArray == NULL) {
    KBS_ERROR(KBS_ERROR_NULLPOINTER);
    return (-1);
  }
  register const Kbs_Ulong strLen = suffixarray->str->strLength;
  register const Kbs_Ulong *const lcpArray = suffixarray->lcpArray;
	register Kbs_Ulong maxlcp = 0;
  register Kbs_Ulong i;
	for (i=1; i<strLen; i++) {
    if (lcpArray[i] > maxlcp) {
      maxlcp = lcpArray[i];
    }
	}
	return maxlcp;
}

/*---------------------------------------------------------------------*/
Kbs_Ulong kbs_AvgLcpFromSuffixArray(Kbs_SuffixArray *const suffixarray) {
	if (suffixarray == NULL || suffixarray->str == NULL || suffixarray->str->str == NULL || suffixarray->posArray == NULL) {
		KBS_ERROR(KBS_ERROR_NULLPOINTER);
		return (-1);
	}
  if (suffixarray->lcpArray == NULL) {
    kbs_lcpArrayFromSuffixArray(suffixarray);
  }
  if (suffixarray->lcpArray == NULL) {
    KBS_ERROR(KBS_ERROR_NULLPOINTER);
    return (-1);
  }
  Kbs_Ulong strLen = suffixarray->str->strLength;
  register const Kbs_Ulong *const lcpArray = suffixarray->lcpArray;
  register Kbs_Ulong maxlcp = 0;
  register Kbs_Ulong i;
  register Kbs_Ulong sumlcp = 0;
  Kbs_Ulong avglcp = 0;
  for (i=1; i<strLen; i++) {
    sumlcp += lcpArray[i];
    if (sumlcp >=strLen-1) {
      avglcp++;
      sumlcp = sumlcp - (strLen - 1);
    }
  }
	return avglcp;
}

