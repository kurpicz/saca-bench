
#include "stdlib.h"
#include "stdio.h"
#include "kbs_Types.h"
#include "kbs_Error.h"

void kbs_Error(Kbs_Uchar *filename, Kbs_Uint line, Kbs_Int errorCode) {
	fprintf(stderr, "Error in file %s, line %u:\n  ", filename, line);
	if (errorCode == KBS_ERROR_MALLOC) {
		fprintf(stderr, "Allocation Error, not enough space\n");
		exit(KBS_ERROR_MALLOC);
	}
	if (errorCode == KBS_ERROR_FILESTAT) {
		fprintf(stderr, "Unable to get stat of file\n");
	}
	if (errorCode == KBS_ERROR_FILEMODE) {
		fprintf(stderr, "File is not in appropriate mode\n");
	}
	if (errorCode == KBS_ERROR_FILEOPEN) {
		fprintf(stderr, "Unable to open file\n");
	}
	if (errorCode == KBS_ERROR_MMAP) {
		fprintf(stderr, "Unable to compute memory mapping\n");
	}
	if (errorCode == KBS_ERROR_FILEFORMAT) {
		fprintf(stderr, "The file has not the appropriate format\n");
	}
	if (errorCode == KBS_ERROR_VALUEOVERFLOW) {
		fprintf(stderr, "Overflow of the given type\n");
	}
	if (errorCode == KBS_ERROR_NULLPOINTER) {
		fprintf(stderr, "Try to access a NULL reference\n");
	}
	if (errorCode == KBS_ERROR_TIME) {
		fprintf(stderr, "Unable to process the time\n");
	}
	if (errorCode == KBS_ERROR_VALUEOUTOFBOUNDS) {
		fprintf(stderr, "The value of a variable is out of a given range\n");
	}
	if (errorCode == KBS_ERROR_EMPTYCONTAINER) {
		fprintf(stderr, "Try to access elements of an empty set/container\n");
	}

}

