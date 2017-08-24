#include "lcp_array.h"

#define chr(i) (cs==sizeof(int_t)?((int_t*)T)[i]:((unsigned char *)T)[i])

/*******************************************************************/

int find_inverse(int_t* SA, uint_t n, uint_t* A){
	
	uint_t i;
	for(i = 0; i < n; i++) A[SA[i]] = i;

return 0;
}

/*******************************************************************/

int lcp_kasai(char* T, int_t* SA, uint_t n, int_t* LCP){
	
	uint_t *I = (uint_t* ) malloc(n * sizeof(uint_t));
	find_inverse(SA, n, I);
	
	uint_t l = 0;
	uint_t j, k, i;
	
	for(i = 0; i < n; i++){

		k = I[i];
	
		if(!k) {
			LCP[k] = 0;
		}
		else{
			
			j = SA[k-1];
			while(T[i+l] == T[j+l] && !(T[i+l] == 0 && T[j+l]==0)){
				l++; 
			}
			
			LCP[k] = l;
			if(T[i+l] == 0 && T[j+l]==0) LCP[k]++;
			if(l>0) l--;
		}
	}	
	
	free(I);

return 0;
}

/*******************************************************************/

int lcp_PHI(unsigned char* T, int_t* SA, int_t* LCP, uint_t n, int cs, unsigned char separator){

	uint_t* PLCP = (uint_t*) malloc(n * sizeof(uint_t));;

	//PHI is stored in PLCP array
	uint_t i,j;
	for (i=0, j = 0; i < n; ++i) {
		PLCP[SA[i]] = j;
		j = SA[i];
	}

	int_t l;
	for (i=0, l=0; i < n-1; ++i) {
		while (chr(i+l) == chr(PLCP[i]+l) && !(chr(i+l) == separator && chr(PLCP[i]+l)==separator) ) {
		    ++l;
		}
		PLCP[i] = l;
		if (l) --l;
	}

	for (i=0; i < n; ++i) {
		LCP[i] = PLCP[SA[i]];
	}
	
	free(PLCP);

return 0;
}

/*******************************************************************/

int lcp_PHI_int(int_t* T, int_t* SA, int_t* LCP, uint_t n, int cs){

	uint_t* PLCP = (uint_t*) malloc(n * sizeof(uint_t));;

	//PHI is stored in PLCP array
	uint_t i,j;
	for (i=0, j = 0; i < n; ++i) {
		PLCP[SA[i]] = j;
		j = SA[i];
	}

	int_t l;
	for (i=0, l=0; i < n-1; ++i) {
		while (chr(i+l) == chr(PLCP[i]+l)) {
		    ++l;
		}
		PLCP[i] = l;
		if (l) --l;
	}

	for (i=0; i < n; ++i) {
		LCP[i] = PLCP[SA[i]];
	}
	
	free(PLCP);

return 0;
}
/*******************************************************************/
int lcp_array_check(unsigned char *T, int_t *SA, int_t *LCP, uint_t n, int cs, unsigned char separator){
	
	uint_t i,j,k;
	uint_t h;
	uint64_t sum=0;
	int_t maximum=0;

	for(i=1;i<n;i++) {
		
		j=SA[i-1]; k=SA[i];
		for(h=0;j+h<n && k+h<n;h++) if(chr(j+h)!=chr(k+h) || (chr(j+h)==separator && chr(k+h)==separator)) break;
		
		if(LCP[i]!=h) {
			fprintf(stdout,"isNotLCP! Incorrect LCP value: LCP[%" PRIdN "]=%" PRIdN "!=%" PRIdN "\t(%" PRIdN ")\n", i, LCP[i],h, SA[i]);
	//		return 0;
		}
		sum+=LCP[i];
		maximum=max(maximum, LCP[i]);
	}
	
	printf("LCP_mean = %.2lf\n", (double)sum/(double)n);
	printf("LCP_max = %" PRIdN "\n", maximum);
	
return 1;
}

/*******************************************************************/
int lcp_array_check_phi(unsigned char *T, int_t *SA, int_t *LCP, uint_t n, int cs, unsigned char separator){
        
        uint_t i;
        uint64_t sum=0;
        int_t maximum=0;
        int_t previous_lcp = 1;

        uint_t *ISA = (uint_t* ) malloc(n * sizeof(uint_t));
        find_inverse(SA, n, ISA);

        for(i=1;i<n;i++) {

                int_t k=ISA[i]; 
               
                if(k==0) continue;

                int_t j=SA[ISA[i]-1];
        
                //j=SA[i-1]; k=SA[i];
                int_t h=previous_lcp-1;
                for(h=0;i+h<n && j+h<n;h++) if(chr(i+h)!=chr(j+h) || (chr(i+h)==separator && chr(j+h)==separator)) break;

                if(LCP[k]!=h) {
                        fprintf(stdout,"isNotLCP! Incorrect LCP value: LCP[%" PRIdN "]=%" PRIdN "!=%" PRIdN "\t(%" PRIdN ")\n", k, LCP[k],h, SA[i]);
                        //return 0;
                }
                sum+=LCP[i];
                maximum=max(maximum, LCP[i]);

                previous_lcp = max(1,LCP[i]);
        }

        printf("LCP_mean = %.2lf\n", (double)sum/(double)n);
        printf("LCP_max = %" PRIdN "\n", maximum);

        free(ISA);

return 1;
}

/*******************************************************************/
int lcp_array_check_lcp(unsigned char *T, int_t *SA, int_t *LCP, uint_t n, int cs, unsigned char separator){
	
	uint_t i;
	uint64_t sum=0;
	int_t maximum=0;

	int_t *LCP2 = (int_t*) malloc(n*sizeof(int_t));
	for(i=0; i<n; i++) LCP2[i]=0;

	time_t t_start=0;
	clock_t c_start=0;

	time_start(&t_start, &c_start);
	lcp_PHI(T, SA, LCP2, n, sizeof(char), 1);//separator=1
	fprintf(stderr,"%.6lf\n", time_stop(t_start, c_start));

	for(i=0; i<n; i++){
	        if(LCP[i]!=LCP2[i]){
			fprintf(stdout,"isNotLCP! Incorrect LCP value: LCP[%" PRIdN "]=%" PRIdN "!=%" PRIdN "\t(%" PRIdN ")\n", i, LCP[i],LCP2[i], SA[i]);
	        }

		sum+=LCP[i];
		maximum=max(maximum, LCP[i]);
	}
	
	printf("LCP_mean = %.2lf\n", (double)sum/(double)n);
	printf("LCP_max = %" PRIdN "\n", maximum);

	fprintf(stderr, "%.2lf\t %" PRIdN "\n", (double)sum/(double)n, maximum);

	free(LCP2);
	
return 1;
}


/*******************************************************************/

int lcp_array_print(unsigned char *T, int_t *SA, int_t *LCP, size_t n, int cs){

	int_t i;

        for(i=0; i<n; i++){

                printf("%" PRIdN ") %" PRIdN ", %" PRIdN "  \t", i, SA[i], LCP[i]);

                int_t j=SA[i];
                for(j=SA[i]; (j<SA[i]+min(10,LCP[i]+10)); j++)
                        printf("%" PRIdN " ", chr(j));
                printf("\n");
        }

return 1;
}

/*******************************************************************/ 

int lcp_array_write(int_t *LCP, int_t n, char* c_file, const char* ext){

        FILE *f_out;
        char *c_out = malloc((strlen(c_file)+strlen(ext)+3)*sizeof(char));
        
        sprintf(c_out, "%s.%s", c_file, ext);
        f_out = file_open(c_out, "wb");
        
	int_t i;
	for(i=0; i<n; i++){//writes LCP 
	        fwrite(&LCP[i], sizeof(int_t), 1, f_out);
	}

        file_close(f_out);
        free(c_out);

return 1;
}


/*******************************************************************/

int_t lcp_array_read(int_t** LCP, char* c_file, const char* ext){

        FILE *f_in;
        char *c_in = malloc((strlen(c_file)+strlen(ext))*sizeof(char));

        sprintf(c_in, "%s.%s", c_file, ext);
        f_in = file_open(c_in, "rb");

	fseek(f_in, 0L, SEEK_END);
	size_t size = ftell(f_in);
	rewind(f_in);
	
	int_t n = size/sizeof(int_t);

        *LCP = (int_t*) malloc(n*sizeof(int_t));
        fread(*LCP, sizeof(int_t), n, f_in);

        file_close(f_in);
        free(c_in);

return n;
}

/*******************************************************************/
