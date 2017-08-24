#include "document_array.h"

#define DEBUG   0

#define chr(i) (cs==sizeof(int_t)?((int_t*)T)[i]:((unsigned char *)T)[i])

/*******************************************************************/
int check_LF_array(int_t* SA, int_t* LF, uint_t n, unsigned char separator){
	
	uint_t i;
	uint_t* ISA = (uint_t*) malloc(n*sizeof(uint_t));

	for(i=0; i<n;i++) ISA[SA[i]]=i;

	for(i=0; i<n;i++) 
		if(SA[i]>0)
		if(ISA[SA[i]-1]!=LF[i]) {
			free(ISA);
			return 0;
		}

	free(ISA);

return 1;
}

/*******************************************************************/
int compute_LF_array(unsigned char* T, int_t* SA, int_t* LF, uint_t n, unsigned int SIGMA, int cs, unsigned char separator,  uint_t k){

	uint_t i, sum=0;
	unsigned char c; 
	uint_t* count = (uint_t*) malloc(SIGMA*sizeof(uint_t));

	for(i=0; i<SIGMA;i++) count[i]=0;
	for(i=0; i<n;i++) count[chr(i)]++;

	for(i=0; i<SIGMA;i++){
		sum += count[i];
		count[i]=sum-count[i];
	}

	if(!separator){//T^cat 1
		for(i=0; i<n;i++){
			if(SA[i]>0) c = chr(SA[i]-1);
			else c = 0;
			LF[i] = count[c]++;
		}
	}
	else{ //T^cat 2, if S_i==S_j, S_i < S_j iff i < j
		LF[0] = -1;
		for(i=1; i<n;i++){
			if(SA[i]>0) c = chr(SA[i]-1);
			else c = 0;
			
			if(c==separator) LF[i]=-1;
			else LF[i] = count[c]++;
		}
	
		uint_t j=0;	
		LF[0] = k--;
		for(i=1; i<n;i++){//LF-property does not hold for $-symbols (we considered each one as different symbols)
			j=LF[j];
			if(chr(SA[j]-1)==separator) LF[j]=k--;
		}
	}

	free(count);

return 0;
}

/*******************************************************************/

int document_array_LF(unsigned char* T, int_t* SA, int_t* DA, uint_t n, unsigned int SIGMA, int cs, unsigned char separator, uint_t k){

	uint_t tmp, i, j, count=k;
	compute_LF_array(T, SA, DA, n, SIGMA, cs, separator, k);

	#if DEBUG
	if(!check_LF_array(SA, DA, n, separator)) printf("isNotLF!!\n");
	else printf("isLF!!\n");
	#endif

	separator=1;
	j=0;
	for (i=0; i<n; i++){

		tmp = DA[j];
		if(chr(SA[j])==separator) count--;
		DA[j]=count;
		j=tmp;
	}

	#if DEBUG
	document_array_print(T, SA, DA, min(20,n), cs);
	#endif

return 0;
}

/*******************************************************************/
int compute_LF_array_int(int_t* T, int_t* SA, int_t* LF, uint_t n, unsigned int SIGMA, int cs, uint_t k){

	uint_t i, sum=0;
	int_t c; 
	uint_t* count = (uint_t*) malloc(SIGMA*sizeof(uint_t));

	for(i=0; i<SIGMA;i++) count[i]=0;
	for(i=0; i<n;i++) count[chr(i)]++;

	for(i=0; i<SIGMA;i++){
		sum += count[i];
		count[i]=sum-count[i];
	}

	LF[0] = k;
	for(i=1; i<n;i++){
		if(SA[i]>0) c = chr(SA[i]-1);
		else c = 0;
		LF[i] = count[c]++;
	}

	free(count);

return 0;
}

/*******************************************************************/

int document_array_LF_int(int_t* T, int_t* SA, int_t* DA, uint_t n, unsigned int SIGMA, int cs, unsigned char separator, uint_t k){

	uint_t tmp, i, j, count=k;
	compute_LF_array_int(T, SA, DA, n, SIGMA, cs, k);

	#if DEBUG
	if(!check_LF_array(SA, DA, n, separator)) printf("isNotLF!!\n");
	else printf("isLF!!\n");
	#endif

	j=0;
	for (i=0; i<n; i++){

		tmp = DA[j];
		if(chr(SA[j])<k) count--;
		DA[j]=count;
		j=tmp;
	}
	DA[0]=k;

	#if DEBUG
	document_array_print((unsigned char*)T, SA, DA, min(20,n), cs);
	#endif

return 0;
}

/*******************************************************************/

int document_array_check(unsigned char *T, int_t *SA, int_t *DA, uint_t n, int cs, unsigned char separator, uint_t k){

	uint_t i, count=k;
	int_t* R = (int_t*) malloc(n*sizeof(int_t));

	for(i=n-1; i>0;i--){
		if(chr(i)==separator) count--;
		R[i]=count;
	}
	R[0]=0;

	for(i=0; i<n;i++){
		if(DA[i]!=R[SA[i]]){
			free(R);
			return 0;
		}
	}

	free(R);	
	
return 1;
}

/*******************************************************************/

int document_array_check_int(int_t *T, int_t *SA, int_t *DA, uint_t n, int cs, uint_t k){

	uint_t i, count=k;
	int_t* R = (int_t*) malloc(n*sizeof(int_t));

	for(i=n-1; i>0;i--){
		if(chr(i)<k) count--;
		R[i]=count;
	}
	R[0]=0;

	for(i=0; i<n;i++){
		if(DA[i]!=R[SA[i]]){
			free(R);
			return 1;
		}
	}

	free(R);	
	
return 1;
}
/*******************************************************************/

int document_array_print(unsigned char *T, int_t *SA, int_t *DA, size_t n, int cs){

	int_t i;
        for(i=0; i<n; i++){

                printf("%" PRIdN ") %" PRIdN "\t %" PRIdN "  \t", i, SA[i], DA[i]);

                int_t j=SA[i];
                for(j=SA[i]; (j<SA[i]+10); j++)
                        printf("%" PRIdN " ", chr(j));
                printf("\n");
        }

return 1;
}

/*******************************************************************/ 

int document_array_write(int_t *DA, int_t n, char* c_file, const char* ext){

        FILE *f_out;
        char *c_out = malloc((strlen(c_file)+strlen(ext)+3)*sizeof(char));
        
        sprintf(c_out, "%s.%s", c_file, ext);
        f_out = file_open(c_out, "wb");
        
	int_t i;
	for(i=0; i<n; i++){//writes DA 
	        fwrite(&DA[i], sizeof(int_t), 1, f_out);
	}

        file_close(f_out);
        free(c_out);

return 1;
}


/*******************************************************************/

int_t document_array_read(int_t** DA, char* c_file, const char* ext){

        FILE *f_in;
        char *c_in = malloc((strlen(c_file)+strlen(ext))*sizeof(char));

        sprintf(c_in, "%s.%s", c_file, ext);
        f_in = file_open(c_in, "rb");

	fseek(f_in, 0L, SEEK_END);
	size_t size = ftell(f_in);
	rewind(f_in);
	
	int_t n = size/sizeof(int_t);

        *DA = (int_t*) malloc(n*sizeof(int_t));
        fread(*DA, sizeof(int_t), n, f_in);

        file_close(f_in);
        free(c_in);

return n;
}

/*******************************************************************/
