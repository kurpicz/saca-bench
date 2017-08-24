#include "gsais.h"

#define chr(i) (cs==sizeof(int_t)?((int_t*)s)[i]:((unsigned char *)s)[i])


#define tget(i) ( (t[(i)/8]&mask[(i)%8]) ? 1 : 0 )
#define tset(i, b) t[(i)/8]=(b) ? (mask[(i)%8]|t[(i)/8]) : ((~mask[(i)%8])&t[(i)/8])

#define isLMS(i) (i>0 && tget(i) && !tget(i-1))


#define false 0 
#define true 1  

#define DEBUG 0
#define DEPTH 0	 // compute time and size of reduced problem for each recursion call
#define PHASES 0 // compute time for each phase

#if m64
	const int_t EMPTY=0xffffffffffffffff; 
#else
	const int EMPTY=0xffffffff; 
#endif

unsigned char mask[]={0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};  

#define RMQ   2  //variants = (1, trivial) (2, using Gog's stack)

#define STACK_SIZE 895 //to use 10Kb of working space

typedef struct _pair{
  uint_t idx;
  int_t lcp;
} t_pair_sais;

int compare (const void * a, const void * b){
  if(*(const uint_t *)a < *(const uint_t *)b) return -1;
  if(*(const uint_t *)a > *(const uint_t *)b) return 1;
return 0;
}

void stack_push_sais(t_pair_sais* STACK, int_t *top, uint_t idx, int_t lcp){

  STACK[*top].idx=idx;
  STACK[*top].lcp=lcp;

  (*top)++;
}

void compute_lcp_phi_sparse_sais(int_t *s, int_t *SA1, 
  int_t *RA, int_t *LCP, int_t *PLCP,
  uint_t n1, int cs, unsigned char separator) {

  uint_t i;

  PLCP[SA1[0]]=0;//PLCP* (lms) is stored in PLCP array
  for(i=1; i<n1; i++)
    PLCP[SA1[i]] = LCP[i]; 

  LCP[SA1[0]]=0;//PHI is stored in LCP array
  for(i=1; i<n1; i++)
    LCP[SA1[i]] = SA1[i-1]; //RA[SA1[i-1]];

  int_t l=0; //q=0;
  for(i=0; i<n1-1;i++){
    if(chr(RA[i])==separator) continue;

    l = max(PLCP[i], l);//consider the LCP-value of the lms-substrings

    while(chr(RA[i]+l)==chr(RA[LCP[i]]+l) && !(chr(RA[i]+l) == separator && chr(RA[LCP[i]]+l)==separator) ) ++l;
    PLCP[i]=l;

    if(LCP[i]==n1-1) l -= RA[i+1]-RA[i];
    else l -= max(RA[i+1]-RA[i], RA[LCP[i]+1]-RA[LCP[i]]);//LCP[i] stores the distance of i-th suffix to its successor
  }

  LCP[0]=0;
  for(i=1; i<n1;i++) LCP[i]=PLCP[SA1[i]];

}

// compute the head or end of each bucket
void getBuckets(int_t *s, int_t *bkt, int_t n, int_t K, int cs, int end) { 
  int_t i, sum=0;
  for(i=0; i<K; i++) bkt[i]=0; // clear all buckets
  for(i=0; i<n; i++) bkt[chr(i)]++; // compute the size of each bucket
  for(i=0; i<K; i++) { sum+=bkt[i]; bkt[i]=end ? sum-1 : sum-bkt[i]; }
}

// compute SAl
void induceSAl(unsigned char *t, int_t *SA, int_t *s, int_t *bkt, 
               int_t n, int_t K, int cs, int level) { 
  int_t i, j;
  getBuckets(s, bkt, n, K, cs, false); // find heads of buckets
//  if(level==0) bkt[0]++; 
  for(i=0; i<n; i++)
    if(SA[i]!=EMPTY) {
	  j=SA[i]-1; 
	  if(j>=0 && !tget(j)) SA[bkt[chr(j)]++]=j;
    }
}

// compute SAs
void induceSAs(unsigned char *t, int_t *SA, int_t *s, int_t *bkt, 
               int_t n, int_t K, int cs, int level) { 
  int_t i, j;
  getBuckets(s, bkt, n, K, cs, true); // find ends of buckets
  for(i=n-1; i>=0; i--)
    if(SA[i]!=EMPTY) {
	  j=SA[i]-1; 
	  if(j>=0 && tget(j)) SA[bkt[chr(j)]--]=j;
    }
}

/*****************************************************************************/
// compute SAl
void induceSAl_generalized(unsigned char *t, int_t *SA, uint_t *s, int_t *bkt, 
               int_t n, int_t K, int cs, int level, int separator) { 
  int_t i, j;
  getBuckets((int_t*)s, bkt, n, K, cs, false); // find heads of buckets
//  if(level==0) bkt[0]++; 
  for(i=0; i<n; i++)
    if(SA[i]!=EMPTY) {
	  j=SA[i]-1; 
	  if(j>=0 && !tget(j) && chr(j)!=separator) SA[bkt[chr(j)]++]=j;
    }
}

// compute SAs
void induceSAs_generalized(unsigned char *t, int_t *SA, uint_t *s, int_t *bkt, 
               int_t n, int_t K, int cs, int level, int separator) { 
  int_t i, j;
  getBuckets((int_t*)s, bkt, n, K, cs, true); // find ends of buckets
  for(i=n-1; i>=0; i--)
    if(SA[i]!=EMPTY) {
	  j=SA[i]-1; 
	  if(j>=0 && tget(j) && chr(j)!=separator) SA[bkt[chr(j)]--]=j;
    }
}

/*****************************************************************************/
// compute SAl
void induceSAl_generalized_LCP(unsigned char *t, int_t *SA, int_t *LCP, uint_t *s, int_t *bkt, 
               int_t n, int_t K, int cs, int level, int separator) { 
  int_t i, j;
  
  for(i=0;i<K;i++)
    if(bkt[i]+1<n) if(SA[bkt[i]+1]!=U_MAX) LCP[bkt[i]+1]=I_MIN;  
  
  getBuckets((int_t*)s, bkt, n, K, cs, false); // find heads of buckets
  
  for(i=0;i<K;i++)
    if(bkt[i]<n) LCP[bkt[i]]=-2; 

  #if RMQ == 1 
  int_t *M=(int_t *)malloc(sizeof(int_t)*K);
  for(i=0;i<K;i++){
    M[i]=I_MAX;
  }
  #elif RMQ == 2
  uint_t* last_occ = (uint_t*) malloc(K*sizeof(uint_t));
  uint_t* tmp = (uint_t*) malloc(K*sizeof(uint_t));

  t_pair_sais* STACK = (t_pair_sais*) malloc((STACK_SIZE+1)*sizeof(t_pair_sais));
  int_t top = 0;
  //init
  stack_push_sais(STACK, &top, 0, -1);
  for(i=0;i<K;i++) last_occ[i]=0;
  #endif 
  
  bkt[0]++; // skip the virtual sentinel. 
  for(i=0; i<n; i++){
    if(SA[i]!=U_MAX){ //EMPTY) {
		
	  if(LCP[i]==I_MIN){ //is a L/S-seam position
  	  int_t l=0;
	  if(SA[bkt[chr(SA[i])]-1]<n-1)	
   	    while(chr(SA[i]+l)==chr(SA[bkt[chr(SA[i])]-1]+l))++l;
  	  LCP[i]=l;
      }
      #if RMQ == 1
        uint_t k;
        for(k=0; k<K; k++) if(M[k]>LCP[i]) M[k] = max(0,LCP[i]);
      #elif RMQ == 2
        int_t min_lcp=0;
        uint_t last;

        if(!SA[i]) last = 0;
        else{
          last = last_occ[chr(SA[i]-1)];
          last_occ[chr(SA[i]-1)] = i+1;
        }
 
        int_t lcp=max(0,LCP[i]);
        while(STACK[(top)-1].lcp>=lcp) (top)--;	

        stack_push_sais(STACK, &top, i+1, lcp);
        j = top-1;

        while(STACK[j].idx>last) j--;
        min_lcp=STACK[(j+1)].lcp;
      #endif
	  
      if(SA[i]>0) {
	  j=SA[i]-1; 
	  if(j>=0 && !tget(j) && chr(j)!=separator){
		   SA[bkt[chr(j)]]=j;
           #if RMQ == 1
     	      LCP[bkt[chr(j)]]+=M[chr(j)]+1;
  	          M[chr(j)] = I_MAX;
            #elif RMQ == 2
  	          LCP[bkt[chr(j)]]+=min_lcp+1; 
            #endif
            bkt[chr(j)]++;
	   }
      	if(bkt[chr(SA[i])]-1<i){ //if is LMS-type
	  if(chr(SA[i])!=separator)
	  SA[i]=U_MAX;
        }

      }
      #if RMQ == 2
      if(top>=STACK_SIZE){//if stack is full

        int_t j;
        memcpy(tmp, last_occ, K*sizeof(uint_t));
        qsort(tmp, K, sizeof(uint_t), compare);
       
        int_t curr=1, end=1;
        STACK[top].idx=U_MAX;
        for(j=0;j<K; j++){

          if(STACK[end-1].idx < tmp[j]+1){

            while(STACK[curr].idx<tmp[j]+1) curr++;

            if(curr<top) {
              STACK[end].idx=STACK[curr].idx;
              STACK[end].lcp=STACK[curr].lcp;
              end++; 
	      curr++;
            }
	  }
        }
 
        if(end>=STACK_SIZE){
          fprintf(stderr,"ERROR: induceSAl0_LCP\n");
          exit(1);
        }
        top = end;
      }
      #endif	   
    }
  }  
  #if RMQ == 1
  free(M);
  #elif RMQ == 2
  free(STACK);
  free(last_occ);
  free(tmp);
  #endif
}

// compute SAs
void induceSAs_generalized_LCP(unsigned char *t, int_t *SA, int_t *LCP, uint_t *s, int_t *bkt, 
               int_t n, int_t K, int cs, int level, int separator) { 
  int_t i, j;
 
  getBuckets((int_t*)s, bkt, n, K, cs, true); // find ends of buckets
  
  #if RMQ == 1
  int_t *M=(int_t *)malloc(sizeof(int_t)*K);
  for(i=0;i<K;i++) M[i]=I_MAX;
  #elif RMQ == 2 
  uint_t* last_occ = (uint_t*) malloc(K*sizeof(uint_t));
  uint_t* tmp = (uint_t*) malloc(K*sizeof(uint_t));

  t_pair_sais* STACK = (t_pair_sais*) malloc((STACK_SIZE+1)*sizeof(t_pair_sais));
  int_t top = 0;
  //init
  stack_push_sais(STACK, &top, n, -1);
  for(i=0;i<K;i++) last_occ[i]=n-1;
  #endif  

  for(i=n-1; i>0; i--){
    if(SA[i]>0){//!=EMPTY) {
	 j=SA[i]-1; 
	 if(j>=0 && tget(j) && chr(j)!=separator){
	   SA[bkt[chr(j)]]=j;
           #if RMQ == 1
  	      if(LCP[bkt[chr(j)]+1]>=0) 
  	        LCP[bkt[chr(j)]+1]=M[chr(j)]+1;

  	      if(LCP[bkt[chr(j)]]>0) 
  	        LCP[bkt[chr(j)]]=I_MAX;

           #elif RMQ == 2
             int_t min = I_MAX, end = top-1; 
  
  	     int_t last=last_occ[chr(j)];
             //search (can be binary)
             while(STACK[end].idx<=last) end--;
  
             min=STACK[(end+1)].lcp;
             last_occ[chr(j)] = i;
  
  	     if(LCP[bkt[chr(j)]+1]>=0) 
               LCP[bkt[chr(j)]+1]=min+1;
           #endif
  
           #if RMQ == 1
  	   M[chr(j)] = I_MAX;
           #endif
  
           bkt[chr(j)]--;
 
  	   if(SA[bkt[chr(j)]]!=U_MAX) {//L/S-seam
             int_t l=0;	
             while(chr(SA[bkt[chr(j)]+1]+l)==chr(SA[bkt[chr(j)]]+l))++l;
             LCP[bkt[chr(j)]+1]=l;
  	 }		     
      }
    }
    if(LCP[i]<0) LCP[i]=0;

    #if RMQ == 1
      int_t k;
      for(k=0; k<K; k++) if(M[k]>LCP[i]) M[k] = LCP[i];
    #elif RMQ == 2

      int_t lcp=max(0,LCP[i]);

      while(STACK[(top)-1].lcp>=lcp) (top)--;
      stack_push_sais(STACK, &top, i, lcp);

      if(top>=STACK_SIZE){

          int_t j;
          memcpy(tmp, last_occ, K*sizeof(uint_t));
          qsort(tmp, K, sizeof(uint_t), compare);

          int_t curr=0, end=1;
          STACK[top].idx=U_MAX;

          for(j=K-1;j>=0; j--){

            if(STACK[end-1].idx > tmp[j]){

              while(STACK[curr].idx>tmp[j]) curr++;

              STACK[end].idx=STACK[curr].idx;
              STACK[end].lcp=STACK[curr].lcp;
              end++; 
            }
          } 

          if(end>=STACK_SIZE){
            fprintf(stderr,"ERROR: induceSAl0_LCP\n");
            exit(1);
          }
          top = end;
      }
      #endif	  
  }
  LCP[0]=0;

  //variant 1
  #if RMQ == 1
  free(M);
  #elif RMQ == 2
  free(STACK);
  free(last_occ);
  free(tmp);
  #endif
}
/*****************************************************************************/
// compute SAl
void induceSAl_generalized_DA(unsigned char *t, int_t *SA, int_t *DA, uint_t *s, int_t *bkt, 
               int_t n, int_t K, int cs, int level, int separator) { 
  int_t i, j;
  getBuckets((int_t*)s, bkt, n, K, cs, false); // find heads of buckets
//  if(level==0) bkt[0]++; 
  for(i=0; i<n; i++)
    if(SA[i]!=EMPTY) {
	  j=SA[i]-1; 
	  if(j>=0 && !tget(j) && chr(j)!=separator){
		SA[bkt[chr(j)]]=j;
		DA[bkt[chr(j)]++]=DA[i];
	  }
    }
}

// compute SAs
void induceSAs_generalized_DA(unsigned char *t, int_t *SA, int_t *DA, uint_t *s, int_t *bkt, 
               int_t n, int_t K, int cs, int level, int separator) { 
  int_t i, j;
  getBuckets((int_t*)s, bkt, n, K, cs, true); // find ends of buckets
  for(i=n-1; i>=0; i--)
    if(SA[i]!=EMPTY) {
	  j=SA[i]-1; 
	  if(j>=0 && tget(j) && chr(j)!=separator){
		SA[bkt[chr(j)]]=j;
		DA[bkt[chr(j)]--]=DA[i];
	  }
    }
}

/*****************************************************************************/

// compute SAl
void induceSAl_generalized_LCP_DA(unsigned char *t, int_t *SA, int_t *LCP, int_t *DA, uint_t *s, int_t *bkt, 
               int_t n, int_t K, int cs, int level, int separator) { 
  int_t i, j;
  
  for(i=0;i<K;i++)
    if(bkt[i]+1<n) if(SA[bkt[i]+1]!=U_MAX) LCP[bkt[i]+1]=I_MIN;  
  
  getBuckets((int_t*)s, bkt, n, K, cs, false); // find heads of buckets
  
  for(i=0;i<K;i++)
    if(bkt[i]<n) LCP[bkt[i]]=-2; 

  #if RMQ == 1 
  int_t *M=(int_t *)malloc(sizeof(int_t)*K);
  for(i=0;i<K;i++){
    M[i]=I_MAX;
  }
  #elif RMQ == 2
  uint_t* last_occ = (uint_t*) malloc(K*sizeof(uint_t));
  uint_t* tmp = (uint_t*) malloc(K*sizeof(uint_t));

  t_pair_sais* STACK = (t_pair_sais*) malloc((STACK_SIZE+1)*sizeof(t_pair_sais));
  int_t top = 0;
  //init
  stack_push_sais(STACK, &top, 0, -1);
  for(i=0;i<K;i++) last_occ[i]=0;
  #endif 
  
  bkt[0]++; // skip the virtual sentinel. 
  for(i=0; i<n; i++){
    if(SA[i]!=U_MAX){ //EMPTY) {
		
	  if(LCP[i]==I_MIN){ //is a L/S-seam position
  	  int_t l=0;
	  if(SA[bkt[chr(SA[i])]-1]<n-1)	
   	    while(chr(SA[i]+l)==chr(SA[bkt[chr(SA[i])]-1]+l))++l;
  	  LCP[i]=l;
      }
      #if RMQ == 1
        uint_t k;
        for(k=0; k<K; k++) if(M[k]>LCP[i]) M[k] = max(0,LCP[i]);
      #elif RMQ == 2
        int_t min_lcp=0;
        uint_t last;

        if(!SA[i]) last = 0;
        else{
          last = last_occ[chr(SA[i]-1)];
          last_occ[chr(SA[i]-1)] = i+1;
        }
 
        int_t lcp=max(0,LCP[i]);
        while(STACK[(top)-1].lcp>=lcp) (top)--;	

        stack_push_sais(STACK, &top, i+1, lcp);
        j = top-1;

        while(STACK[j].idx>last) j--;
        min_lcp=STACK[(j+1)].lcp;
      #endif
	  
      if(SA[i]>0) {
	  j=SA[i]-1; 
	  if(j>=0 && !tget(j) && chr(j)!=separator){
		   SA[bkt[chr(j)]]=j;
		   DA[bkt[chr(j)]]=DA[i];
           #if RMQ == 1
     	      LCP[bkt[chr(j)]]+=M[chr(j)]+1;
  	          M[chr(j)] = I_MAX;
            #elif RMQ == 2
  	          LCP[bkt[chr(j)]]+=min_lcp+1; 
            #endif
            bkt[chr(j)]++;
	   }
      	if(bkt[chr(SA[i])]-1<i){ //if is LMS-type
	  if(chr(SA[i])!=separator)
	  SA[i]=U_MAX;
        }

      }
      #if RMQ == 2
      if(top>=STACK_SIZE){//if stack is full

        int_t j;
        memcpy(tmp, last_occ, K*sizeof(uint_t));
        qsort(tmp, K, sizeof(uint_t), compare);
       
        int_t curr=1, end=1;
        STACK[top].idx=U_MAX;
        for(j=0;j<K; j++){

          if(STACK[end-1].idx < tmp[j]+1){

            while(STACK[curr].idx<tmp[j]+1) curr++;

            if(curr<top) {
              STACK[end].idx=STACK[curr].idx;
              STACK[end].lcp=STACK[curr].lcp;
              end++; 
	      curr++;
            }
	  }
        }
 
        if(end>=STACK_SIZE){
          fprintf(stderr,"ERROR: induceSAl0_LCP\n");
          exit(1);
        }
        top = end;
      }
      #endif	   
    }
  }  
  #if RMQ == 1
  free(M);
  #elif RMQ == 2
  free(STACK);
  free(last_occ);
  free(tmp);
  #endif
}

// compute SAs
void induceSAs_generalized_LCP_DA(unsigned char *t, int_t *SA, int_t *LCP, int_t *DA, uint_t *s, int_t *bkt, 
               int_t n, int_t K, int cs, int level, int separator) { 
  int_t i, j;
 
  getBuckets((int_t*)s, bkt, n, K, cs, true); // find ends of buckets
  
  #if RMQ == 1
  int_t *M=(int_t *)malloc(sizeof(int_t)*K);
  for(i=0;i<K;i++) M[i]=I_MAX;
  #elif RMQ == 2 
  uint_t* last_occ = (uint_t*) malloc(K*sizeof(uint_t));
  uint_t* tmp = (uint_t*) malloc(K*sizeof(uint_t));

  t_pair_sais* STACK = (t_pair_sais*) malloc((STACK_SIZE+1)*sizeof(t_pair_sais));
  int_t top = 0;
  //init
  stack_push_sais(STACK, &top, n, -1);
  for(i=0;i<K;i++) last_occ[i]=n-1;
  #endif  

  for(i=n-1; i>0; i--){
    if(SA[i]>0){//!=EMPTY) {
	 j=SA[i]-1; 
	 if(j>=0 && tget(j) && chr(j)!=separator){
	   SA[bkt[chr(j)]]=j;
	   DA[bkt[chr(j)]]=DA[i];
           #if RMQ == 1
  	      if(LCP[bkt[chr(j)]+1]>=0) 
  	        LCP[bkt[chr(j)]+1]=M[chr(j)]+1;

  	      if(LCP[bkt[chr(j)]]>0) 
  	        LCP[bkt[chr(j)]]=I_MAX;

           #elif RMQ == 2
             int_t min = I_MAX, end = top-1; 
  
  	     int_t last=last_occ[chr(j)];
             //search (can be binary)
             while(STACK[end].idx<=last) end--;
  
             min=STACK[(end+1)].lcp;
             last_occ[chr(j)] = i;
  
  	     if(LCP[bkt[chr(j)]+1]>=0) 
               LCP[bkt[chr(j)]+1]=min+1;
           #endif
  
           #if RMQ == 1
  	   M[chr(j)] = I_MAX;
           #endif
  
           bkt[chr(j)]--;
 
  	   if(SA[bkt[chr(j)]]!=U_MAX) {//L/S-seam
             int_t l=0;	
             while(chr(SA[bkt[chr(j)]+1]+l)==chr(SA[bkt[chr(j)]]+l))++l;
             LCP[bkt[chr(j)]+1]=l;
  	 }		     
      }
    }
    if(LCP[i]<0) LCP[i]=0;

    #if RMQ == 1
      int_t k;
      for(k=0; k<K; k++) if(M[k]>LCP[i]) M[k] = LCP[i];
    #elif RMQ == 2

      int_t lcp=max(0,LCP[i]);

      while(STACK[(top)-1].lcp>=lcp) (top)--;
      stack_push_sais(STACK, &top, i, lcp);

      if(top>=STACK_SIZE){

          int_t j;
          memcpy(tmp, last_occ, K*sizeof(uint_t));
          qsort(tmp, K, sizeof(uint_t), compare);

          int_t curr=0, end=1;
          STACK[top].idx=U_MAX;

          for(j=K-1;j>=0; j--){

            if(STACK[end-1].idx > tmp[j]){

              while(STACK[curr].idx>tmp[j]) curr++;

              STACK[end].idx=STACK[curr].idx;
              STACK[end].lcp=STACK[curr].lcp;
              end++; 
            }
          } 

          if(end>=STACK_SIZE){
            fprintf(stderr,"ERROR: induceSAl0_LCP\n");
            exit(1);
          }
          top = end;
      }
      #endif	  
  }
  LCP[0]=0;

  //variant 1
  #if RMQ == 1
  free(M);
  #elif RMQ == 2
  free(STACK);
  free(last_occ);
  free(tmp);
  #endif
}

/*****************************************************************************/

int_t SAIS(int_t *s, int_t *SA, int_t n, int_t K, int cs, int level) {
  int_t i, j;

  #if PHASES
  time_t t_start_phase = 0.0;
  clock_t c_start_phase = 0.0;
  #endif

  #if DEPTH
  time_t t_start = time(NULL);
  clock_t c_start =  clock();
  #endif

  #if PHASES
  if(!level){
	t_start_phase = time(NULL);
	c_start_phase =  clock();
  }
  #endif

  unsigned char *t=(unsigned char *)malloc(n/8+1); // LS-type array in bits

  // stage 1: reduce the problem by at least 1/2

  // Classify the type of each character
  tset(n-2, 0); tset(n-1, 1); // the sentinel must be in s1, important!!!
  for(i=n-3; i>=0; i--) tset(i, (chr(i)<chr(i+1) || (chr(i)==chr(i+1) && tget(i+1)==1))?1:0);

  int_t *bkt = (int_t *)malloc(sizeof(int_t)*K); // bucket counters

  // sort all the S-substrings
  getBuckets(s, bkt, n, K, cs, true); // find ends of buckets
  for(i=0; i<n; i++) SA[i]=EMPTY;
  for(i=n-2; i>=0; i--)
    if(isLMS(i)) SA[bkt[chr(i)]--]=i;
  SA[0]=n-1; // set the single sentinel LMS-substring

  induceSAl(t, SA, s, bkt, n, K, cs, level); 
  induceSAs(t, SA, s, bkt, n, K, cs, level); 
  free(bkt);

  // compact all the sorted substrings into the first n1 items of s
  // 2*n1 must be not larger than n (proveable)
  int_t n1=0;
  for(i=0; i<n; i++)
    if(isLMS(SA[i]))
      SA[n1++]=SA[i];

  // Init the name array buffer
  for(i=n1; i<n; i++) SA[i]=EMPTY;

  // find the lexicographic names of all substrings
  int_t name=0;
  int_t prev=-1;
  for(i=0; i<n1; i++) {
    int_t pos=SA[i]; int diff=false;
    int_t d;
    for(d=0; d<n; d++)
      if(prev==-1 || pos+d==n-1 || prev+d==n-1 ||
         chr(pos+d)!=chr(prev+d) ||
         tget(pos+d)!=tget(prev+d))
      { diff=true; break; }
      else
        if(d>0 && (isLMS(pos+d) || isLMS(prev+d)))
          break;

    if(diff) 
      { name++; prev=pos; }
	  pos=(pos%2==0)?pos/2:(pos-1)/2;
    SA[n1+pos]=name-1; 
  }
  for(i=n-1, j=n-1; i>=n1; i--)
	  if(SA[i]!=EMPTY) SA[j--]=SA[i];

  #if PHASES
  if(!level){
	printf("phase 1:\n");
	time_stop(t_start_phase, c_start_phase);
  }
  #endif

  #if PHASES
  if(!level){
	t_start_phase = time(NULL);
	c_start_phase =  clock();
  }
  #endif
 
  // s1 is done now
  int_t *SA1=SA, *s1=SA+n-n1;

  // stage 2: solve the reduced problem
  int_t depth=1;
  // recurse if names are not yet unique
  if(name<n1) {
    depth += SAIS((int_t*)s1, SA1, n1, name, sizeof(int_t), level+1);
  } else{ // generate the suffix array of s1 directly
    for(i=0; i<n1; i++) SA1[s1[i]] = i;
  }

  // stage 3: induce the result for the original problem
  bkt = (int_t *)malloc(sizeof(int_t)*K); // bucket counters

  // put all left-most S characters into their buckets
  getBuckets(s, bkt, n, K, cs, true); // find ends of buckets
  j=0;
  for(i=1; i<n; i++)
    if(isLMS(i)) s1[j++]=i; // get p1

  for(i=0; i<n1; i++) SA1[i]=s1[SA1[i]]; // get index in s1
  for(i=n1; i<n; i++) SA[i]=EMPTY; // init SA[n1..n-1]
  for(i=n1-1; i>=0; i--) {
    j=SA[i]; SA[i]=EMPTY;
    if(level==0 && i==0)
      SA[0]=n-1;
    else
      SA[bkt[chr(j)]--]=j;
  }
  
  #if PHASES
  if(!level){
	printf("phase 2:\n");
	time_stop(t_start_phase, c_start_phase);
  }
  #endif

  #if PHASES
  if(!level){
	t_start_phase = time(NULL);
	c_start_phase =  clock();
  }
  #endif

  induceSAl(t, SA, s, bkt, n, K, cs, level); 

  #if PHASES
  if(!level){
	printf("phase 3:\n");
	time_stop(t_start_phase, c_start_phase);
  }
  #endif

  #if PHASES
  if(!level){
	t_start_phase = time(NULL);
	c_start_phase =  clock();
  }
  #endif

  induceSAs(t, SA, s, bkt, n, K, cs, level); 

  free(bkt); 
  free(t);

  #if PHASES
  if(!level){
	printf("phase 4:\n");
	time_stop(t_start_phase, c_start_phase);
  }
  #endif

  #if DEPTH
  printf("depth %" PRIdN ":\n", depth);
  time_stop(t_start, c_start);
  #endif

return depth;
}

/*****************************************************************************/

int_t gSAIS(uint_t *s, int_t *SA, int_t n, int_t K, int cs, unsigned char separator, int level) {
  int_t i, j;
  
  #if PHASES
  time_t t_start_phase = 0.0;
  clock_t c_start_phase = 0.0;
  #endif

  #if DEPTH
  time_t t_start = time(NULL);
  clock_t c_start =  clock();
  #endif

  #if PHASES
	t_start_phase = time(NULL);
	c_start_phase =  clock();
  #endif

  unsigned char *t=(unsigned char *)malloc(n/8+1); // LS-type array in bits

  // stage 1: reduce the problem by at least 1/2

  // Classify the type of each character
  tset(n-2, 0); tset(n-1, 1); // the sentinel must be in s1, important!!!
  for(i=n-3; i>=0; i--) tset(i, (chr(i)<chr(i+1) || (chr(i)==chr(i+1) && tget(i+1)==1))?1:0);

  int_t *bkt = (int_t *)malloc(sizeof(int_t)*K); // bucket counters

  // sort all the S-substrings
  getBuckets((int_t*)s, bkt, n, K, cs, true); // find ends of buckets
  for(i=0; i<n; i++) SA[i]=EMPTY;

  // gsa-is
  int_t tmp=bkt[separator]--;// shift one position left of bkt[separator]

  SA[0]=n-1; // set the single sentinel LMS-substring

  SA[tmp]=SA[0]-1;// insert the last separator at the end of bkt[separator]

  int_t p=n-2;

  for(i=n-2; i>=0; i--){
    if(isLMS(i)){

      if(chr(i)==separator) // gsa-is
        SA[++bkt[chr(p)]]=EMPTY; // removes LMS-positions that induces separator suffixes

      SA[bkt[chr(i)]--]=i;
      p=i;
    }
  }

  induceSAl_generalized(t, SA, s, bkt, n, K, cs, level, separator); 
  induceSAs_generalized(t, SA, s, bkt, n, K, cs, level, separator); 

  // insert separator suffixes in their buckets
  bkt[separator]=1; // gsa-is
  for(i=0; i<n-1; i++) 
    if(chr(i)==separator)
      SA[bkt[chr(i)]++]=i;
  
  free(bkt);

  // compact all the sorted substrings into the first n1 items of s
  // 2*n1 must be not larger than n (proveable)
  int_t n1=0;
  for(i=0; i<n; i++)
    if(isLMS(SA[i]))
      SA[n1++]=SA[i];


  // Init the name array buffer
  for(i=n1; i<n; i++) SA[i]=EMPTY;

  // find the lexicographic names of all substrings
  int_t name=0, prev=-1;
  for(i=0; i<n1; i++) {

    int_t pos=SA[i]; int diff=false;
    int_t d;
    for(d=0; d<n; d++)
      if(prev==-1 || pos+d==n-1 || prev+d==n-1 ||
         chr(pos+d)!=chr(prev+d) ||
         (chr(pos+d)==separator && chr(prev+d)==separator) || // $_i < $_j iff i < j
         tget(pos+d)!=tget(prev+d) // gsa-is
      )
      { 
	diff=true; break;
      }
      else
        if(d>0 && (isLMS(pos+d) || isLMS(prev+d)))
          break;

    if(diff){
      name++; 
      prev=pos; 
    }

    pos=(pos%2==0)?pos/2:(pos-1)/2;
    SA[n1+pos]=name-1; 
  }

  for(i=n-1, j=n-1; i>=n1; i--)
	  if(SA[i]!=EMPTY) SA[j--]=SA[i];
  
  #if PHASES
	printf("phase 1:\n");
	time_stop(t_start_phase, c_start_phase);
  #endif

  #if PHASES
	t_start_phase = time(NULL);
	c_start_phase =  clock();
  #endif

  // s1 is done now
  int_t *SA1=SA, *s1=SA+n-n1;

  // stage 2: solve the reduced problem

  int_t depth=1;
  // recurse if names are not yet unique
  if(name<n1) {
    depth += SAIS((int_t*)s1, SA1, n1, name, sizeof(int_t), level+1);
  } else{ // generate the suffix array of s1 directly
    for(i=0; i<n1; i++) SA1[s1[i]] = i;
  }
  
  #if PHASES
	printf("phase 2:\n");
	time_stop(t_start_phase, c_start_phase);
  #endif

  #if PHASES
	t_start_phase = time(NULL);
	c_start_phase =  clock();
  #endif

  // stage 3: induce the result for the original problem

  bkt = (int_t *)malloc(sizeof(int_t)*K); // bucket counters

  // put all left-most S characters into their buckets
  getBuckets((int_t*)s, bkt, n, K, cs, true); // find ends of buckets
  j=0;
  for(i=1; i<n; i++)
    if(isLMS(i)) s1[j++]=i; // get p1

  for(i=0; i<n1; i++) SA1[i]=s1[SA1[i]]; // get index in s1
  for(i=n1; i<n; i++) SA[i]=EMPTY; // init SA[n1..n-1]

  tmp=bkt[separator]--;// shift one position left of bkt[separator]
  for(i=n1-1; i>=0; i--) {
      j=SA[i]; SA[i]=EMPTY;
      if(i==0)
          SA[0]=n-1;
      else
          SA[bkt[chr(j)]--]=j;
  }
  
  SA[tmp]=SA[0]-1;// insert the last separator at the end of bkt[separator]
  induceSAl_generalized(t, SA, s, bkt, n, K, cs, level, separator); 
  
  #if PHASES
	printf("phase 3:\n");
	time_stop(t_start_phase, c_start_phase);
  #endif

  #if PHASES
	t_start_phase = time(NULL);
	c_start_phase =  clock();
  #endif

  induceSAs_generalized(t, SA, s, bkt, n, K, cs, level, separator); 
  
  free(bkt); 
  free(t);

  #if DEPTH
  printf("depth %" PRIdN ":\n", depth);
  time_stop(t_start, c_start);
  #endif
  
  #if PHASES
	printf("phase 4:\n");
	time_stop(t_start_phase, c_start_phase);
  #endif


return depth;
}

/*****************************************************************************/

int_t gSAIS_LCP(uint_t *s, int_t *SA, int_t *LCP, int_t n, int_t K, int cs, unsigned char separator, int level) {
  int_t i, j;
  
  #if PHASES
  time_t t_start_phase = 0.0;
  clock_t c_start_phase = 0.0;
  #endif

  #if DEPTH
  time_t t_start = time(NULL);
  clock_t c_start =  clock();
  #endif

  #if PHASES
	t_start_phase = time(NULL);
	c_start_phase =  clock();
  #endif

  unsigned char *t=(unsigned char *)malloc(n/8+1); // LS-type array in bits

  // stage 1: reduce the problem by at least 1/2

  // Classify the type of each character
  tset(n-2, 0); tset(n-1, 1); // the sentinel must be in s1, important!!!
  for(i=n-3; i>=0; i--) tset(i, (chr(i)<chr(i+1) || (chr(i)==chr(i+1) && tget(i+1)==1))?1:0);

  int_t *bkt = (int_t *)malloc(sizeof(int_t)*K); // bucket counters

  // sort all the S-substrings
  getBuckets((int_t*)s, bkt, n, K, cs, true); // find ends of buckets
  for(i=0; i<n; i++) SA[i]=EMPTY;

  // gsa-is
  int_t tmp=bkt[separator]--;// shift one position left of bkt[separator]

  SA[0]=n-1; // set the single sentinel LMS-substring

  SA[tmp]=SA[0]-1;// insert the last separator at the end of bkt[separator]

  int_t p=n-2;

  for(i=n-2; i>=0; i--){
    if(isLMS(i)){

      if(chr(i)==separator) // gsa-is
        SA[++bkt[chr(p)]]=EMPTY; // removes LMS-positions that induces separator suffixes

      SA[bkt[chr(i)]--]=i;
      p=i;
    }
  }

  induceSAl_generalized(t, SA, s, bkt, n, K, cs, level, separator); 
  induceSAs_generalized(t, SA, s, bkt, n, K, cs, level, separator); 

  // insert separator suffixes in their buckets
  bkt[separator]=1; // gsa-is
  for(i=0; i<n-1; i++) 
    if(chr(i)==separator)
      SA[bkt[chr(i)]++]=i;
  
  free(bkt);

  // compact all the sorted substrings into the first n1 items of s
  // 2*n1 must be not larger than n (proveable)
  int_t n1=0;
  for(i=0; i<n; i++)
    if(isLMS(SA[i]))
      SA[n1++]=SA[i];


  // Init the name array buffer
  for(i=n1; i<n; i++) SA[i]=EMPTY;

  // find the lexicographic names of all substrings
  int_t name=0, prev=-1;
  for(i=0; i<n1; i++) {

    int_t pos=SA[i]; int diff=false;
    int_t d;
    for(d=0; d<n; d++)
      if(prev==-1 || pos+d==n-1 || prev+d==n-1 ||
         chr(pos+d)!=chr(prev+d) ||
         (chr(pos+d)==separator && chr(prev+d)==separator) || // $_i < $_j iff i < j
         tget(pos+d)!=tget(prev+d) // gsa-is
      )
      { 
	diff=true; break;
      }
      else
        if(d>0 && (isLMS(pos+d) || isLMS(prev+d)))
          break;
	LCP[i]=d;
	
    if(diff){
      name++; 
      prev=pos; 
    }

    pos=(pos%2==0)?pos/2:(pos-1)/2;
    SA[n1+pos]=name-1; 
  }

  for(i=n-1, j=n-1; i>=n1; i--)
	  if(SA[i]!=EMPTY) SA[j--]=SA[i];
  
  #if PHASES
	printf("phase 1:\n");
	time_stop(t_start_phase, c_start_phase);
  #endif

  #if PHASES
	t_start_phase = time(NULL);
	c_start_phase =  clock();
  #endif

  // s1 is done now
  int_t *SA1=SA, *s1=SA+n-n1;

  // stage 2: solve the reduced problem

  int_t depth=1;
  // recurse if names are not yet unique
  if(name<n1) {
    depth += SAIS((int_t*)s1, SA1, n1, name, sizeof(int_t), level+1);
  } else{ // generate the suffix array of s1 directly
    for(i=0; i<n1; i++) SA1[s1[i]] = i;
  }
  
  #if PHASES
	printf("phase 2:\n");
	time_stop(t_start_phase, c_start_phase);
  #endif

  #if PHASES
	t_start_phase = time(NULL);
	c_start_phase =  clock();
  #endif

  // stage 3: induce the result for the original problem

  bkt = (int_t *)malloc(sizeof(int_t)*K); // bucket counters

  // put all left-most S characters into their buckets
  getBuckets((int_t*)s, bkt, n, K, cs, true); // find ends of buckets
  j=0;
  for(i=1; i<n; i++)
    if(isLMS(i)) s1[j++]=i; // get p1

/**/

  int_t *RA=s1;
  int_t *PLCP=LCP+n-n1;//PHI is stored in PLCP array

  //compute the LCP of consecutive LMS-suffixes
  compute_lcp_phi_sparse_sais((int_t*)s, SA1, RA, LCP, PLCP, n1, cs, separator); 

/**/

  for(i=0; i<n1; i++) SA1[i]=s1[SA1[i]]; // get index in s1
  for(i=n1; i<n; i++) SA[i]=U_MAX; //EMPTY; // init SA[n1..n-1]
/**/
  for(i=n1;i<n;i++) LCP[i]=0;
/**/

  int_t l;
  tmp=bkt[separator]--;// shift one position left of bkt[separator]
  for(i=n1-1; i>=0; i--) {
      j=SA[i]; SA[i]=U_MAX;
      l=LCP[i]; LCP[i]=0;

      if(i==0)
          SA[0]=n-1;
      else{
          SA[bkt[chr(j)]]=j;          
          LCP[bkt[chr(j)]--]=l;
      }
  }
  
  SA[tmp]=SA[0]-1;// insert the last separator at the end of bkt[separator]
  induceSAl_generalized_LCP(t, SA, LCP, s, bkt, n, K, cs, level, separator); 
  
  #if PHASES
	printf("phase 3:\n");
	time_stop(t_start_phase, c_start_phase);
  #endif

  #if PHASES
	t_start_phase = time(NULL);
	c_start_phase =  clock();
 
  #endif

  induceSAs_generalized_LCP(t, SA, LCP, s, bkt, n, K, cs, level, separator); 
  
  free(bkt); 
  free(t);

  #if DEPTH
  printf("depth %" PRIdN ":\n", depth);
  time_stop(t_start, c_start);
  #endif
  
  #if PHASES
	printf("phase 4:\n");
	time_stop(t_start_phase, c_start_phase);
  #endif


return depth;
}

/*****************************************************************************/

int_t gSAIS_DA(uint_t *s, int_t *SA, int_t *DA, int_t n, int_t K, int cs, unsigned char separator, int level) {
  int_t i, j;
  
  #if PHASES
  time_t t_start_phase = 0.0;
  clock_t c_start_phase = 0.0;
  #endif

  #if DEPTH
  time_t t_start = time(NULL);
  clock_t c_start =  clock();
  #endif

  #if PHASES
	t_start_phase = time(NULL);
	c_start_phase =  clock();
  #endif

  unsigned char *t=(unsigned char *)malloc(n/8+1); // LS-type array in bits

  // stage 1: reduce the problem by at least 1/2

  // Classify the type of each character
  tset(n-2, 0); tset(n-1, 1); // the sentinel must be in s1, important!!!
  for(i=n-3; i>=0; i--) tset(i, (chr(i)<chr(i+1) || (chr(i)==chr(i+1) && tget(i+1)==1))?1:0);

  int_t *bkt = (int_t *)malloc(sizeof(int_t)*K); // bucket counters

  // sort all the S-substrings
  getBuckets((int_t*)s, bkt, n, K, cs, true); // find ends of buckets
  for(i=0; i<n; i++) SA[i]=EMPTY;

  // gsa-is
  int_t tmp=bkt[separator]--;// shift one position left of bkt[separator]

  SA[0]=n-1; // set the single sentinel LMS-substring

  SA[tmp]=SA[0]-1;// insert the last separator at the end of bkt[separator]

  int_t p=n-2;

  for(i=n-2; i>=0; i--){
    if(isLMS(i)){

      if(chr(i)==separator) // gsa-is
        SA[++bkt[chr(p)]]=EMPTY; // removes LMS-positions that induces separator suffixes

      SA[bkt[chr(i)]--]=i;
      p=i;
    }
  }

  induceSAl_generalized(t, SA, s, bkt, n, K, cs, level, separator); 
  induceSAs_generalized(t, SA, s, bkt, n, K, cs, level, separator); 

  // insert separator suffixes in their buckets
  bkt[separator]=1; // gsa-is
  for(i=0; i<n-1; i++) 
    if(chr(i)==separator)
      SA[bkt[chr(i)]++]=i;
  
  free(bkt);

  // compact all the sorted substrings into the first n1 items of s
  // 2*n1 must be not larger than n (proveable)
  int_t n1=0;
  for(i=0; i<n; i++)
    if(isLMS(SA[i]))
      SA[n1++]=SA[i];


  // Init the name array buffer
  for(i=n1; i<n; i++) SA[i]=EMPTY;

  // find the lexicographic names of all substrings
  int_t name=0, prev=-1;
  for(i=0; i<n1; i++) {

    int_t pos=SA[i]; int diff=false;
    int_t d;
    for(d=0; d<n; d++)
      if(prev==-1 || pos+d==n-1 || prev+d==n-1 ||
         chr(pos+d)!=chr(prev+d) ||
         (chr(pos+d)==separator && chr(prev+d)==separator) || // $_i < $_j iff i < j
         tget(pos+d)!=tget(prev+d) // gsa-is
      )
      { 
	diff=true; break;
      }
      else
        if(d>0 && (isLMS(pos+d) || isLMS(prev+d)))
          break;

    if(diff){
      name++; 
      prev=pos; 
    }

    pos=(pos%2==0)?pos/2:(pos-1)/2;
    SA[n1+pos]=name-1; 
  }

  for(i=n-1, j=n-1; i>=n1; i--)
	  if(SA[i]!=EMPTY) SA[j--]=SA[i];
  
  #if PHASES
	printf("phase 1:\n");
	time_stop(t_start_phase, c_start_phase);
  #endif

  #if PHASES
	t_start_phase = time(NULL);
	c_start_phase =  clock();
  #endif

  // s1 is done now
  int_t *SA1=SA, *s1=SA+n-n1;

  // stage 2: solve the reduced problem

  int_t depth=1;
  // recurse if names are not yet unique
  if(name<n1) {
    depth += SAIS((int_t*)s1, SA1, n1, name, sizeof(int_t), level+1);
  } else{ // generate the suffix array of s1 directly
    for(i=0; i<n1; i++) SA1[s1[i]] = i;
  }
  
  #if PHASES
	printf("phase 2:\n");
	time_stop(t_start_phase, c_start_phase);
  #endif

  #if PHASES
	t_start_phase = time(NULL);
	c_start_phase =  clock();
  #endif

  // stage 3: induce the result for the original problem
  #if DEBUG
  printf("recursive:\n");
  printf("SA\n");
  for(i=0; i<n; i++)
    printf("%" PRIdN "\t", SA[i]+1);
  printf("\n\n");
  #endif
  
/**/
  int_t *d1=DA+n-n1;
  int_t k=0;
/**/
  
  bkt = (int_t *)malloc(sizeof(int_t)*K); // bucket counters

  // put all left-most S characters into their buckets
  getBuckets((int_t*)s, bkt, n, K, cs, true); // find ends of buckets
  j=0;
  for(i=1; i<n; i++){
    if(isLMS(i)){
		s1[j]=i; // get p1
		d1[j]=k;
		j++;
	}
	if(chr(i)==separator)k++;
  }
  d1[n1-1]=k;
/**/  

  #if DEBUG
  printf("getSAlms:\n");
  printf("SA\n");
  for(i=0; i<n; i++)
    printf("%" PRIdN "\t", SA[i]+1);
  printf("\n");
  printf("DA\n");
  for(i=0; i<n; i++)
        printf("%" PRIdN "\t", DA[i]);
  printf("\n\n");
  #endif
  
  for(i=0; i<n1; i++) {DA[i]=d1[SA[i]]; SA1[i]=s1[SA1[i]];} // get index in s1
  for(i=n1; i<n; i++) {SA[i]=EMPTY;DA[i]=-1;} // init SA[n1..n-1]

  #if DEBUG
  printf("\nstage 3:\n\n");
  printf("mapping back:\n");
  printf("SA\n");
  for(i=0; i<n; i++)
    printf("%" PRIdN "\t", SA[i]+1);
  printf("\n");
  printf("DA\n");
  for(i=0; i<n; i++)
        printf("%" PRIdN "\t", DA[i]);
  printf("\n\n");
  #endif
    
  tmp=bkt[separator]--;// shift one position left of bkt[separator]
  for(i=n1-1; i>=0; i--) {
      j=SA[i]; SA[i]=EMPTY;
      if(i==0)
          SA[0]=n-1;
      else{
          SA[bkt[chr(j)]]=j;
          DA[bkt[chr(j)]--]=DA[i];
      }
  }
  
  SA[tmp]=SA[0]-1;// insert the last separator at the end of bkt[separator]
  DA[tmp]=tmp-1;
  
  #if DEBUG
  printf("SA (mapped)\n");
  for(i=0; i<n; i++)
        printf("%" PRIdN "\t", SA[i]+1);
  printf("\n");
  printf("DA\n");
  for(i=0; i<n; i++)
        printf("%" PRIdN "\t", DA[i]);
  printf("\n\n");
  #endif
      
  induceSAl_generalized_DA(t, SA, DA, s, bkt, n, K, cs, level, separator); 

  #if DEBUG
  printf("L-type\n");
  for(i=0; i<n; i++)
        printf("%" PRIdN "\t", SA[i]+1);
  printf("\n");
  printf("DA\n");
  for(i=0; i<n; i++)
        printf("%" PRIdN "\t", DA[i]);
  printf("\n\n");
  #endif
  
  #if PHASES
	printf("phase 3:\n");
	time_stop(t_start_phase, c_start_phase);
  #endif

  #if PHASES
	t_start_phase = time(NULL);
	c_start_phase =  clock();
  #endif

  induceSAs_generalized_DA(t, SA, DA, s, bkt, n, K, cs, level, separator); 

  #if DEBUG
  printf("S-type\n");
  for(i=0; i<n; i++)
        printf("%" PRIdN "\t", SA[i]+1);
  printf("\n");
  printf("DA\n");
  for(i=0; i<n; i++)
        printf("%" PRIdN "\t", DA[i]);
  printf("\n\n");
  #endif
  
  free(bkt); 
  free(t);

  #if DEPTH
  printf("depth %" PRIdN ":\n", depth);
  time_stop(t_start, c_start);
  #endif
  
  #if PHASES
	printf("phase 4:\n");
	time_stop(t_start_phase, c_start_phase);
  #endif


return depth;
}

/*****************************************************************************/

int_t gSAIS_LCP_DA(uint_t *s, int_t *SA, int_t *LCP, int_t *DA, int_t n, int_t K, int cs, unsigned char separator, int level) {
  int_t i, j;
  
  #if PHASES
  time_t t_start_phase = 0.0;
  clock_t c_start_phase = 0.0;
  #endif

  #if DEPTH
  time_t t_start = time(NULL);
  clock_t c_start =  clock();
  #endif

  #if PHASES
	t_start_phase = time(NULL);
	c_start_phase =  clock();
  #endif

  unsigned char *t=(unsigned char *)malloc(n/8+1); // LS-type array in bits

  // stage 1: reduce the problem by at least 1/2

  // Classify the type of each character
  tset(n-2, 0); tset(n-1, 1); // the sentinel must be in s1, important!!!
  for(i=n-3; i>=0; i--) tset(i, (chr(i)<chr(i+1) || (chr(i)==chr(i+1) && tget(i+1)==1))?1:0);

  int_t *bkt = (int_t *)malloc(sizeof(int_t)*K); // bucket counters

  // sort all the S-substrings
  getBuckets((int_t*)s, bkt, n, K, cs, true); // find ends of buckets
  for(i=0; i<n; i++) SA[i]=EMPTY;

  // gsa-is
  int_t tmp=bkt[separator]--;// shift one position left of bkt[separator]

  SA[0]=n-1; // set the single sentinel LMS-substring

  SA[tmp]=SA[0]-1;// insert the last separator at the end of bkt[separator]

  int_t p=n-2;

  for(i=n-2; i>=0; i--){
    if(isLMS(i)){

      if(chr(i)==separator) // gsa-is
        SA[++bkt[chr(p)]]=EMPTY; // removes LMS-positions that induces separator suffixes

      SA[bkt[chr(i)]--]=i;
      p=i;
    }
  }

  induceSAl_generalized(t, SA, s, bkt, n, K, cs, level, separator); 
  induceSAs_generalized(t, SA, s, bkt, n, K, cs, level, separator); 

  // insert separator suffixes in their buckets
  bkt[separator]=1; // gsa-is
  for(i=0; i<n-1; i++) 
    if(chr(i)==separator)
      SA[bkt[chr(i)]++]=i;
  
  free(bkt);

  // compact all the sorted substrings into the first n1 items of s
  // 2*n1 must be not larger than n (proveable)
  int_t n1=0;
  for(i=0; i<n; i++)
    if(isLMS(SA[i]))
      SA[n1++]=SA[i];


  // Init the name array buffer
  for(i=n1; i<n; i++) SA[i]=EMPTY;

  // find the lexicographic names of all substrings
  int_t name=0, prev=-1;
  for(i=0; i<n1; i++) {

    int_t pos=SA[i]; int diff=false;
    int_t d;
    for(d=0; d<n; d++)
      if(prev==-1 || pos+d==n-1 || prev+d==n-1 ||
         chr(pos+d)!=chr(prev+d) ||
         (chr(pos+d)==separator && chr(prev+d)==separator) || // $_i < $_j iff i < j
         tget(pos+d)!=tget(prev+d) // gsa-is
      )
      { 
	diff=true; break;
      }
      else
        if(d>0 && (isLMS(pos+d) || isLMS(prev+d)))
          break;
	LCP[i]=d;
	
    if(diff){
      name++; 
      prev=pos; 
    }

    pos=(pos%2==0)?pos/2:(pos-1)/2;
    SA[n1+pos]=name-1; 
  }

  for(i=n-1, j=n-1; i>=n1; i--)
	  if(SA[i]!=EMPTY) SA[j--]=SA[i];
  
  #if PHASES
	printf("phase 1:\n");
	time_stop(t_start_phase, c_start_phase);
  #endif

  #if PHASES
	t_start_phase = time(NULL);
	c_start_phase =  clock();
  #endif

  // s1 is done now
  int_t *SA1=SA, *s1=SA+n-n1;

  // stage 2: solve the reduced problem

  int_t depth=1;
  // recurse if names are not yet unique
  if(name<n1) {
    depth += SAIS((int_t*)s1, SA1, n1, name, sizeof(int_t), level+1);
  } else{ // generate the suffix array of s1 directly
    for(i=0; i<n1; i++) SA1[s1[i]] = i;
  }
  
  #if PHASES
	printf("phase 2:\n");
	time_stop(t_start_phase, c_start_phase);
  #endif

  #if PHASES
	t_start_phase = time(NULL);
	c_start_phase =  clock();
  #endif

/**/
  int_t *d1=DA+n-n1;
  int_t k=0;
/**/

  // stage 3: induce the result for the original problem

  bkt = (int_t *)malloc(sizeof(int_t)*K); // bucket counters

  // put all left-most S characters into their buckets
  getBuckets((int_t*)s, bkt, n, K, cs, true); // find ends of buckets
  j=0;
  for(i=1; i<n; i++){
    if(isLMS(i)){
                s1[j]=i; // get p1
                d1[j]=k;
                j++;
        }
        if(chr(i)==separator)k++;
  }
  d1[n1-1]=k;

/**/

  int_t *RA=s1;
  int_t *PLCP=LCP+n-n1;//PHI is stored in PLCP array

  //compute the LCP of consecutive LMS-suffixes
  compute_lcp_phi_sparse_sais((int_t*)s, SA1, RA, LCP, PLCP, n1, cs, separator); 

/**/

  for(i=0; i<n1; i++) {DA[i]=d1[SA[i]]; SA1[i]=s1[SA1[i]];} // get index in s1
  for(i=n1; i<n; i++) {SA[i]=U_MAX; DA[i]=-1;} //EMPTY; // init SA[n1..n-1]
/**/
  for(i=n1;i<n;i++) LCP[i]=0;
/**/

  int_t l;
  tmp=bkt[separator]--;// shift one position left of bkt[separator]
  for(i=n1-1; i>=0; i--) {
      j=SA[i]; SA[i]=U_MAX;
      l=LCP[i]; LCP[i]=0;

      if(i==0)
          SA[0]=n-1;
      else{
          SA[bkt[chr(j)]]=j; 
          DA[bkt[chr(j)]]=DA[i];  
          LCP[bkt[chr(j)]--]=l;
      }
  }
  
  SA[tmp]=SA[0]-1;// insert the last separator at the end of bkt[separator]
  DA[tmp]=tmp-1;
   
  induceSAl_generalized_LCP_DA(t, SA, LCP, DA, s, bkt, n, K, cs, level, separator); 
  
  #if PHASES
	printf("phase 3:\n");
	time_stop(t_start_phase, c_start_phase);
  #endif

  #if PHASES
	t_start_phase = time(NULL);
	c_start_phase =  clock();
 
  #endif

  induceSAs_generalized_LCP_DA(t, SA, LCP, DA, s, bkt, n, K, cs, level, separator); 
  
  free(bkt); 
  free(t);

  #if DEPTH
  printf("depth %" PRIdN ":\n", depth);
  time_stop(t_start, c_start);
  #endif
  
  #if PHASES
	printf("phase 4:\n");
	time_stop(t_start_phase, c_start_phase);
  #endif


return depth;
}

/*****************************************************************************/


int sais(unsigned char *s, uint_t *SA, uint_t n){
  if((s == NULL) || (SA == NULL) || (n < 0)) return -1;
  return SAIS((int_t*)s, (int_t *)SA, n, 256, sizeof(char), 0);
}

int sais_int(int_t *s, uint_t *SA, uint_t n, uint_t k){
  if((s == NULL) || (SA == NULL) || (n < 0)) return -1;
  return SAIS((int_t*)s, (int_t *)SA, n, k, sizeof(int_t), 0);
}

int gsais(unsigned char *s, uint_t *SA, int_t *LCP, int_t *DA, uint_t n){
  if((s == NULL) || (SA == NULL) || (n < 0)) return -1;

  if((LCP == NULL) && (DA == NULL))
	return gSAIS((uint_t*)s, (int_t *)SA, n, 256, sizeof(char), 1, 0);
  else if (DA == NULL)
	return gSAIS_LCP((uint_t*)s, (int_t *)SA, LCP, n, 256, sizeof(char), 1, 0);
  else if (LCP == NULL)
	return gSAIS_DA((uint_t*)s, (int_t *)SA, DA, n, 256, sizeof(char), 1, 0);
  else
	return gSAIS_LCP_DA((uint_t*)s, (int_t *)SA, LCP, DA, n, 256, sizeof(char), 1, 0);
}

int gsais_int(uint_t *s, uint_t *SA, int_t *LCP, int_t *DA, uint_t n, uint_t k){
  if((s == NULL) || (SA == NULL) || (n < 0)) return -1;
  
  if((LCP == NULL) && (DA == NULL))
	return gSAIS((uint_t*)s, (int_t *)SA, n, k, sizeof(int_t), 1, 0);
  else if (DA == NULL)
	return gSAIS_LCP((uint_t*)s, (int_t *)SA, LCP, n, k, sizeof(int_t), 1, 0);
  else if (LCP == NULL)
	return gSAIS_DA((uint_t*)s, (int_t *)SA, DA, n, k, sizeof(int_t), 1, 0);
  else
	return gSAIS_LCP_DA((uint_t*)s, (int_t *)SA, LCP, DA, n, k, sizeof(int_t), 1, 0);
}

/*****************************************************************************/
