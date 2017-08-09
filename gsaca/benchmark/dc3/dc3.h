#ifndef _DC3_H
#define _DC3_H 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//expects a nullterminated string (S[n-1] = 0), computes suffix array.
//returns 0 on success, -1 if n < 0, or -2 if out of memory
int dc3(const unsigned char *S, int *SA, int n);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif
