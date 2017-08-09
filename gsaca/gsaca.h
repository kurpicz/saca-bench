/*
 * gsaca.h for gsaca
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

#ifndef _GSACA_H
#define _GSACA_H 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * constructs the suffix array of the given string S.
 * @param S a nullterminated string of length n, i.e. S[n-1] = '\0'.
 * @param SA an array of size n, where the suffix array will be stored to.
 * @param n length of the string and the suffix array.
 * @return 0 if construction worked, -1 on illegal parameters,
 *	or -2 if not enough memory is available.
 */
int gsaca(const unsigned char *S, int *SA, int n);

/**
 * constructs the suffix array of the given string S.
 * @param S a nullterminated string of length n, i.e. S[n-1] = '\0'.
 * @param SA an array of size n, where the suffix array will be stored to.
 * @param n length of the string and the suffix array.
 * @return 0 if construction worked, -1 on illegal parameters,
 *	or -2 if not enough memory is available.
 */
int gsaca_unsigned(const unsigned char *S, unsigned int *SA, unsigned int n);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* _GSACA_H */
