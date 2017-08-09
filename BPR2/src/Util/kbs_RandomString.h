#ifndef KBS_RANDOMSTRING_H
#define KBS_RANDOMSTRING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "kbs_Types.h"
#include "kbs_String.h"


/* Prototypes */


Kbs_Uchar kbs_genRand_UChar(Kbs_Uint alphabetSize);
Kbs_Ustring *kbs_genRand_UString(Kbs_Uint alphabetSize, Kbs_Ulong seqSize);
Kbs_Ustring *kbs_genRandPeriodic_UString(Kbs_Uint alphabetSize, Kbs_Ulong lengthPeriod, Kbs_Ulong strLength);


#endif
