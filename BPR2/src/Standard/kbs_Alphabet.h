#ifndef KBS_ALPHABET_H
#define KBS_ALPHABET_H

#include "kbs_Types.h"

#define KBS_MAX_ALPHABET_SIZE 256


typedef struct kbs_alphabet{
	Kbs_Uint 		alphaSize;   /** size of the alphabet */
	Kbs_Uchar 	*charArray;  /** collection of characters in mapping order */
	Kbs_Int			alphaMapping[KBS_MAX_ALPHABET_SIZE]; /** map of characters to alphabet 0..alphabetSize-1 */
	Kbs_Ulong		charFreq[KBS_MAX_ALPHABET_SIZE]; /** frequency of each character in the resp. string  */
	Kbs_Uchar		numberSpecialChar;               /** just used for special purposes */
	Kbs_Uchar		*specialChar;                    /** array of special characters */
}Kbs_Alphabet;



/**
 * The alphabet mapping is sorted,
 * such that the smallest ascii-value is mapped to 0 and so on
 * @param alphabet to be mapped in sorted order
 */
void kbs_sort_Alphabet(Kbs_Alphabet *const alphabet);



/**
 * frees the alphabet
 * @param alphabet to be deleted
 */
void kbs_delete_Alphabet(Kbs_Alphabet *alphabet);


#endif
