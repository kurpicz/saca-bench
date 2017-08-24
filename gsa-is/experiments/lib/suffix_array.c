#include "suffix_array.h"

#define chr(i) (cs==sizeof(int_t)?((int_t*)T)[i]:((unsigned char *)T)[i])

/*******************************************************************/

int suffix_array_write(int_t *SA, int_t n, char* c_file, const char* ext){

	FILE *f_out;
	char *c_out = malloc((strlen(c_file)+strlen(ext)+3)*sizeof(char));
	
	sprintf(c_out, "%s.%s", c_file, ext);
	f_out = file_open(c_out, "wb");
	
	fwrite(SA, sizeof(int_t), n, f_out);
	
	file_close(f_out);
	free(c_out);

return 1;
}

/*******************************************************************/

int_t suffix_array_read(int_t** SA, char* c_file, const char* ext){

        FILE *f_in;
        char *c_in = malloc((strlen(c_file)+strlen(ext)+3)*sizeof(char));

        sprintf(c_in, "%s.%s", c_file, ext);
        f_in = file_open(c_in, "rb");

	fseek(f_in, 0L, SEEK_END);
	size_t size = ftell(f_in);
	rewind(f_in);
	
	int_t n = size/sizeof(int_t);

        *SA = (int_t*) malloc(n*sizeof(int_t));
        fread(*SA, sizeof(int_t), n, f_in);

        file_close(f_in);
        free(c_in);

return n;
}

/*******************************************************************/

int_t suffix_array_print(unsigned char *T, int_t *SA, size_t n, int cs){

int_t i;
	for(i=0; i<n; i++){

		printf("%" PRIdN ") %" PRIdN "\t", i, SA[i]);
	
		int_t j=SA[i];
		for(j=SA[i]; (j<SA[i]+5); j++)
			printf("%" PRIdN " ", chr(j));
		printf("\n");
	}

return 1;
}

/*******************************************************************/

int_t sleq(unsigned char *T, int_t a, int_t b, size_t n, int cs, unsigned char sentinel) {

	size_t i;

	for(i=0; i<n; i++){

	        if (chr(a) < chr(b))
			return 1;
	        else if (chr(a) > chr(b))
        	        return 0;
		else if(chr(a)==sentinel && chr(b)==sentinel){// $_i < $_j iff i < j
			if(a < b) 
				return 1;
			else 	
				return 0;
		}
		a++;
		b++;
        }

return 1;
} 

/*******************************************************************/

int_t suffix_array_check(unsigned char *T, int_t *SA,  size_t n, int cs, unsigned char sentinel){

	int_t i,j,k;
	
	for (i = 0; i < n-1;  i++) {
		size_t min = SA[i+1]<SA[i]?(n-SA[i]):(n-SA[i+1]);
		if (!sleq(T, SA[i], SA[i+1], min, cs, sentinel)){

			printf("#%" PRIdN ") %" PRIdN ", %" PRIdN "&\n", i, SA[i], SA[i+1]);

			for(j=SA[i], k=SA[i+1]; (j<SA[i]+5); j++, k++)
				printf("%" PRIdN " | %" PRIdN "\n", chr(j), chr(k));
			printf("\n");

			return 0;
		}
	}

	unsigned char *tmp = (unsigned char*) malloc(n*sizeof(unsigned char));

	for (i = 0; i < n;  i++) 
		tmp[i] = 0;

	for (i = 0; i < n;  i++) 
		tmp[SA[i]] = 1;
	
	for (i = 0; i < n;  i++){ 
		if(!tmp[i]){
			free(tmp);
			fprintf(stderr, "Error: is not a permutation\n");
			return 0;
		}
	}

	free(tmp);
	
return 1;  
}

/*******************************************************************/
