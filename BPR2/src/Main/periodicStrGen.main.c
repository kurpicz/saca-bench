#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "randomlib.h"
#include "kbs_Error.h"
#include "kbs_RandomString.h"
#include "kbs_RandomStringFile.h"
#include "kbs_Types.h"

/*--------------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
	if (argc != 5) {
		printf("%d\n", (int) argc);
		printf("usage: periodicstr <directory to write target string> <alphabet size> <period length> <string length>\n");
		exit(0);
	}
	Kbs_Uint alphabetSize = atoi(argv[2]);
	Kbs_Ulong lengthPeriod = atoi(argv[3]);
	Kbs_Ulong strLength = atoi(argv[4]);
	if (chdir(argv[1]) == -1) {
		printf("\nVerzeichnis %s nicht gefunden\n", argv[1]);
		KBS_ERROR(KBS_ERROR_FILE);
		exit(KBS_ERROR_FILE);
	}
	Kbs_Ustring *string = kbs_genRandPeriodic_UString(alphabetSize, lengthPeriod, strLength);
	if (string == NULL || string->str == NULL || string->strLength == 0) {
		printf("failed to generate periodic string - failed\n");
		KBS_ERROR(KBS_ERROR_NULLPOINTER);
		exit(KBS_ERROR_NULLPOINTER);
	}
	Kbs_Char *periodicStrFilename = kbs_generate_PeriodicStrFileName(alphabetSize, lengthPeriod, strLength);
	FILE *file = fopen(periodicStrFilename, "wb+");
	if (file == NULL) {
		printf("failed to open file - %s failed\n", periodicStrFilename);
		free(periodicStrFilename);
		KBS_ERROR(KBS_ERROR_FILEOPEN);
		exit(KBS_ERROR_FILEOPEN);
	}
	if ( fwrite(string->str, sizeof(Kbs_Uchar), strLength, file) != strLength) {
		printf("File %s not written completely\n", periodicStrFilename);
	}
	fclose(file);
	printf("File %s successfully generated\n", periodicStrFilename);
	free(periodicStrFilename);
  return 0;
}
