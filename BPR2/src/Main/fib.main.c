#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "kbs_Error.h"
#include "kbs_FibonacciString.h"
#include "kbs_String.h"
#include "kbs_Types.h"

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("%d\n", (int) argc);
		printf("usage: fibstring <directory to write target string> <string length>\n");
		exit(0);
	}
	Kbs_Ulong length = atoi(argv[2]);
	if (chdir(argv[1]) == -1) {
		printf("unable to change directory %s\n", argv[1]);
		KBS_ERROR(KBS_ERROR_FILE);
		exit(KBS_ERROR_FILE);
	}
	Kbs_Ustring *string = kbs_genFibonacciLen_Ustring(length);
	if (string == NULL || string->str == NULL || string->strLength == 0) {
		printf("failed to generate periodic string - failed\n");
		KBS_ERROR(KBS_ERROR_NULLPOINTER);
		exit(KBS_ERROR_NULLPOINTER);
	}
	Kbs_Char *fibStrFilename = kbs_genFibonacciFileName(length);
	FILE *file = fopen(fibStrFilename, "wb+");
	if (file == NULL) {
		printf("failed to open file - %s failed\n", fibStrFilename);
		free(fibStrFilename);
		KBS_ERROR(KBS_ERROR_FILEOPEN);
		exit(KBS_ERROR_FILEOPEN);
	}
	if ( fwrite(string->str, sizeof(Kbs_Uchar), length, file) != length) {
		printf("File %s not written completely\n", fibStrFilename);
	}
	fclose(file);
	printf("File %s successfully generated\n", fibStrFilename);
	free(fibStrFilename);
  return 0;

} /* main() */
