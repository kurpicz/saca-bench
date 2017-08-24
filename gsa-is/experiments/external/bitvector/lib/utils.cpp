#include "utils.hpp"

/**********************************************************************/

void time_start(time_t *t_time, clock_t *c_clock){

	*t_time = time(NULL);
	*c_clock =  clock();
}

double time_stop(time_t t_time, clock_t c_clock){

	double aux1 = (clock() - c_clock) / (double)(CLOCKS_PER_SEC);
	double aux2 = difftime (time(NULL),t_time);
	
	printf("CLOCK = %lf TIME = %lf\n", aux1, aux2);
	
	return aux1;
}


/**********************************************************************/
void die(const char* where) {

	printf("Error at %s: %s.\n",where,errno ? strerror(errno) : "errno not set");
	exit(errno?errno:1);
}
 
void dies(const char* where, char* format, ...) {

	int_t err = errno;
	va_list val;

	printf("Error at %s: %s.\n",where,errno ? strerror(errno) : "errno not set");
	va_start(val,format);
	vprintf(format,val);
	va_end(val);
	printf("\n");

	exit(err ? err : 1);
}
/**********************************************************************/
int_t print_char(char* A, int_t n){

	int_t i;
	for(i=0; i<n; i++)	
		printf("%" PRIdN ") %c\n", i, A[i]);

	printf("\n");

return 0;
}
/**********************************************************************/
int_t print_int(int_t* A, int_t n){

	int_t i;
	for(i=0; i<n; i++)	
		printf("%" PRIdN ") %" PRIdN "\n", i, A[i]);

	printf("\n");

return 0;
}
/**********************************************************************/
int_t min_range(int_t* A, int_t l, int_t r){


	if(r>l)return 0;

	printf("[l, r] = [%" PRIdN ", %" PRIdN "]\n", l, r);

	int_t min = INT_MAX;
	int_t i;
	for(i=l; i<=l; i++)
		min = (A[i]<min?A[i]:min);

	printf("min = %" PRIdN "\n", min);

return min;
}
/*******************************************************************/
int_t* cat_int(unsigned char** R, int k, int_t *n){

	(*n)++; //add 0 at the end

	int_t i, j;
	int_t l=0;
	int_t *str_int = (int_t*) malloc((*n)*sizeof(int_t));

	for(i=0; i<k; i++){
		int_t m = strlen((char*)R[i]);
		for(j=0; j<m; j++){
			//removes symbols > 255
			if(R[i][j]+1<256) str_int[l++] = R[i][j]+(k+1);
			else (*n)--;
		}
//		for(j=0; j<m; j++)
//			str_int[l++] = R[i][j]+(k+1);
		str_int[l++] = i+1; //add $_i as separator
	}
	
	str_int[l++]=0;
        if(*n>l){
		str_int = (int_t*) realloc(str_int, (l)*sizeof(int_t));
		printf("N = %" PRIdN "\n", l);
	}
	*n = l;

return str_int;
}
/*******************************************************************/
unsigned char* cat_char(unsigned char** R, int k, int_t *n){

	(*n)++; //add 0 at the end

	int_t i, j;
	int_t l=0;
	unsigned char *str = (unsigned char*) malloc((*n)*sizeof(unsigned char));

	for(i=0; i<k; i++){
		int_t m = strlen((char*)R[i]);
		//removes empty strings
		if(m==0){
			(*n)--;
			continue;
		}
		for(j=0; j<m; j++){
			//removes symbols > 255
			if(R[i][j]+1<256 && R[i][j]+1>0) str[l++] = R[i][j]+1;
			else (*n)--;
		}
		str[l++] = 1; //add 1 as separator
	}

	str[l++]=0;
        if(*n>l){
		str = (unsigned char*) realloc(str, (l)*sizeof(unsigned char));
		printf("N = %" PRIdN "\n", l);
	}
	*n = l;

return str;
}

/**********************************************************************/
double log2(double i){
	return log(i)/log(2);
}
/**********************************************************************/
