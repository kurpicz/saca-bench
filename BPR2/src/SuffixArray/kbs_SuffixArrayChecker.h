
#ifndef KBS_SUFFIXARRAYCHECKER_H
#define KBS_SUFFIXARRAYCHECKER_H

#include "kbs_SuffixArray.h"
#include "kbs_Types.h"




/**
An array sa[0,n-1] of length n is a suffix array of
a string s[0,n-1] if and only if the following conditions hold <br>
1. For all i between 0 and n-1 => sa[i] is between 0 and n-1 <br>
2. For all i between 1 and n-1 => s[sa[i-1]]<=s[sa[i]] is between 0 and n-1 <br>
3. For all i between 1 and n-1: <br>
  if s[sa[i-1]]=s[sa[i]] and sa[i-1] != n-1 <br>
	=> there exist j,k between 0 and n-1
	such that sa[j]=sa[i-1]+1 and sa[k]=sa[i]+1

*/

/**
 * straight forward checking of above conditions
 * @param sa suffix array to be checked for correctness
 * @return suffix array is correct
 */
Kbs_Bool kbs_check_SuffixArray(const Kbs_SuffixArray *const sa);


#endif
