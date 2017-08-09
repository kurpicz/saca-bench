/*
 * gsaca.c for gsaca
 * Copyright (c) 2015 Uwe Baier All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "gsaca.h"
#include <limits.h>
#include <stdlib.h>

#define C_SIZE (UCHAR_MAX+1)
#define C_CUM(c) C[c << 1]		//cumulative C array
#define C_CNT(c) C[(c << 1) + 1] 	//C array counting characters
#define GLINK PREV			//alias
#define GENDLINK ISA			//alias

//helper functions for GSIZE, an array for variable-length numbers:
//Let GSIZE[i]=k and GSIZE[j]=l (i < j) be any two numbers in GSIZE.
//Then, the algorithm always ensures that i+k <= j holds.
//All operations except calloc and free are supposed to be O(1).
void *       gsize_calloc( unsigned int n );
void         gsize_free( void *g );
void         gsize_set( void *g, unsigned int pos, unsigned int val );
unsigned int gsize_get( const void *g, unsigned int pos );
void         gsize_clear( void *g, unsigned int pos ); //sets gsize at pos to 0
unsigned int gsize_dec_get( void *g, unsigned int pos ); //first decrement, then read
void         gsize_inc( void *g, unsigned int pos ); //increment

int gsaca_unsigned(const unsigned char *S, unsigned int *SA, unsigned int n) {
	unsigned int *ISA, *PREV;
	void *GSIZE;
	unsigned int i,j;
	unsigned int gstart, gend;
	unsigned int s, p, sr;
	unsigned int tmp, gstarttmp, gendtmp;
	unsigned int C[2*C_SIZE]; //counts and cumulative counts

	if (n == 0)		return 0;
	if (S[n-1] != '\0')	return -1;

	//set up needed structures
	ISA = (unsigned int *)malloc( n * sizeof(unsigned int) );
	PREV = (unsigned int *)malloc( n * sizeof(unsigned int) );
	GSIZE = gsize_calloc( n );
	if (!ISA || !PREV || !GSIZE)	{
		free(ISA); free(PREV); gsize_free(GSIZE);
		return -2;
	}

	//// PHASE 1: pre-sort suffixes ////
	//build initial group structure
	for (i = 0; i < C_SIZE; ++i)	C_CNT(i) = 0;
	for (i = 0; i < n; ++i) ++C_CNT(S[i]); //count characters

	if (C_CNT(0) != 1) { //more than 1 nullbyte
		free(ISA); free(PREV); gsize_free(GSIZE);
		return -1;
	}
	j = 0;
	for (i = 0; i < C_SIZE; ++i) { //build cumulative counts and set up GSIZE
		if (C_CNT(i) > 0) {
			C_CUM(i) = j;
			gsize_set(GSIZE, j, C_CNT(i));
			j += C_CNT(i);
		}
	}
	for (i = n-1; i < n; --i) { //set up ISA, GLINK and SA
		gstart = C_CUM(S[i]);
		sr = gstart + --C_CNT(S[i]);
		GLINK[i] = gstart;
		ISA[i] = sr;
		SA[sr] = i;
	}

	//process groups from highest to lowest
	for (gend = n-1; gend > 0; gend = gstarttmp-1) {
		gstart = GLINK[SA[gend]];
		gstarttmp = gstart;	gendtmp = gend;

		//clear GSIZE group size for marking
		gsize_clear(GSIZE, gstart);

		//compute prev - pointers and mark suffixes of own group that
		//have a prev-pointer of own group pointing to them
		for (i = gend; i >= gstart; --i) {
			s = SA[i]; //use prev - pointers from already used groups
			for (p = s-1; p < n; p = PREV[p]) {
				if (ISA[p] <= gend) {
					if (ISA[p] >= gstart) {
						gsize_set(GSIZE, ISA[p], 1); //mark ISA[p]
					}
					break;
				}
			}
			PREV[s] = p;
		}

		//set GENDLINK of all suffixes for phase 2 and move unmarked
		// suffixes to the front of the actual group
		j = 0;
		for (i = gstart; i <= gend; ++i) {
			s = SA[i];
			GENDLINK[s] = gend;
			if (gsize_get(GSIZE, i) == 0) { //i is not marked
				SA[gstart+(j++)] = s;
			}
		}

		//order the suffixes according on how much suffixes of same
		//group are jumped by them
		gend = gstart + j; //exclusive bound by now
		j = 0;
		do {
			i = gend-1; sr = gend;
			while (i >= gstart) {
				s = SA[i];
				p = PREV[s];
				if (p < n) {
					if (ISA[p] < gstarttmp) { //p is in a lex. smaller group
						SA[i--] = SA[--gend];
						SA[gend] = p; //push prev to back
					} else { //p is in same group
						PREV[s] = PREV[p];
						PREV[p] = n; //clear prev pointer, is not used in phase 2
						--i;
					}
				} else { //prev points to nothing
					SA[i] = SA[gstart++]; //remove entry
				}
			}
			//write number of suffixes written to end on stack using GSIZE
			if (gend < sr) {
				gsize_set(GSIZE, gend, sr-gend);
				++j; //also, count number of splitted groups
			}
		} while (gstart < gend);

		//rearrange previous suffixes stored in other groups
		while (j--) {
			gend = gstart + gsize_get(GSIZE, gstart);
			//decrement group count of previous group suffixes, and move them to back
			for (i = gend-1; i >= gstart; --i) {
				p = SA[i];
				sr = GLINK[p];
				sr += gsize_dec_get(GSIZE, sr);
				//move p to back by exchanging it with last suffix s of group
				s = SA[sr];
				tmp = ISA[p];
				SA[tmp] = s;	ISA[s] = tmp;
				SA[sr] = p;	ISA[p] = sr;				
			}
			//set new GLINK for moved suffixes
			for (i = gstart; i < gend; ++i) {
				p = SA[i];
				sr = GLINK[p];
				sr += gsize_get(GSIZE, sr);
				GLINK[p] = sr;
			}
			//set up GSIZE for newly created groups
			for (i = gstart; i < gend; ++i) {
				p = SA[i];
				sr = GLINK[p];
				gsize_inc(GSIZE, sr);
			}
			gstart = gend;
		}
		
		//prepare current group for phase 2
		SA[gendtmp] = gstarttmp; //counter where to place next entry
	}

	//// PHASE 2: sort suffixes finally ////
	SA[0] = n-1;
	for (i = 0; i < n; i++) {
		s = SA[i]-1;
		while (s < n) {
			sr = GENDLINK[s];
			if (sr == n) //suffix already placed to SA, stop
				break;
			sr = SA[sr]++; //get position where to write s
			SA[sr] = s;
			//mark that suffix is placed in SA already
			GENDLINK[s] = n; 
			/* NOTE: Using annother technique
			   to indicate if a suffix is placed to SA already,
			   ISA could be computed too. A possible way would be to
			   set PREV[s] = s, and check that in the above if. 
			*/
			s = PREV[s]; //process next suffix
		}
	}

	free(ISA); free(PREV); gsize_free(GSIZE);
	return 0;
}

int gsaca(const unsigned char *S, int *SA, int n) {
	if (n < 0)	return -1;
	return gsaca_unsigned( S, (unsigned int *)SA, (unsigned int)n);
}


//// GSIZE OPERATIONS /////////////////////////////////////////////////////////
void *gsize_calloc( unsigned int n ) {
	return calloc(n, sizeof(unsigned int));
}

void gsize_free( void *g ) {
	free(g);
}

void gsize_set( void *g, unsigned int pos, unsigned int val ) {
	((unsigned int *)g)[pos] = val;
}

unsigned int gsize_get( const void *g, unsigned int pos ) {
	return ((const unsigned int *)g)[pos];
}

void gsize_clear( void *g, unsigned int pos ) {
	((unsigned int *)g)[pos] = 0;
}

unsigned int gsize_dec_get( void *g, unsigned int pos ) {
	return --((unsigned int *)g)[pos];
}

void gsize_inc( void *g, unsigned int pos ) {
	++((unsigned int *)g)[pos];
}
