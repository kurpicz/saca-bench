#include <iostream>
#include <cstdio>
#include <cstdlib>

#include <sdsl/bit_vectors.hpp>
#include <sdsl/sd_vector.hpp>

#include "lib/utils.hpp"
#include "lib/document_array.hpp"

#ifndef DEBUG
        #define DEBUG   0
#endif

#ifndef SDV
        #define SDV   0
#endif

using namespace std;
using namespace sdsl;

int_t file_text_read(unsigned char** str, char* c_file, const char* ext);
int_t suffix_array_read(int_t** SA, char* c_file, const char* ext);
int file_chdir(char* dir);

/***********************************************************************/

int main(int argc, char** argv){

int VALIDATE=0, OUTPUT=0;
time_t t_start=0, t_total=0;
clock_t c_start=0, c_total=0;

        if(argc!=6){
		return 1;
        }

        int_t n=0;
        int   k;

        char* c_dir = argv[1];
        char* c_file = argv[2];

        sscanf(argv[3], "%d", &k);
        sscanf(argv[4], "%u", &VALIDATE);
        sscanf(argv[5], "%u", &OUTPUT);

	time_start(&t_total, &c_total);
	time_start(&t_start, &c_start);

        file_chdir(c_dir);


//loading

	unsigned char *str = NULL;
	n = file_text_read(&str, c_file, "str");

	int_t *SA = NULL;
	if(n != suffix_array_read(&SA, c_file, "sa")) return 1;

        printf("loading:\n");
	fprintf(stderr,"%.6lf\n", time_stop(t_start, c_start));

        printf("K = %" PRId32 "\n", k);
        printf("N = %" PRIdN "\n", n+1);
	printf("sizeof(int) = %zu bytes\n", sizeof(int_t));

	time_start(&t_start, &c_start);

	int_t ones=0;//counts the number of separators

	#if SDV	
        	printf("sparse bitvector:\n");
		sd_vector_builder builder(n,k);
	#else
		bit_vector b = bit_vector(n, 0);
	#endif

	//1. computes B[i]
	for(size_t i=0; i < (size_t) n; i++)
		if(str[i]==1){
			ones++;
			#if SDV	
				builder.set(i); 
			#else
				b[i] = 1; //separator==1			
			#endif
		}

	if(k!=ones){
		fprintf(stderr, "## ERROR: N. of strings in <%s> differs from %" PRId32" ##\n", c_file, k);
		return 1;
	}

	//2. free T
	//free(str);

	//3. compute rank_structure
	#if SDV	
		sd_vector<> sdv(builder);
		rank_support_sd<1> b_rank(&sdv);
	#else
		rank_support_v<1> b_rank(&b);
	#endif


	//4. malloc DA
	int_t *DA = (int_t*) malloc(n*sizeof(int_t));

	for(int_t i=0; i < n; i++)
		DA[i] = b_rank(SA[i]);

	/**/

	if(VALIDATE){

		//n = file_text_read(&str, c_file, "str");
		if(!document_array_check(str, SA, DA, n, sizeof(char), 1, k)) printf("isNotDA!!\n");
		else printf("isDA!!\n");
		#if DEBUG
			document_array_print(str, SA, DA, min(n,20), sizeof(char));
		#endif
		//free(str);
	}

	if(OUTPUT){
                document_array_write(DA, n, c_file, "da_bit");
	}
	

	fprintf(stderr,"%.6lf\n", time_stop(t_start, c_start));

        printf("total:\n");
        fprintf(stderr,"%.6lf\n", time_stop(t_total, c_total));

	free(str);
	free(SA);
	free(DA);

return 0;
}

/***********************************************************************/

int_t suffix_array_read(int_t** SA, char* c_file, const char* ext){

        FILE *f_in;
        char *c_in = (char*) malloc((strlen(c_file)+strlen(ext)+3)*sizeof(char));

        sprintf(c_in, "%s.%s", c_file, ext);
        f_in = fopen(c_in, "rb");
        if (!f_in) perror ("suffix_array_text_read");

	fseek(f_in, 0L, SEEK_END);
	size_t size = ftell(f_in);
	rewind(f_in);
	
	int_t n = size/sizeof(int_t);

        *SA = (int_t*) malloc(n*sizeof(int_t));
        fread(*SA, sizeof(int_t), n, f_in);

        fclose(f_in);
        free(c_in);

return n;
}

/***********************************************************************/

int_t file_text_read(unsigned char** str, char* c_file, const char* ext){

        FILE *f_in;
        char *c_in = (char*) malloc((strlen(c_file)+strlen(ext)+3)*sizeof(char));

        sprintf(c_in, "%s.%s", c_file, ext);
        f_in = fopen(c_in, "rb");
        if (!f_in) perror ("file_text_read");

	fseek(f_in, 0L, SEEK_END);
	size_t size = ftell(f_in);
	rewind(f_in);

	int_t n = size/sizeof(unsigned char);

        *str = (unsigned char*) malloc(n*sizeof(unsigned char));
        fread(*str, sizeof(unsigned char), n, f_in);

        fclose(f_in);
        free(c_in);

return n;
}

/***********************************************************************/

int file_chdir(char* dir){

        char* oldwd = getcwd(NULL,0);
        if (!oldwd) die(__func__);
        if (chdir(dir) == -1) die(__func__);

        free(oldwd);
return 0;
}

/***********************************************************************/
