
#ifndef KBS_ERROR_H
#define KBS_ERROR_H

#include "kbs_Types.h"

#define KBS_ERROR_MALLOC 						1     /** identifies allocation error*/
#define KBS_ERROR_FILESTAT 					2     /** identifies error to get stat of file */
#define KBS_ERROR_FILEMODE 					3     /** identifies error in file permission  */
#define KBS_ERROR_FILEOPEN 					4     /** identifies error in opening a file   */
#define KBS_ERROR_MMAP							5     /** identifies mmap error*/
#define KBS_ERROR_FILEFORMAT				6     /** */
#define KBS_ERROR_VALUEOVERFLOW			7     /** */
#define KBS_ERROR_NULLPOINTER				8     /** */
#define KBS_ERROR_TIME							9     /** */
#define KBS_ERROR_VALUEOUTOFBOUNDS	10    /** */
#define KBS_ERROR_EMPTYCONTAINER		11    /** */
#define KBS_ERROR_FILE							12    /** identifies error in file handling */
#define KBS_ERROR_OTHER							13    /** */

#define KBS_ERROR(CODE)         kbs_Error((Kbs_Uchar *)__FILE__, __LINE__, CODE);

void kbs_Error(Kbs_Uchar *filename, Kbs_Uint line, Kbs_Int errorCode);

#endif
