
#ifndef KBS_SUFFIXARRAYANNOTATED_H
#define KBS_SUFFIXARRAYANNOTATED_H

#include "kbs_String.h"
#include "kbs_Time.h"
#include "kbs_Types.h"




/**
 * Lcp table computation due to Kasai et al.
 * @param suffixarray - to compute lcp table for
 * @param invSA - inverse suffix array if available or NULL
 * @return lcp table
 */
Kbs_Ulong *kbs_lcpArrayFromSuffixArray(Kbs_SuffixArray *const suffixarray);


/**
 * @param suffixarray - to compute lcp table for
 * @param invSA - inverse suffix array if available or NULL
 * @return maximal longest common prefix
 */
Kbs_Ulong kbs_MaxLcpFromSuffixArray(Kbs_SuffixArray *const suffixarray);


/**
 * @param suffixarray - to compute lcp table for
 * @param invSA - inverse suffix array if available or NULL
 * @return average longest common prefix
 */
Kbs_Ulong kbs_AvgLcpFromSuffixArray(Kbs_SuffixArray *const suffixarray);


#endif
