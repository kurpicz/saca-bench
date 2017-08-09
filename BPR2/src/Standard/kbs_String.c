#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include "kbs_Error.h"
#include "kbs_String.h"
#include "kbs_Types.h"





/* ------------------------------------------------------------------------*/
Kbs_Ustring* kbs_getUstring_FromFile(const Kbs_Char *const filename) {
	struct stat fileInfo;
	Kbs_Ustring *newStr;
	FILE *file;
	if(filename == NULL) {
		KBS_ERROR(KBS_ERROR_NULLPOINTER);
		return NULL;
	}
	newStr = (Kbs_Ustring *) malloc(sizeof(Kbs_Ustring));
	if (newStr == NULL) {
		KBS_ERROR(KBS_ERROR_MALLOC);
	}
	if ( stat(filename,	&fileInfo ) ) {
		free(newStr);
	  printf("Unable to get stat of file %s \n", filename);
		KBS_ERROR(KBS_ERROR_FILEOPEN);
		return NULL;
	}
	newStr->strLength = fileInfo.st_size;
  newStr->alphabet = NULL;
	newStr->str = (Kbs_Uchar *) malloc(((fileInfo.st_size) + sizeof(Kbs_Uchar)*KBS_STRING_EXTENSION_SIZE) * sizeof(Kbs_Uchar));
	if (newStr->str == NULL) {
		KBS_ERROR(KBS_ERROR_MALLOC);
	}
	if ( !(file = fopen(filename, "r")) ) {
		free(newStr->str);
		free(newStr);
		printf("Unable to open file %s \n", filename);
		KBS_ERROR(KBS_ERROR_FILEOPEN);
		return NULL;
	}
	rewind(file);
	if ( newStr->strLength > fread(newStr->str, sizeof(Kbs_Uchar), newStr->strLength, file) ) {
		fclose(file);
		free(newStr->str);
		free(newStr);
		KBS_ERROR(KBS_ERROR_FILEOPEN);
		return NULL;
	}
	fclose(file);
	newStr->str[newStr->strLength] = '\0';
	return newStr;
}



/*------------------------------------------------------------------------*/
Kbs_Ustring* kbs_getUstringWithAlphabet_FromFile(Kbs_Char *filename) {
	Kbs_Ustring *newStr;
	newStr = kbs_getUstring_FromFile(filename);
	kbs_get_AlphabetForUstring(newStr);
	return newStr;
}


/*------------------------------------------------------------------------*/
void kbs_delete_Ustring(Kbs_Ustring* oldStr) {
	if(oldStr == NULL) {
		return;
	}
	if (oldStr->str != NULL){
		free(oldStr->str );
	}
	kbs_delete_Alphabet(oldStr->alphabet);
	free(oldStr);
}



/*----------------------------------------------------------------------------*/
void kbs_get_AlphabetForUstring(Kbs_Ustring *thisString) {
	Kbs_Ulong i;
	Kbs_Alphabet* alphabet;
	Kbs_Ulong stringLength;
	Kbs_Uchar tmpChar;
	if (thisString == NULL) {
		fprintf(stdout, "File %s line %u: Try to get alphabet of Null-String\n", __FILE__, __LINE__ );
		return;
	}
	alphabet = (Kbs_Alphabet *) malloc(sizeof(Kbs_Alphabet));
	if (alphabet == NULL) {
		KBS_ERROR(KBS_ERROR_MALLOC);
	}
	for (i=0; i<KBS_MAX_ALPHABET_SIZE; i++) {
		alphabet->charFreq[i] = 0;
	}
	alphabet->alphaSize = 0;
	stringLength = thisString->strLength;
	for (i=0; i<stringLength; i++) {
		tmpChar = thisString->str[i];
		if (alphabet->charFreq[tmpChar] == 0) {
			alphabet->alphaSize++;
		}
		alphabet->charFreq[tmpChar]++;
	}

	alphabet->charArray = (Kbs_Uchar *) calloc(alphabet->alphaSize+1, sizeof(Kbs_Uchar));
	if (alphabet->charArray == NULL) {
		KBS_ERROR(KBS_ERROR_MALLOC);
	}
	alphabet->charArray[alphabet->alphaSize] = 0;
	Kbs_Int k = 0;
	for (i=0; i<KBS_MAX_ALPHABET_SIZE; i++) {
		alphabet->alphaMapping[i] = -1;
		if (alphabet->charFreq[i] > 0) {
			alphabet->charArray[k] = i;
			alphabet->alphaMapping[i] = k;
			k++;
		}
	}
	if (k != alphabet->alphaSize) {
		printf("k ist ungleich alphabet size");
	}
	thisString->alphabet = alphabet;
}



