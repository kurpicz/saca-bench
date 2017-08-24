#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gsacak.h"
#include "experiments/external/malloc_count/malloc_count.h" //memory counter

int main(int argc, char *argv[]){

	printf("sizeof(int_t) = %zu bytes\n", sizeof(int_t));
	unsigned char *Text;
	uint_t n=0;

	// intput data
	if(argc>=2){

		//concatenate all strings s_1, s_2, .., s_d in s_1$s_2$..%s_d$#
		int i = 2, sum=0;
		for(; i<= argc; i++){
			sum += strlen((argv[i-1]))+1;
		}
		n = sum+1;
		Text = malloc(n*sizeof(unsigned char));
		sum=0;
		for(i=2; i<= argc; i++){
			sscanf(argv[i-1], "%s", &Text[sum]);
			sum += strlen((argv[i-1]))+1;
			Text[sum-1]=1;//separator
		}
		Text[n-1]=0;
		printf("N = %d\n", n);
	}
	else{
		unsigned char *S[3] = {"banana", "anaba", "anan"};
		int i, sum=0;
		for(i=0; i< 3; i++){
			sum += strlen((S[i]))+1;
		}
		n = sum+1;
		Text = malloc(n*sizeof(unsigned char));
		sum=0;
		for(i=0; i< 3; i++){
			sscanf(S[i], "%s", &Text[sum]);
			sum += strlen((S[i]))+1;
			Text[sum-1]=1;//separator
		}
		Text[n-1]=0;
		printf("N = %d\n", n);
	}	

	int i, j;
	printf("Text = ");
	for(i=0;i<n-1;i++){
		if(Text[i]==1) printf("$");
		else printf("%c", Text[i]);
	}
	printf("#\n");

/**/
	// allocate
	uint_t *SA = (uint_t *)malloc(n * sizeof(uint_t));
	int_t *LCP = (int_t *)malloc(n * sizeof(int_t));
	int_t *DA = (int_t *)malloc(n * sizeof(int_t));
	
	// sort
	gsacak((unsigned char *)Text, (uint_t*)SA, LCP, DA, n);
	
	// output
	printf("i\tSA\tDA\tLCP\tBWT\tsuffixes\n");
	for(i = 0; i < n; ++i) {
	    char j = (SA[i])? Text[SA[i]-1]:'#';
	    if(j==1) j = '$';
	    printf("%d\t%d\t%d\t%d\t%c\t",i, SA[i], DA[i], LCP[i], j);
	    for(j = SA[i]; j < n; ++j) {
		if(Text[j]==1) printf("$");
		else printf("%c", Text[j]);
	    }
	    printf("#\n");
	}


	// deallocate
	free(SA);
	free(DA);
	free(LCP);
	free(Text);
	
return 0;
}

