/******************************************************************************
 *  FILE   : words.c
 *  AUTHOR : Jens Stoye
 *  DATE   : 12/1997
 *****************************************************************************/

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <math.h> // sqrt(),pow()

#include "kbs_Error.h"
#include "kbs_FibonacciString.h"
#include "kbs_String.h"
#include "kbs_Types.h"



/*---------------------------------------------------------------------------*/
/* Write Fibonacci string of length (see Crochemore/Rytter, 1995) to fptr.
 *
 */
Kbs_Ustring *kbs_genFibonacciLen_Ustring(const Kbs_Ulong length) {

	if (length == 0) {
		return NULL;
	}
	Kbs_Ustring *fibString = (Kbs_Ustring *)malloc(sizeof(Kbs_Ustring *));
	if (fibString == NULL) {
		KBS_ERROR(KBS_ERROR_MALLOC);
	}
	fibString->strLength = length;
	fibString->str = (Kbs_Uchar *)malloc(sizeof(Kbs_Uchar) * (length+1));
	if (fibString->str == NULL) {
		KBS_ERROR(KBS_ERROR_MALLOC);
	}
	if(length == 1) fibString->str[0] = 'b';
  else {
    fibString->str[0] = 'a';
    fibString->str[1] = 'b';
    Kbs_Ulong l1 = 1;
    Kbs_Ulong l2 = 2;
		while (l2 < length) {
			Kbs_Ulong k;
			for(k=0; k<l1; k++) {
				if (l2+k >= length) {
					break;
				}
				fibString->str[l2+k] = fibString->str[k];
      }
			Kbs_Ulong tmp = l1;
      l1 = l2;
      l2 += tmp;
		}
  }
  fibString->str[length] = '\0';
	return fibString;
} /* fib() */


/*--------------------------------------------------------------------------*/
Kbs_Char *kbs_genFibonacciFileName(Kbs_Ulong fileSize) {
	Kbs_Char *filename;
	filename = (Kbs_Char *) malloc(sizeof(Kbs_Char) * 100);
	if (filename == NULL) {
		KBS_ERROR(KBS_ERROR_MALLOC);
	}
	memset(filename, '\0', 100);
	sprintf(filename,"fib_s%d", (int)fileSize);
	return filename;
}
