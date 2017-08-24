/*
 * Induced suffix sorting, LCP and document arrays construction for string collections
 *
 * Authors: Felipe A. Louza, Simon Gog, Guilherme P. Telles
 * contact: louza@ic.unicamp.br
 * 
 * version 1.3
 * 07/06/2016
 *
 */

#include <stdio.h>
#include <errno.h>
#include <time.h>

#include "lib/file.h"
#include "lib/suffix_array.h"
#include "lib/lcp_array.h"
#include "lib/document_array.h"
// #include "external/malloc_count/malloc_count.h"
#include "../gsais.h"
#include "../gsacak.h"

#ifndef STDERR
	#define STDERR 0
#endif

#ifndef DEBUG
        #define DEBUG   0
#endif

/*******************************************************************/

int main(int argc, char** argv){

int VALIDATE=0, MODE=0, OUTPUT=0, LCP_COMPUTE=0, DA_COMPUTE=0, BWT_COMPUTE=0;
time_t t_start=0, t_total=0;
clock_t c_start=0, c_total=0;

	if(argc!=10){
		dies(__func__,"argc!=9");
	}

	unsigned char **R;
	int_t i, n=0;
	int   k;

	char* c_dir = argv[1];
	char* c_file = argv[2];

	sscanf(argv[3], "%d", &k);
	sscanf(argv[4], "%u", &MODE);
	sscanf(argv[5], "%u", &LCP_COMPUTE);
	sscanf(argv[6], "%u", &DA_COMPUTE);
	sscanf(argv[7], "%u", &BWT_COMPUTE);
	sscanf(argv[8], "%u", &VALIDATE);
	sscanf(argv[9], "%u", &OUTPUT);

	if(MODE==7 || MODE==8  || MODE==11 || MODE==12) LCP_COMPUTE=1;
	if(MODE==9 || MODE==10 || MODE==11 || MODE==12) DA_COMPUTE=1;
	if(MODE>12) return 1;

	file_chdir(c_dir);

	//disk access
	R = (unsigned char**) file_load_multiple(c_file, k, &n);
	if(!R){
		fprintf(stderr, "Error: less than %d strings in %s\n", k, c_file);
		return 1;
	}

	unsigned char *str = NULL;
	int_t  *str_int = NULL;
	
	//concatenate strings
	if(MODE==1 || MODE==2){//sais or saca-k (int)	
		str_int = cat_int(R, k, &n);
		
		#if DEBUG
		int_t i;
		for(i=0;i<min(10,n); i++)
			 printf("%" PRIdN ") %" PRIdN "\n", i, str_int[i]);
		printf("\n");
		#endif
	}
	else{ // sais, saca-k, gsais, gsaca-k (char)
		str = cat_char(R, k, &n);
		#if DEBUG
		int_t i;
		for(i=0;i<min(10,n); i++)
			 printf("%" PRIdN ") %d\n", i, str[i]);
		printf("\n");
		#endif
	}

	printf("K = %" PRId32 "\n", k);
	printf("N = %" PRIdN "\n", n+1);
	printf("sizeof(int) = %zu bytes\n", sizeof(int_t));

	#if DEBUG
		printf("R:\n");
		for(i=0; i<min(5,k); i++)
			printf("%" PRIdN ") %s (%zu)\n", i, R[i], strlen((char*)R[i]));
	#endif

	//free memory
	for(i=0; i<k; i++)
		free(R[i]);
	free(R);

	int_t *SA = (int_t*) malloc(n*sizeof(int_t));
	for(i=0; i<n; i++) SA[i]=0;
	int_t depth=0;

	int_t *LCP = NULL;	
	if(LCP_COMPUTE){
		LCP = (int_t*) malloc(n*sizeof(int_t));
		for(i=0; i<n; i++) LCP[i]=0;
	}

	int_t *DA = NULL;	
	if(DA_COMPUTE){
		DA = (int_t*) malloc(n*sizeof(int_t));
                for(i=0; i<n; i++) DA[i]=-1;
	}

	time_start(&t_total, &c_total);
	time_start(&t_start, &c_start);

	switch(MODE){
               case 1: 	printf("## SAIS (int) ##\n");
			#if STDERR
               	       		fprintf(stderr, "## SAIS (int) ##\n");
			#endif
                       	depth = sais_int((int_t*)str_int, (uint_t*)SA, n, 256+k);
                       	break;

               case 2: 	printf("## SACA_K (int) ##\n"); 
			#if STDERR
                       		fprintf(stderr, "## SACA_K (int) ##\n"); 
			#endif
                       	depth = sacak_int((int_t*)str_int, (uint_t*)SA, n, 256+k);
                       	break;
               
               case 3: 	printf("## SAIS (char) ##\n");
			#if STDERR
	               	       	fprintf(stderr, "## SAIS (char) ##\n");
			#endif
                       	depth = sais(str, (uint_t*)SA, n);
                       	break;

               case 4: 	printf("## SACA_K (char) ##\n"); 
			#if STDERR
                       		fprintf(stderr, "## SACA_K (char) ##\n"); 
			#endif
                       	depth = sacak(str, (uint_t*)SA, n);
                       	break;
       
               case 5: 	printf("## gSAIS ##\n"); 
			#if STDERR
                	       	fprintf(stderr, "## gSAIS ##\n"); 
			#endif
                       	depth = gsais((unsigned char*)str, (uint_t*)SA, NULL, NULL, n);
                       	break;

               case 6: 	printf("## gSACA_K ##\n"); 
			#if STDERR
        	      	       	fprintf(stderr, "## gSACA_K ##\n"); 
			#endif
                       	depth = gsacak((unsigned char*)str, (uint_t*)SA, NULL, NULL, n);
                       	break;

               case 7: 	printf("## gSAIS+LCP ##\n"); 
			#if STDERR
	                       	fprintf(stderr, "## gSAIS+LCP ##\n"); 
			#endif
                       	depth = gsais((unsigned char*)str, (uint_t*)SA, LCP, NULL, n);
                       	break;

               case 8: 	printf("## gSACA_K+LCP ##\n"); 
			#if STDERR
               			fprintf(stderr, "## gSACA_K+LCP ##\n"); 
			#endif
                       	depth = gsacak((unsigned char*)str, (uint_t*)SA, LCP, NULL, n);
                       	break;

               case 9: 	printf("## gSAIS+DA ##\n"); 
			#if STDERR
               			fprintf(stderr, "## gSAIS+DA ##\n"); 
			#endif
                       	depth = gsais((unsigned char*)str, (uint_t*)SA, NULL, DA, n);
                       	break;

               case 10: printf("## gSACA_K+DA ##\n"); 
			#if STDERR
        	     		fprintf(stderr, "## gSACA_K+DA ##\n"); 
			#endif
                       	depth = gsacak((unsigned char*)str, (uint_t*)SA, NULL, DA, n);
                       	break;
                       
               case 11: printf("## gSAIS+LCP+DA ##\n"); 
			#if STDERR
	               		fprintf(stderr, "## gSAIS+LCP+DA ##\n"); 
			#endif
                       	depth = gsais((unsigned char*)str, (uint_t*)SA, LCP, DA, n);
                       	break;
                       
               case 12: printf("## gSACA_K+LCP+DA ##\n"); 
			#if STDERR
                		fprintf(stderr, "## gSACA_K+LCP+DA ##\n"); 
			#endif
                        depth = gsacak((unsigned char*)str, (uint_t*)SA, LCP, DA, n);
                        break;

		default: break;
	}

	fprintf(stderr,"%.6lf\n", time_stop(t_start, c_start));

	//LCP array
	if(LCP_COMPUTE && (MODE!=7 && MODE!=8 && MODE!=11 && MODE!=12)){
		time_start(&t_start, &c_start);
		if(MODE==1 || MODE==2)
			lcp_PHI_int((int_t*)str_int, SA, LCP, n, sizeof(int_t));
		else
			lcp_PHI((unsigned char*)str, SA, LCP, n, sizeof(char), 1);//separator=1
		printf("PHI-algorithm (LCP array):\n");
		fprintf(stderr,"%.6lf\n", time_stop(t_start, c_start));
	}
	
	//Document array
	if(DA_COMPUTE && (MODE!=9 && MODE!=10 && MODE!=11 && MODE!=12)){
		time_start(&t_start, &c_start);
		if(MODE==1 || MODE==2)
			document_array_LF_int((int_t*)str_int, SA, DA, n, 256+k, sizeof(int_t), 1, k);
		else if(MODE==3 || MODE==4)
			document_array_LF((unsigned char*)str, SA, DA, n, 256, sizeof(char), 0, k);//separator=0,1,..,k-1
		else
			document_array_LF((unsigned char*)str, SA, DA, n, 256, sizeof(char), 1, k);//separator=1
		printf("LF-algorithm (document array):\n");
		fprintf(stderr,"%.6lf\n", time_stop(t_start, c_start));
	}


        printf("total:\n");
        fprintf(stderr,"%.6lf\n", time_stop(t_total, c_total));


	#if DEBUG
	if(MODE==1 || MODE==2)//sais or saca-k	
		if(LCP_COMPUTE)	lcp_array_print((unsigned char*)str_int, SA, LCP, min(20,n), sizeof(int_t));	
		else suffix_array_print((unsigned char*)str_int, SA, min(10,n), sizeof(int_t));	
	else
		if(LCP_COMPUTE)	lcp_array_print((unsigned char*)str, SA, LCP, min(20,n), sizeof(char));	
		else suffix_array_print((unsigned char*)str, SA, min(10,n), sizeof(char));
	#endif


	// validate	
	if(VALIDATE){

		fprintf(stderr, "## CHECK ##\n"); 
		if(MODE==1 || MODE==2){//sais or saca-k	
	        	if(!suffix_array_check((unsigned char*)str_int, SA, n, sizeof(int_t), 0)) fprintf(stderr,"isNotSorted!!\n");
		        else printf("isSorted!!\ndepth = %" PRIdN "\n", depth);
		}
		else if(MODE==3 || MODE==4){
	        	if(!suffix_array_check((unsigned char*)str, SA, n, sizeof(char), 0)) fprintf(stderr,"isNotSorted!!\n");//compares until the sentinel=0
		        else printf("isSorted!!\ndepth = %" PRIdN "\n", depth);
		}
		else if(MODE>=5){
	        	if(!suffix_array_check((unsigned char*)str, SA, n, sizeof(char), 1)) fprintf(stderr,"isNotSorted!!\n");//compares until the separator=1
		        else printf("isSorted!!\ndepth = %" PRIdN "\n", depth);
		}

		if(LCP_COMPUTE){
			if(MODE==1 || MODE==2)//sais or saca-k	
	                	if(!lcp_array_check_lcp((unsigned char*)str_int, SA, LCP, n, sizeof(int_t), 1)) fprintf(stderr,"isNotLCP!!\n");
	        	        else printf("isLCP!!\n");
			else
	                	if(!lcp_array_check_lcp((unsigned char*)str, SA, LCP, n, sizeof(char), 1)) fprintf(stderr,"isNotLCP!!\n");
	        	        else printf("isLCP!!\n");
		}

		if(DA_COMPUTE){
			if(MODE==1 || MODE==2)//sais or saca-k	
			        if(!document_array_check_int(str_int, SA, DA, n, sizeof(int_t), k)) fprintf(stderr, "isNotDA!!\n");
			        else printf("isDA!!\n");
			else
			        if(!document_array_check(str, SA, DA, n, sizeof(char), 1, k)) fprintf(stderr, "isNotDA!!\n");
			        else printf("isDA!!\n");
		}
	}
	else printf("depth = %" PRIdN "\n", depth);

	// output
	if(OUTPUT){
		if(MODE==1||MODE==2) file_text_int_write(str_int, n, c_file, "str_int");
		else file_text_write(str, n, c_file, "str");

		if(BWT_COMPUTE){ 
			if(MODE==1||MODE==2) file_bwt_int_write(str_int, SA, n, c_file, "bwt");
			else file_bwt_write(str, SA, n, c_file, "bwt");
		}

		suffix_array_write(SA, n, c_file, "sa");
		if(LCP_COMPUTE) lcp_array_write(LCP, n, c_file, "lcp");
		if(DA_COMPUTE) document_array_write(DA, n, c_file, "da");
	}

	free(SA);
	if(LCP_COMPUTE) free(LCP);
	if(DA_COMPUTE) free(DA);
	if(MODE==1 || MODE==2) free(str_int);
	else free(str);

return 0;
}
