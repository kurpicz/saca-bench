
#ifndef KBS_SUFFIXARRAYCONSTDSTEPANDPRE_H
#define KBS_SUFFIXARRAYCONSTDSTEPANDPRE_H

#include "kbs_String.h"
#include "kbs_SuffixArray.h"
#include "kbs_Time.h"
#include "kbs_Types.h"



/**
 * Build the suffix array with the bpr algorithm.
 * The bucket references are realized via pointers refering into the suffix array.
 * For each suffix its final position or the last position in its bucket is
 * hold in an separate array.
 * First the smallest level-1 buckets are determined
 * For the previously sorted level-1 bucket 'a', the 2-level buckets 'ya' are determined
 * by a left-to-right scan of bucket 'a'.
 * The space demands are up to 10*(seq->strLength)
 * @param seq - string to build suffix array for.
 * @param q - prefix length for which the initial bucket sort is performed.
 * @return - the suffix array for the string seq, where invPosArray = bucketIndices = NULL.
 */
Kbs_SuffixArray *kbs_buildDstepUsePrePlusCopyFreqOrder_SuffixArray(Kbs_Ustring *const seq, register const Kbs_Ulong q);



#endif
