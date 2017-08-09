
#ifndef KBS_STRING_H
#define KBS_STRING_H

#include "kbs_Alphabet.h"
#include "kbs_Types.h"


typedef struct kbs_ustring{
	Kbs_Ulong 	strLength;      /** length of the string */
	Kbs_Uchar 	*str;           /** character array of length strLength plus terminating 0 */
	Kbs_Alphabet *alphabet;     /** the alphabet of the string, if it is determined previously */
}Kbs_Ustring;


#define KBS_STRING_EXTENSION_SIZE 32



/*----------------------------------------------------------------------------*/
/**
 * Gets the Kbs_Ustring from a given file without the alphabet
 * @param filename - file containing the string.
 * @return Kbs_Ustring located in filename
 * @see kbs_getUstringWithAlphabet_FromFile
 */
Kbs_Ustring* kbs_getUstring_FromFile(const Kbs_Char *const filename);


/*----------------------------------------------------------------------------*/
/**
 * Gets the Kbs_Ustring with its alphabet from a given file
 * @param filename - file containing the string.
 * @return Kbs_Ustring located in filename
 * @see kbs_getUstring_FromFile
 */
Kbs_Ustring* kbs_getUstringWithAlphabet_FromFile(Kbs_Char *filename);

/*----------------------------------------------------------------------------*/
/**
 * frees a Kbs_Ustring
 * @param oldStr - Kbs_Ustring string to free
 */
void kbs_delete_Ustring(Kbs_Ustring* oldStr);


/**
 * Shows the contents of Kbs_Ustring on standard out
 * @param thisString string to be shown
 * @see Kbs_Ustring
 */
void kbs_get_AlphabetForUstring(Kbs_Ustring *thisString);




#endif
