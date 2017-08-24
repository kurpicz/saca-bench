# gsa-is

gSAIS and gSACA-K are suffix array construction algorithms for string collections. 

## Introduction

gSAIS and gSACA-K [1, 2] extend the linear-time suffix sorting algorithms SAIS
[3] and SACA-K [4] to compute the suffix array for a string collection,
maintaining their theoretical bounds and improving their practical performance.

Moreover, gSAIS and gSACA-K can also compute the LCP-array (LCP) and the
document array (DA) as a byproduct, with the same theoretical bounds.

Our algorithms, gSACA-K, gSACA-K+LCP and gSACA-K+DA are optimal for strings
from constant alphabets. Experimental results have shown that our algorithms
are fast with a very small memory footprint.


## Build requirements

An ANSI C Compiler (e.g. GNU GCC)

## API

**gsacak.h** (the same for gsais.h)

```c
/** @brief Computes the suffix array SA (LCP, DA) of T^cat in s[0..n-1]
 *
 *  @param s    input concatenated string, using separators s[i]=1 and with s[n-1]=0
 *  @param SA   suffix array 
 *  @param LCP  LCP array 
 *  @param DA   Document array
 *  @param n    string length
 * 
 *  @return depth of the recursive calls.
 */
int gsacak(unsigned char *s, uint_t *SA, int_t *LCP, int_t *DA, uint_t n);

/** @brief Computes the suffix array SA (LCP, DA) of T^cat in s[0..n-1]
 *
 *  @param s    input concatenated string, using separators s[i]=1 and with s[n-1]=0
 *  @param SA   suffix array 
 *  @param LCP  LCP array 
 *  @param DA   Document array
 *  @param n    string length
 *  @param K    alphabet size
 * 
 *  @return depth of the recursive calls.
 */
int gsacak_int(uint_t *s, uint_t *SA, int_t *LCP, int_t *DA, uint_t n, uint_t k);
```

## Options

```c
 int gsacak(s, SA, NULL, NULL, n) //computes only SA
 int gsacak(s, SA, LCP,  NULL, n) //computes SA and LCP
 int gsacak(s, SA, NULL, DA,   n) //computes SA and DA
 int gsacak(s, SA, LCP,  DA,   n) //computes SA, LCP and DA
```


## Example

**Compilation:**

```sh
gcc -c gsacak.c experiments/external/malloc_count/malloc_count.c
gcc test.c -o test gsacak.o malloc_count.o -ldl
```

**Run a test:**

```c
./test banana anaba anan
```

**Output:**

```c
sizeof(int_t) = 4 bytes
N = 18
Text = banana$anaba$anan$#
i	SA	DA	LCP	BWT	suffixes
0	18	3	0	$	#
1	6	0	0	a	$anaba$anan$#
2	12	1	0	a	$anan$#
3	17	2	0	n	$#
4	5	0	0	n	a$anaba$anan$#
5	11	1	1	b	a$anan$#
6	9	1	1	n	aba$anan$#
7	15	2	1	n	an$#
8	3	0	2	n	ana$anaba$anan$#
9	7	1	3	$	anaba$anan$#
10	13	2	3	$	anan$#
11	1	0	4	b	anana$anaba$anan$#
12	10	1	0	a	ba$anan$#
13	0	0	2	#	banana$anaba$anan$#
14	16	2	0	a	n$#
15	4	0	1	a	na$anaba$anan$#
16	8	1	2	a	naba$anan$#
17	14	2	2	a	nan$#
18	2	0	3	a	nana$anaba$anan$#
malloc_count ### exiting, total: 19,703, peak: 10,487, current: 0
```
**Remark:**

The peak memory 10,487 is exactly 10KB + 247 bytes.
10KB is the workspace and 247 (13\*19 bytes) bytes is the space used by the concatenated string s and the arrays SA, LCP and DA (13\*n bytes)


**Strings larger than n=2^20:**

One can change to 64 bits integers adding -DM64=1 in the compilation.


## Citation

Please, if you use this tool in an academic setting cite the following paper:

    @article{LouzaGT17b,
     author    = {Felipe Alves da Louza and Simon Gog and Guilherme P. Telles},
     title     = {Inducing enhanced suffix arrays for string collections},
     journal   = {Theor. Comput. Sci.},
     volume    = {678},
     pages     = {22--39},
     year      = {2017},
     url       = {http://www.sciencedirect.com/science/article/pii/S0304397517302621},
     doi       = {10.1016/j.tcs.2017.03.039},
    }
    

--
## References

\[1\] Louza, F. A., Gog, S., Telles, G. P., Induced Suffix Sorting for String Collections. In Proc. DCC, pp. 43-58, 2016, [IEEE](http://ieeexplore.ieee.org/document/7786148/).

\[2\] Louza, F. A., Gog, S., Telles, G. P., Inducing enhanced suffix arrays for string collections. Theor. Comput. Sci., vol. 678, pp. 22-39, 2017, [Elsevier](http://www.sciencedirect.com/science/article/pii/S0304397517302621).

\[3\] Nong G., Zhang S., Chan W. H., Two efficient algorithms for linear time suffix array construction, IEEE Trans. Comput., vol. 60, no. 10, pp. 1471–1484, 2011

\[4\] Nong, G., Practical linear-time O(1)-workspace suffix sorting for constant alphabets, ACM Trans. Inform. Syst., vol. 31, no. 3, pp. 1–15, 2013

