#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <config.h>

#include "kbs_Error.h"
#include "kbs_Math.h"
#include "kbs_String.h"
#include "kbs_SuffixArray.h"
#include "kbs_SuffixArrayAnnotated.h"
#include "kbs_SuffixArrayChecker.h"
#include "kbs_SuffixArrayConstDStepAndPre.h"
#include "kbs_Types.h"

int main(int argc, char *argv[]) {
	Kbs_Ustring* ustr = NULL;
	Kbs_TimeStamp *beginTime;
	Kbs_TimeStamp *endTime;
	Kbs_TimeStamp *diffTime;
	Kbs_SuffixArray *sa = NULL;
	Kbs_Ulong q = 3;

	if (argc > 3 || argc < 2) {
		printf("usage: bktref <options> <file to construct suffix array for> \n");
		exit(1);
	}
	if (!strcmp(argv[1], "--version")) {
		printf(PACKAGE_STRING);
		printf("\n");
		exit(0);
	}
	if (!strcmp(argv[1], "--info")) {
		printf("Version: ");
		printf(PACKAGE_STRING);
		printf("\n");
		printf("Report bugs to ");
		printf(PACKAGE_BUGREPORT);
		printf("\n");
		exit(0);
	}
	if (access(argv[argc-1],F_OK)) {
		printf("file %s does not exist\n", argv[argc-1]);
		exit(1);
	}
	if (access(argv[argc-1],R_OK)) {
		printf("file %s is not readable\n", argv[argc-1]);
		exit(1);
	}
	if (argc == 3) {
		Kbs_Uchar *optionstr = argv[1];
		if (*optionstr != '-') {
			printf("%s is not valid option\n", argv[1]);
			exit(1);
		}
		optionstr++;
		if (*optionstr != 'q') {
			printf("%s is not valid option\n", argv[1]);
			exit(1);
		}
		optionstr++;
		if (*optionstr != '=') {
			printf("%s is not valid option\n", argv[1]);
			exit(1);
		}
		optionstr++;
		q = atoi(optionstr);
		if (q < 2) {
			printf("%d is not valid value for q\n", (int)q);
			exit(1);
		}
	}
  ustr = kbs_getUstring_FromFile(argv[argc-1]);

  beginTime = kbs_get_TimeStamp();
  kbs_get_AlphabetForUstring(ustr);
	if (ustr == NULL) {
		printf("unable to get file %s\n", argv[argc-1]);
		KBS_ERROR(KBS_ERROR_NULLPOINTER);
		exit(1);
	}
	if (argc == 2) {
		if (ustr->alphabet->alphaSize <= 9) {
			q = 7;
		}
		if (9 < ustr->alphabet->alphaSize && ustr->alphabet->alphaSize <= 13) {
			q = 6;
		}
		if (13 < ustr->alphabet->alphaSize && ustr->alphabet->alphaSize <= 21) {
			q = 5;
		}
		if (13 < ustr->alphabet->alphaSize && ustr->alphabet->alphaSize <= 21) {
			q = 5;
		}
		if (21 < ustr->alphabet->alphaSize && ustr->alphabet->alphaSize <= 46) {
			q = 4;
		}
		if (46 < ustr->alphabet->alphaSize) {
			q = 3;
		}
	}
	printf("\nstring of file %s with alphabet of size %d, q=%d\n", argv[argc-1], (int)(ustr->alphabet->alphaSize), (int)q);
	/* implementation using direct pointers as bucket pointers */
	sa = kbs_buildDstepUsePrePlusCopyFreqOrder_SuffixArray(ustr, q);
	endTime = kbs_get_TimeStamp();

  /* This is for checking the correctness of the suffix array with Burkhardt and Kaerkkaeinen's algorithm */
// 	if (!kbs_check_SuffixArray(sa)) {
//  		printf("Suffix array is not correct\n");
//   }
//   else {printf("Suffix array successfully checked\n");}

  /* This is to compute the maximal and average LCP of suffixes in the LCP table */
//   printf("Maximal LCP: %d\n", (int)kbs_MaxLcpFromSuffixArray(sa));
//   printf("Average LCP: %d\n", (int)kbs_AvgLcpFromSuffixArray(sa));

	kbs_delete_SA_IncludingString(sa);
	diffTime = kbs_get_DifferenceTimeStamp(beginTime, endTime);
	printf("Real Time: %10d.%06d\n", (int)diffTime->realtime.tv_sec, (int)diffTime->realtime.tv_usec);
	printf("Cpu time:\t%10.4f\n", (diffTime->systemTime.tms_utime)/(Kbs_Double)sysconf(_SC_CLK_TCK));
	printf("Sys time:\t%10.4f\n", (diffTime->systemTime.tms_stime)/(Kbs_Double)sysconf(_SC_CLK_TCK));
	printf("Cpu+Sys time:\t%10.4f\n\n", (diffTime->systemTime.tms_utime + diffTime->systemTime.tms_stime )/(Kbs_Double)sysconf(_SC_CLK_TCK));
	free(beginTime);
	free(endTime);
	free(diffTime);
	return 0;
} /* main() */
