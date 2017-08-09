
#ifndef KBS_SUFFIXARRAY_H
#define KBS_SUFFIXARRAY_H

#include "kbs_String.h"
#include "kbs_Time.h"
#include "kbs_Types.h"

/** suffix array with its inverse and bucket pointers with respect to qGramSize
 */
typedef struct kbs_suffixarray{
	Kbs_Ustring *str;						/** string for which this is the suffix array */
	Kbs_Ulong *posArray;				/** the position array for str */
  Kbs_Ulong **invPosArray;    /** the inverse pointers */
  Kbs_Ulong *lcpArray;        /** the LCP array */
	Kbs_Ulong **buckets;		    /** the bucket table pointer according to prefixes of length qGramSize */
	Kbs_Ulong qGramSize;				/** prefix length for the buckets */
}Kbs_SuffixArray;



#define KBS_INSSORT_THRES_LEN 15


/**
 * Frees the suffix array
 * @param oldSA suffix array to be freed
 */
void kbs_delete_SA_WithoutString(Kbs_SuffixArray *oldSA);

/**
 * Frees the suffix array
 * @param oldSA suffix array to be freed
 */
void kbs_delete_SA_IncludingString(Kbs_SuffixArray *oldSA);


/**
 *
 */
void kbs_show_SA(const Kbs_SuffixArray *const sa);

/**
 *
 */
Kbs_Bool kbs_isEqual_SA(const Kbs_SuffixArray *const sa1, const Kbs_SuffixArray *const sa2);



/**
 *
 */
void kbs_show_LongArray(Kbs_Long *longArray, Kbs_Ulong length);

#endif
