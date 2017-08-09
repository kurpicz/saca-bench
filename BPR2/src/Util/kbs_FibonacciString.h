#ifndef KBS_FIBONACCISTRING_H
#define KBS_FIBONACCISTRING_H

#include <stdio.h>
#include <string.h>

#include "kbs_Error.h"
#include "kbs_String.h"
#include "kbs_Types.h"



/*---------------------------------------------------------------------------*/
/* Write Fibonacci string of length (see Crochemore/Rytter, 1995) to fptr.
 * @param length - length of string to be generated
 * @return Fibonacci string of size length
 */
Kbs_Ustring *kbs_genFibonacciLen_Ustring(const Kbs_Ulong length);

/*---------------------------------------------------------------------------*/
/* Generates filename for Fibonacci string of length fileSize
 * @param fileSize - length of string to be generated
 * @return Fibonacci string of length fileSize
 */
Kbs_Char *kbs_genFibonacciFileName(Kbs_Ulong fileSize);

#endif
