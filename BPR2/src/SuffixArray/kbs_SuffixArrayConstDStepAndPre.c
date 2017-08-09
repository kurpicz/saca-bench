#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <config.h>
#include "kbs_Alphabet.h"
#include "kbs_Error.h"
#include "kbs_Limits.h"
#include "kbs_Math.h"
#include "kbs_String.h"
#include "kbs_SuffixArray.h"
#include "kbs_SuffixArrayConstDStepAndPre.h"
#include "kbs_Time.h"
#include "kbs_Types.h"


#define INSSORT_LIMIT 15


__inline__ static Kbs_Ulong computeDiffDepthBucket_SaBucket(register Kbs_Ulong *const leftPtr, register Kbs_Ulong *const rightPtr, register const Kbs_Ulong offset, register const Kbs_Ulong q, register Kbs_Ulong **const sufPtrMap);

__inline__ static void computeBucketSize2_SaBucket(register Kbs_Ulong *const leftPtr, register Kbs_Ulong *const rightPtr, register const Kbs_Ulong offset, register const Kbs_Ulong q, register Kbs_Ulong **const sufPtrMap);
__inline__ static void computeBucketSize3_SaBucket(register Kbs_Ulong *const leftPtr, register Kbs_Ulong *const rightPtr, register const Kbs_Ulong offset, register const Kbs_Ulong q, register Kbs_Ulong **const sufPtrMap);

__inline__ static void insSortUpdateRecurse_SaBucket(register Kbs_Ulong *const leftPtr, Kbs_Ulong *const rightPtr, register const Kbs_Ulong offset, register const Kbs_Ulong q, register Kbs_Ulong **const sufPtrMap);
static void updatePtrAndRefineBuckets_SaBucket(Kbs_Ulong *leftPtr, Kbs_Ulong *rightPtr, const Kbs_Ulong offset, const Kbs_Ulong q, Kbs_Ulong **const sufPtrMap);

__inline__ static Kbs_Ulong medianOfThreeUlong(const Kbs_Ulong *const a, const Kbs_Ulong *const b, const Kbs_Ulong *const c);
__inline__ static void vectorSwap(Kbs_Ulong *const leftPtr, Kbs_Ulong *rightPtr, Kbs_Ulong *swapEndPtr);
static void partitionUpdateRecurse_SaBucket(Kbs_Ulong *const leftPtr, register Kbs_Ulong *const rightPtr, register const Kbs_Ulong offset, register const Kbs_Ulong q,  Kbs_Ulong **const sufPtrMap);

static Kbs_Ulong **determineAll_Buckets_Sarray(const Kbs_Ustring *const ustring, register const Kbs_Ulong q, Kbs_Ulong **suffixArrayPtr);
static Kbs_Ulong **determinePower2Alpha_Buckets_Sarray(const Kbs_Ustring *const ustring, register const Kbs_Ulong q, Kbs_Ulong **suffixArrayPtr);
static Kbs_Ulong **determineAll_Buckets_Sarray_Sptrmap(const Kbs_Ustring *const ustring, register const Kbs_Ulong q, Kbs_Ulong **suffixArrayPtr, Kbs_Ulong ***sufPtrMapPtr);
static Kbs_Ulong **determinePower2Alpha_Buckets_Sarray_Sptrmap(const Kbs_Ustring *const ustring, register const Kbs_Ulong q, Kbs_Ulong **suffixArrayPtr, Kbs_Ulong ***sufPtrMapPtr);
static Kbs_Ulong **determine_Buckets_Sarray_Sptrmap(const Kbs_Ustring *const ustring, register const Kbs_Ulong q, Kbs_Ulong **suffixArrayPtr, Kbs_Ulong ***sufPtrMapPtr);

static Kbs_Uchar *getCharWeightedOrder_Alphabet(const Kbs_Alphabet *const alphabet, const Kbs_Ulong **const buckets, const Kbs_Ulong bucketsInLevel2Bucket);
static Kbs_Uchar *getCharWeightedRank_Alphabet(const Kbs_Alphabet *const alphabet, const Kbs_Ulong **const buckets, const Kbs_Ulong *const suffixArray, const Kbs_Ulong q);




/*---------------------------------------------------------------------------------------*/
/**
 * Computes about the LCP of all suffixes in this bucket. It will be the newoffset.
 * @param leftPtr points to the leftmost suffix of the current bucket.
 * @param rightPtr points to the rightmost suffix of the current bucket.
 * @param offset is the length of the common prefix of the suffixes rounded down to a multiple of q.
 * @param q is the initial prefix length used for the bucket sort. It also determines the increase of offset.
 * @param sufPtrMap at position i points to the rightmost position of the bucket that contains suffix i (bptr).
 * @return the LCP of suffixes in this bucket (newoffset).
 */
__inline__ static Kbs_Ulong computeDiffDepthBucket_SaBucket(register Kbs_Ulong *const leftPtr,
                                    register Kbs_Ulong *const rightPtr,
                                    register const Kbs_Ulong offset,
                                    register const Kbs_Ulong q,
                                    register Kbs_Ulong **const sufPtrMap) {
  register Kbs_Ulong lcp = offset;
  while (1) {
    register Kbs_Ulong *runPtr = leftPtr;
    register Kbs_Ulong *const tmpPtr = sufPtrMap[*rightPtr + lcp];
    while(runPtr < rightPtr) {
      if(sufPtrMap[*runPtr+lcp] != tmpPtr) {
        return lcp;
      }
      runPtr++;
    }
    lcp +=q;
  }
}


/*---------------------------------------------------------------------------------------*/
/**
 * Completely sorts buckets of size 2.
 * @param leftPtr points to the leftmost suffix of the current bucket.
 * @param rightPtr points to the rightmost suffix of the current bucket.
 * @param offset is the length of the common prefix of the suffixes rounded down to a multiple of q.
 * @param q is the initial prefix length used for the bucket sort. It also determines the increase of offset.
 * @param sufPtrMap at position i points to the rightmost position of the bucket that contains suffix i (bptr).
 */
__inline__ static void computeBucketSize2_SaBucket(register Kbs_Ulong *const leftPtr,
                                    register Kbs_Ulong *const rightPtr,
                                    register const Kbs_Ulong offset,
                                    register const Kbs_Ulong q,
                                    register Kbs_Ulong **const sufPtrMap) {
  register Kbs_Ulong suffix1 = *leftPtr + offset;
  register Kbs_Ulong suffix2 = *rightPtr + offset;
  while (sufPtrMap[suffix1] == sufPtrMap[suffix2]) {
    suffix1 += q;
    suffix2 += q;
  }
  if (sufPtrMap[suffix1] > sufPtrMap[suffix2]) {
    register const Kbs_Ulong tmpSwap = *leftPtr;
    *leftPtr = *rightPtr;
    *rightPtr = tmpSwap;
  }
  sufPtrMap[*leftPtr] = leftPtr;
  sufPtrMap[*rightPtr] = rightPtr;
}

/*---------------------------------------------------------------------------------------*/
/**
 * Completely sorts buckets of size 3.
 * @param leftPtr points to the leftmost suffix of the current bucket.
 * @param rightPtr points to the rightmost suffix of the current bucket.
 * @param q is the initial prefix length used for the bucket sort. It also determines the increase of offset.
 * @param offset is the length of the common prefix of the suffixes rounded down to a multiple of q.
 * @param sufPtrMap at position i points to the rightmost position of the bucket that contains suffix i (bptr).
 */
__inline__ static void computeBucketSize3_SaBucket(register Kbs_Ulong *const leftPtr,
                                    register Kbs_Ulong *const rightPtr,
                                    register const Kbs_Ulong offset,
                                    register const Kbs_Ulong q,
                                    register Kbs_Ulong **const sufPtrMap) {
    Kbs_Ulong newOffset = offset;
    while (sufPtrMap[*leftPtr + newOffset] == sufPtrMap[*(leftPtr+1) + newOffset] && sufPtrMap[*(leftPtr+1) + newOffset] == sufPtrMap[*rightPtr + newOffset]) {
      newOffset += q;
    }
    if (sufPtrMap[*leftPtr + newOffset] > sufPtrMap[*rightPtr + newOffset]) {
      register const Kbs_Ulong swapTmp = *leftPtr;
      *leftPtr = *rightPtr;
      *rightPtr = swapTmp;
    }
    if (sufPtrMap[*leftPtr + newOffset] > sufPtrMap[*(leftPtr+1) + newOffset]) {
      register const Kbs_Ulong swapTmp = *leftPtr;
      *leftPtr = *(leftPtr+1);
      *(leftPtr+1) = swapTmp;
    }
    if (sufPtrMap[*(leftPtr+1) + newOffset] > sufPtrMap[*rightPtr + newOffset]) {
      register const Kbs_Ulong swapTmp = *rightPtr;
      *rightPtr = *(leftPtr+1);
      *(leftPtr+1) = swapTmp;
    }
    if (sufPtrMap[*leftPtr + newOffset] == sufPtrMap[*(leftPtr+1) + newOffset]) {
      register Kbs_Ulong suffix1 = *leftPtr + newOffset + q;
      register Kbs_Ulong suffix2 = *(leftPtr+1) + newOffset + q;
      while (sufPtrMap[suffix1] == sufPtrMap[suffix2]) {
        suffix1 += q;
        suffix2 += q;
      }
      if (sufPtrMap[suffix1] > sufPtrMap[suffix2]) {
        register const Kbs_Ulong tmpSwap = *leftPtr;
        *leftPtr = *(leftPtr+1);
        *(leftPtr+1) = tmpSwap;
      }
      sufPtrMap[*leftPtr] = leftPtr;
      sufPtrMap[*(leftPtr+1)] = leftPtr+1;
      sufPtrMap[*rightPtr] = rightPtr;
      return;
    }
    if (sufPtrMap[*(leftPtr+1) + newOffset] == sufPtrMap[*rightPtr + newOffset]) {
      sufPtrMap[*leftPtr] = leftPtr;
      register Kbs_Ulong suffix1 = *(leftPtr+1) + newOffset + q;
      register Kbs_Ulong suffix2 = *rightPtr + newOffset + q;
      while (sufPtrMap[suffix1] == sufPtrMap[suffix2]) {
        suffix1 += q;
        suffix2 += q;
      }
      if (sufPtrMap[suffix1] > sufPtrMap[suffix2]) {
        register const Kbs_Ulong tmpSwap = *rightPtr;
        *rightPtr = *(leftPtr+1);
        *(leftPtr+1) = tmpSwap;
      }
      sufPtrMap[*(leftPtr+1)] = leftPtr+1;
      sufPtrMap[*rightPtr] = rightPtr;
      return;
    }
    sufPtrMap[*leftPtr] = leftPtr;
    sufPtrMap[*(leftPtr+1)] = (leftPtr+1);
    sufPtrMap[*rightPtr] = rightPtr;
}



/*---------------------------------------------------------------------------------------*/
/**
 * Stably sorts a bucket at a refinement level regarding sort keys that are bucket pointers in sufPtrMap with offset.
 * @param leftPtr points to the leftmost suffix of the current bucket.
 * @param rightPtr points to the rightmost suffix of the current bucket.
 * @param offset is the length of the common prefix of the suffixes (a multiple of q).
 * @param q is the initial prefix length used for the bucket sort. It also determines the increase of offset.
 * @param sufPtrMap at position i points to the rightmost position of the bucket that contains suffix i (bptr).
 */
__inline__ static void insSortUpdateRecurse_SaBucket(register Kbs_Ulong *const leftPtr,
                                      Kbs_Ulong *const rightPtr,
                                      register const Kbs_Ulong offset,
                                      register const Kbs_Ulong q,
                                      register Kbs_Ulong **const sufPtrMap) {
  register Kbs_Ulong *rightTmpPtr = leftPtr+1;
  while(rightTmpPtr <= rightPtr) {
    register const Kbs_Ulong tempValue = *rightTmpPtr;
    register const Kbs_Ulong *const tempHashValue = sufPtrMap[(*rightTmpPtr)+offset];
    register Kbs_Ulong *leftTmpPtr = rightTmpPtr;
    while(leftTmpPtr > leftPtr && sufPtrMap[*(leftTmpPtr-1)+offset] > tempHashValue) {
      *leftTmpPtr = *(leftTmpPtr-1);
      leftTmpPtr--;
    }
    *leftTmpPtr = tempValue;
    rightTmpPtr++;
  }
  updatePtrAndRefineBuckets_SaBucket(leftPtr, rightPtr, offset, q, sufPtrMap);
  return;
}

/*---------------------------------------------------------------------------------------*/
/**
 * The function determines the subbuckets after refining this bucket
 * and recursively calls the refinement function for the subbuckets.
 * @param leftPtr points to the leftmost suffix of the current bucket.
 * @param rightPtr points to the rightmost suffix of the current bucket.
 * @param offset is the length of the common prefix of the suffixes (a multiple of q).
 * @param q is the initial prefix length used for the bucket sort. It also determines the increase of offset.
 * @param sufPtrMap at position i points to the rightmost position of the bucket that contains suffix i (bptr).
 */
static void updatePtrAndRefineBuckets_SaBucket(register Kbs_Ulong *const leftPtr,
                                    register Kbs_Ulong *const rightPtr,
                                    register const Kbs_Ulong offset,
                                    const Kbs_Ulong q,
                                    register Kbs_Ulong **const sufPtrMap) {

  /* for all buckets with resp. pointer > rightPtr determine buckets via setting sufPtrMap */
  register Kbs_Ulong *leftIntervalPtr = rightPtr;
  register Kbs_Ulong *rightIntervalPtr = rightPtr;
  register Kbs_Ulong *tmpPtr;
  while(leftPtr <= leftIntervalPtr && rightPtr < (tmpPtr = sufPtrMap[*leftIntervalPtr + offset] )) {
    do{
      sufPtrMap[*leftIntervalPtr] = rightIntervalPtr;
      leftIntervalPtr--;
    } while (leftPtr <= leftIntervalPtr
              && sufPtrMap[*leftIntervalPtr + offset] == tmpPtr);
    rightIntervalPtr = leftIntervalPtr;
  }

  /* since the sufPtrMap for the suffixes between leftPtr and rightPtr might change in previous 2 steps */
  /* determine the bucket concerning suffixptr+offset between leftPtr and rightPtr separately */
  rightIntervalPtr = leftIntervalPtr;
  while (leftPtr <= leftIntervalPtr
          && leftPtr <= sufPtrMap[*leftIntervalPtr + offset]
          && sufPtrMap[*leftIntervalPtr+offset] <= rightPtr) {
    sufPtrMap[*leftIntervalPtr] = rightIntervalPtr;
    leftIntervalPtr--;
  }

  /* for all buckets with resp. pointer+offset < leftPtr determine buckets via setting sufPtrMap */
  /* start with rightIntervalPtr which indicates leftend-1 of bucket with resp. pointer+offset between*/
  /* leftPtr and rightPtr*/
  Kbs_Ulong *const middleRightPtr = rightIntervalPtr;
  Kbs_Ulong *const middleLeftPtr = leftIntervalPtr;
  rightIntervalPtr = leftIntervalPtr;
  while(leftPtr <= leftIntervalPtr) {
    register const Kbs_Ulong *tmpPtr2 = sufPtrMap[*leftIntervalPtr + offset];
    do{
      sufPtrMap[*leftIntervalPtr] = rightIntervalPtr;
      leftIntervalPtr--;
    } while (leftPtr <= leftIntervalPtr
              && sufPtrMap[*leftIntervalPtr + offset] == tmpPtr2);
    rightIntervalPtr = leftIntervalPtr;
  }

  Kbs_Ulong newOffset = offset+q;
  if (sufPtrMap[*leftPtr]==rightPtr) {
    newOffset = computeDiffDepthBucket_SaBucket(leftPtr, rightPtr, newOffset, q, sufPtrMap);
  }
  register Kbs_Ulong *leftTmpPtr = leftPtr;
  while (leftTmpPtr < middleLeftPtr) {
    Kbs_Ulong *const rightTmpPtr = sufPtrMap[*leftTmpPtr];
    const Kbs_Ulong tmpLong = rightTmpPtr - leftTmpPtr;
    if (tmpLong > 0) {
      if (tmpLong == 1) {
        computeBucketSize2_SaBucket(leftTmpPtr, rightTmpPtr, newOffset, q, sufPtrMap);
        leftTmpPtr = rightTmpPtr + 1;
        continue;
      }
      if (tmpLong == 2) {
        computeBucketSize3_SaBucket(leftTmpPtr, rightTmpPtr, newOffset, q, sufPtrMap);
        leftTmpPtr = rightTmpPtr + 1;
        continue;
      }
      insSortUpdateRecurse_SaBucket(leftTmpPtr, rightTmpPtr, newOffset, q, sufPtrMap);
     }
    leftTmpPtr = rightTmpPtr + 1;
  }
  /* for buckets refering to this bucket, the offset can be doubled */
  if (middleRightPtr > middleLeftPtr+1) {
    if (middleRightPtr - middleLeftPtr == 2) {
      computeBucketSize2_SaBucket(middleLeftPtr+1, middleRightPtr, MAX(2*offset, newOffset), q, sufPtrMap);
    }
    else {
      if (middleRightPtr - middleLeftPtr == 3) {
        computeBucketSize3_SaBucket(middleLeftPtr+1, middleRightPtr, MAX(2*offset, newOffset), q, sufPtrMap);
      }
      else {
        insSortUpdateRecurse_SaBucket(middleLeftPtr+1, middleRightPtr, MAX(2*offset, newOffset), q, sufPtrMap);
      }
    }
  }
  leftTmpPtr = middleRightPtr + 1;
  while (leftTmpPtr < rightPtr) {
    Kbs_Ulong *const rightTmpPtr = sufPtrMap[*leftTmpPtr];
    const Kbs_Ulong tmpLong = rightTmpPtr - leftTmpPtr;
    if (tmpLong > 0) {
      if (tmpLong == 1) {
        computeBucketSize2_SaBucket(leftTmpPtr, rightTmpPtr, newOffset, q, sufPtrMap);
        leftTmpPtr = rightTmpPtr + 1;
        continue;
      }
      if (tmpLong == 2) {
        computeBucketSize3_SaBucket(leftTmpPtr, rightTmpPtr, newOffset, q, sufPtrMap);
        leftTmpPtr = rightTmpPtr + 1;
        continue;
      }
      insSortUpdateRecurse_SaBucket(leftTmpPtr, rightTmpPtr, newOffset, q, sufPtrMap);
    }
    leftTmpPtr = rightTmpPtr + 1;
  }
}



/*---------------------------------------------------------------------------------------*/
/**
 * @param a first key
 * @param b second key
 * @param c third key
 * @return 0 if a is the median, 1 if b is the median, 2 if c is the median.
 */
__inline__ static Kbs_Ulong medianOfThreeUlong(const Kbs_Ulong *const a, const Kbs_Ulong *const b, const Kbs_Ulong *const c) {
  if (a==b || a==c) {
    return 0;
  }
  if (b==c) {
    return 2;
  }
  return a<b ?(b<c ? 1 : (a<c ? 2 : 0)) : (b>c ? 1 : (a<c ? 0 : 2));
}

/*---------------------------------------------------------------------------------------*/
/**
 * @param leftPtr points to the leftmost suffix of the first swap space.
 * @param rightPtr points to the rightmost suffix of the first swap space.
 * @param swapEndPtr points to the leftmost suffix of the second swap space.
 */
__inline__ static void vectorSwap(Kbs_Ulong *const leftPtr, Kbs_Ulong *rightPtr, Kbs_Ulong *swapEndPtr) {
    register const Kbs_Ulong swapTmp = *swapEndPtr;
    while (leftPtr < rightPtr) {
      *swapEndPtr = *rightPtr;
      swapEndPtr--;
      *rightPtr = *swapEndPtr;
      rightPtr--;
    }
    *swapEndPtr = *leftPtr;
    *leftPtr = swapTmp;
}


/*---------------------------------------------------------------------------------------*/
/**
 * Ternary partitioning of buckets with Lomuto's scheme.
 * Subbuckets of size 2 and 3 are directly sorted and
 * partitions smaller than a given threshold are sorted by insertion sort.
 * @param leftPtr points to the leftmost position of the current bucket.
 * @param rightPtr points to the rightmost position of the current bucket.
 * @param offset is the length of the common prefix of the suffixes (a multiple of q).
 * @param q is the initial prefix length used for the bucket sort. It also determines the increase of offset.
 * @param sufPtrMap at position i points to the rightmost position of the bucket that contains suffix i (bptr).
 */
static void partitionUpdateRecurse_SaBucket(Kbs_Ulong *const leftPtr,
                                    register Kbs_Ulong *const rightPtr,
                                    register const Kbs_Ulong offset,
                                    register const Kbs_Ulong q,
                                    Kbs_Ulong **const sufPtrMap) {

  register Kbs_Ulong *pivot;
  Kbs_Ulong tmpSize = rightPtr-leftPtr;
  if (tmpSize < 10000) {
    tmpSize = tmpSize/4;
    pivot = sufPtrMap[*(leftPtr + tmpSize) + offset];
    register const Kbs_Ulong *const pivotb = sufPtrMap[*(leftPtr+ 2*tmpSize) + offset];
    register const Kbs_Ulong *const pivotc = sufPtrMap[*(rightPtr-tmpSize) + offset];
    register const Kbs_Int medNumber = medianOfThreeUlong(pivot, pivotb, pivotc);
    register Kbs_Ulong *pivotPtr = leftPtr + tmpSize;
    if (medNumber > 0){
      pivotPtr = (medNumber == 1) ? (leftPtr+ 2*tmpSize) : (rightPtr-tmpSize);
      pivot = (medNumber == 1) ? pivotb : pivotc;
    }
    register const Kbs_Ulong swapTmp = *pivotPtr;
    *pivotPtr = *leftPtr;
    *leftPtr = swapTmp;
  }
  else {
    Kbs_Ulong *keyPtrList[9];
    tmpSize = tmpSize/10;
    int i;
    for (i=0; i<9; i++) {
      keyPtrList[i] = leftPtr + (i+1)*tmpSize;
    }
    /* insertion sort */
    for (i=1; i<9; i++) {
      register const Kbs_Ulong *tempValue = keyPtrList[i];
      register const Kbs_Ulong *const tempHashValue = sufPtrMap[(*tempValue)+offset];
      int j = i-1;
      while(j >= 0 && sufPtrMap[*(keyPtrList[j])+offset] > tempHashValue) {
        keyPtrList[j+1] = keyPtrList[j];
        j--;
      }
      keyPtrList[j+1] = tempValue;
    }
    register const Kbs_Ulong swapTmp = *(keyPtrList[4]);
    *(keyPtrList[4]) = *leftPtr;
    *leftPtr = swapTmp;
    pivot = sufPtrMap[*leftPtr + offset];
  }

  register Kbs_Ulong *pivotRightPtr = leftPtr+1;
  while (pivotRightPtr <= rightPtr && sufPtrMap[*pivotRightPtr + offset] == pivot) {
    ++pivotRightPtr;
  }
  register Kbs_Ulong *smallerPivotPtr = pivotRightPtr;
  while (smallerPivotPtr <= rightPtr && sufPtrMap[*smallerPivotPtr + offset] < pivot) {
    smallerPivotPtr++;
  }

  register Kbs_Ulong *frontPtr = smallerPivotPtr-1;
  while (frontPtr++ < rightPtr) {
    register Kbs_Ulong *const sortkey = sufPtrMap[*frontPtr + offset];
    if (sortkey <= pivot) {
      register const Kbs_Ulong swapTmp = *frontPtr;
      *frontPtr = *smallerPivotPtr;
      *smallerPivotPtr = swapTmp;
      if (sortkey == pivot) {
        *smallerPivotPtr = *pivotRightPtr;
        *(pivotRightPtr++) = swapTmp;
      }
      smallerPivotPtr++;
    }
  }

  /* vector swap the pivot elements*/
  const Kbs_Ulong numberSmaller = smallerPivotPtr-pivotRightPtr;
  if (numberSmaller > 0) {
    register const Kbs_Ulong swapsize = MIN((Kbs_Ulong)(pivotRightPtr-leftPtr), numberSmaller);
    Kbs_Ulong *pivotRightTmpPtr = leftPtr + swapsize-1;
    vectorSwap(leftPtr, pivotRightTmpPtr, smallerPivotPtr-1);

   /* recursively sort < partition*/
    if (numberSmaller == 1) {
      sufPtrMap[*leftPtr] = leftPtr;
    }
    else {
      if (numberSmaller == 2) {
        computeBucketSize2_SaBucket(leftPtr, leftPtr+1, offset, q, sufPtrMap);
      }
      else {
        if (numberSmaller == 3)
          computeBucketSize3_SaBucket(leftPtr, leftPtr+2, offset, q, sufPtrMap);
        else
          partitionUpdateRecurse_SaBucket(leftPtr, leftPtr+numberSmaller-1, offset, q, sufPtrMap);
      }
    }
  }

  /* update pivots and recursively sort = partition*/
  Kbs_Ulong *leftTmpPtr = leftPtr+numberSmaller;
  smallerPivotPtr--;
  if (leftTmpPtr == smallerPivotPtr) {
    sufPtrMap[*leftTmpPtr] = leftTmpPtr;
    if (leftTmpPtr == rightPtr) return;
  }
  else {
    Kbs_Ulong newOffset = (pivot == rightPtr)? (2*offset) : offset+q;
    if (leftTmpPtr+1 == smallerPivotPtr) { 
      computeBucketSize2_SaBucket(leftTmpPtr, smallerPivotPtr, newOffset, q, sufPtrMap);
      if (rightPtr == smallerPivotPtr) return;
    }
    else {
      if (leftTmpPtr+2 == smallerPivotPtr) { 
        computeBucketSize3_SaBucket(leftTmpPtr, smallerPivotPtr, newOffset, q, sufPtrMap);
        if (rightPtr == smallerPivotPtr) return;
      }
      else {
        if (rightPtr == smallerPivotPtr) {
          newOffset = computeDiffDepthBucket_SaBucket(leftPtr+numberSmaller, rightPtr, newOffset, q, sufPtrMap);
          partitionUpdateRecurse_SaBucket(leftTmpPtr, rightPtr, newOffset, q, sufPtrMap);
          return;
        }
        while(leftTmpPtr <= smallerPivotPtr) {
          sufPtrMap[*leftTmpPtr] = smallerPivotPtr;
          leftTmpPtr++;
        }
        if (smallerPivotPtr < leftPtr+numberSmaller + INSSORT_LIMIT) {
          insSortUpdateRecurse_SaBucket(leftPtr+numberSmaller, smallerPivotPtr, newOffset, q, sufPtrMap);
        }
        else
          partitionUpdateRecurse_SaBucket(leftPtr+numberSmaller, smallerPivotPtr, newOffset, q, sufPtrMap);
      }
    }
  }

  /* recursively sort > partition*/
  ++smallerPivotPtr;
  if (smallerPivotPtr == rightPtr) {
    sufPtrMap[*rightPtr] = rightPtr;
    return;
  }
  if (smallerPivotPtr+1 == rightPtr) {
    computeBucketSize2_SaBucket(smallerPivotPtr, rightPtr, offset, q, sufPtrMap);
    return;
  }
  if (smallerPivotPtr+2 == rightPtr) {
    computeBucketSize3_SaBucket(smallerPivotPtr, rightPtr, offset, q, sufPtrMap);
    return;
  }
  partitionUpdateRecurse_SaBucket(smallerPivotPtr, rightPtr, offset, q, sufPtrMap);
}











/*---------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/




/**
 * Determines the maximal q-gram size for the given alphabet size.
 * @param alphabetSize size of the alphabet.
 * @return Maximal q-gram size.
 */
__inline__ static Kbs_Ulong determineMaxD(const Kbs_Ulong alphabetSize) {
  if (alphabetSize == 1) {
    return 16;
  }
  Kbs_Ulong numberHashValues = 1;
  Kbs_Ulong q = 0;
  const Kbs_Ulong bound = KBS_ULONG_MAX / (alphabetSize * sizeof(Kbs_Ulong));
  while (numberHashValues <= bound){
    numberHashValues *= alphabetSize;
    q++;
  }
  return MIN(q,KBS_STRING_EXTENSION_SIZE);
}


/**
 * Constructs all buckets w.r.t. q-gram size and the up to prefix q sorted suffix array.
 * Call determine_Buckets_Sarray(const Kbs_Ustring *const ustring, register const Kbs_Ulong q, Kbs_Ulong **suffixArrayPtr)
 * instead
 * @param ustring        of characters that are inline mapped to 0..alphabetsize-1.
 * @param q              size of q-gram.
 * @param suffixArrayPtr ptr that will be filled with the suffix array.
 * @return Buckets containing pointers into the suffix array.
 * @see determine_Buckets_Sarray
 */
static Kbs_Ulong **determineAll_Buckets_Sarray(const Kbs_Ustring *const ustring,
                                          register const Kbs_Ulong q,
                                          Kbs_Ulong **suffixArrayPtr) {

  union bucket{
    Kbs_Ulong *bucketPtr;
    Kbs_Ulong bucketSize;
  };

  register const Kbs_Ulong strLen = ustring->strLength;
  register const Kbs_Ulong alphabetSize = ustring->alphabet->alphaSize;
  register const Kbs_Ulong numberBuckets = kbs_power_Ulong(alphabetSize, q);
  register union bucket *const buckets  = (union bucket *)malloc(sizeof(union bucket) * (numberBuckets+1));
  memset(buckets, 0, sizeof(union bucket) * (numberBuckets+1));

  memset(ustring->str + strLen, ustring->alphabet->charArray[0], sizeof(Kbs_Uchar)*q);
  memset(ustring->str + strLen + q, '\0', sizeof(Kbs_Uchar)*(KBS_STRING_EXTENSION_SIZE -q));

  /* computation of first hashvalue */
  Kbs_Int *alphaMap = ustring->alphabet->alphaMapping;
  register Kbs_Uchar *mappedUcharArray = ustring->str;
  register Kbs_Ulong hashCode = 0;
  Kbs_Ulong tempPower = 1;
  register Kbs_Long i;
  for (i=q-1; i>=0; i--) {
    hashCode += (mappedUcharArray[i] = alphaMap[mappedUcharArray[i]]) * tempPower;
    tempPower *= alphabetSize;
  }
  const Kbs_Ulong firstHashCode = hashCode;

  /* computation of the size of buckets */
  register const Kbs_Ulong tempModulo = kbs_power_Ulong(alphabetSize, q-1);
  mappedUcharArray += q;
  buckets[hashCode].bucketSize++;
  register Kbs_Ulong j;
  for (j=1; j<strLen; j++) {
    hashCode -= (*(mappedUcharArray-q)) * tempModulo;
    hashCode *= alphabetSize;
    hashCode += *mappedUcharArray = alphaMap[*mappedUcharArray];
    mappedUcharArray++;
    buckets[hashCode].bucketSize++;
  }
  /* update the alphabet for mapped string */
  for(j=0; j<alphabetSize; j++) {
    ustring->alphabet->charFreq[j] = ustring->alphabet->charFreq[ustring->alphabet->charArray[j]];
    ustring->alphabet->charArray[j] = (Kbs_Uchar) j;
    alphaMap[j] = j;
  }
  for(; j<KBS_MAX_ALPHABET_SIZE; j++) {
    alphaMap[j] = (Kbs_Int) (-1);
  }




  Kbs_Ulong *const suffixArray = (Kbs_Ulong *)malloc(sizeof(Kbs_Ulong) * strLen);
  if (suffixArray == NULL) {
    KBS_ERROR(KBS_ERROR_MALLOC);
  }
  /* computation of the bucket pointers, pointers into the suffix array */
  buckets[0].bucketPtr = suffixArray + buckets[0].bucketSize;
  for (j=1; j<=numberBuckets; j++){
    buckets[j].bucketPtr = buckets[j-1].bucketPtr + buckets[j].bucketSize;
  }

 /* computation of the suffix array (buckets that are copied later are left out)*/
  const Kbs_Uchar *const charRank = getCharWeightedRank_Alphabet(ustring->alphabet, (const Kbs_Ulong **)buckets, (Kbs_Ulong *)suffixArray, q);
  mappedUcharArray = ustring->str + q;
  hashCode = firstHashCode ;
  for (j=0; j<strLen-1; j++) {
    register Kbs_Uchar c1;
    buckets[hashCode].bucketPtr--;
    if ( (c1 = charRank[*(mappedUcharArray-q)]) < charRank[*(mappedUcharArray+1-q)]  &&  c1 <= charRank[*(mappedUcharArray+2-q)])
      *(buckets[hashCode].bucketPtr) = j;
    hashCode -= (*(mappedUcharArray-q)) * tempModulo;
    hashCode *= alphabetSize;
    hashCode += *mappedUcharArray;
    mappedUcharArray++;
  }
  free(charRank);
  buckets[hashCode].bucketPtr--;
  *(buckets[hashCode].bucketPtr) = strLen-1;

  buckets[numberBuckets].bucketPtr = suffixArray + strLen;
  *suffixArrayPtr = suffixArray;
  return (Kbs_Ulong **)buckets;
}


/**
 * Constructs all buckets w.r.t. q-gram size q and the up to prefix q sorted suffix array.
 * Precondition: ustring->alphabet->alphaSize = 2^x for some x;
 * otherwise, call determine_Buckets_Sarray.
 * @param ustring        of characters that are inline mapped to 0..alphabetsize-1.
 * @param q              size of q-gram.
 * @param suffixArrayPtr ptr that will be filled with the suffix array.
 * @return Buckets containing pointers into the suffix array.
 * @see determine_Buckets_Sarray
 */
static Kbs_Ulong **determinePower2Alpha_Buckets_Sarray(const Kbs_Ustring *const ustring,
                                          register const Kbs_Ulong q,
                                          Kbs_Ulong **suffixArrayPtr) {

  union bucket{
    Kbs_Ulong *bucketPtr;
    Kbs_Ulong bucketSize;
  };
  register const Kbs_Int exp2 = (Kbs_Int) kbs_getExp_Ulong(2, ustring->alphabet->alphaSize);
  if (exp2 < 0) {
    KBS_ERROR(KBS_ERROR_VALUEOUTOFBOUNDS);
    return NULL;
  }
  register Kbs_Ulong strLen = ustring->strLength;

  /* initialization of string mapping */
  memset(ustring->str + strLen, ustring->alphabet->charArray[0], sizeof(Kbs_Uchar)*q);
  memset(ustring->str + strLen + q, '\0', sizeof(Kbs_Uchar)*(KBS_STRING_EXTENSION_SIZE -q));

  /* initialization of bucket table */
  register const Kbs_Ulong numberBuckets = kbs_power_Ulong(ustring->alphabet->alphaSize, q);
  register union bucket *const buckets = (union bucket *)malloc(sizeof(union bucket) * (numberBuckets+1));
  memset(buckets, '\0', sizeof(union bucket) * (numberBuckets+1));

  /* computation of first hashvalue */
  Kbs_Int *alphaMap = ustring->alphabet->alphaMapping;
  register Kbs_Uchar *mappedUcharArray = ustring->str;
  register Kbs_Ulong j;
  register Kbs_Ulong hashCode = 0;
  for (j=0; j<q; j++) {
    hashCode = hashCode << exp2;
    hashCode += (mappedUcharArray[j] = alphaMap[mappedUcharArray[j]]);
  }
  const Kbs_Ulong firstHashCode = hashCode;

  /* computation of the filling for the buckets */
  register Kbs_Ulong tempModulo = (Kbs_Ulong)0;
  tempModulo = ~tempModulo;
  tempModulo = tempModulo << (exp2*(q-1));
  tempModulo = ~tempModulo;
  mappedUcharArray += q;
  buckets[hashCode].bucketSize++;
  for (j=1; j<strLen; j++) {
    hashCode = hashCode & tempModulo;
    hashCode = hashCode << exp2;
    hashCode = hashCode | (Kbs_Ulong)(*mappedUcharArray = alphaMap[*mappedUcharArray]);
    mappedUcharArray++;
    buckets[hashCode].bucketSize++;
  }
  /* update the alphabet for mapped string */
  for(j=0; j<ustring->alphabet->alphaSize; j++) {
    ustring->alphabet->charFreq[j] = ustring->alphabet->charFreq[ustring->alphabet->charArray[j]];
    ustring->alphabet->charArray[j] = (Kbs_Uchar) j;
    alphaMap[j] = j;
  }
  for(; j<KBS_MAX_ALPHABET_SIZE; j++) {
    alphaMap[j] = (Kbs_Int) (-1);
  }

  Kbs_Ulong *const suffixArray = (Kbs_Ulong *)malloc(sizeof(Kbs_Ulong) * strLen);
  if (suffixArray == NULL) {
    KBS_ERROR(KBS_ERROR_MALLOC);
  }
  /* computation of the bucket pointers, pointers into the suffix array */
  buckets[0].bucketPtr = suffixArray + buckets[0].bucketSize;
  for (j=1; j<=numberBuckets; j++){
    buckets[j].bucketPtr = buckets[j-1].bucketPtr + buckets[j].bucketSize;
  }

 /* computation of the suffix array */
  const Kbs_Uchar *const charRank = getCharWeightedRank_Alphabet(ustring->alphabet, (const Kbs_Ulong **)buckets, suffixArray, q);
  mappedUcharArray = ustring->str + q;
  hashCode = firstHashCode ;
  for (j=0; j<strLen-1; j++) {
    register Kbs_Uchar c1;
    buckets[hashCode].bucketPtr--;
    if ((c1 = charRank[*(mappedUcharArray-q)]) < charRank[*(mappedUcharArray+1-q)] && (c1 <= charRank[*(mappedUcharArray+2-q)]))
      *(buckets[hashCode].bucketPtr) = j;
    hashCode = hashCode & tempModulo;
    hashCode = hashCode << exp2;
    hashCode = hashCode | (Kbs_Ulong)(*mappedUcharArray);
    mappedUcharArray++;
  }
  free(charRank);
  buckets[hashCode].bucketPtr--;
  *(buckets[hashCode].bucketPtr) = strLen-1;

  buckets[numberBuckets].bucketPtr = suffixArray+strLen;
  *suffixArrayPtr = suffixArray;
  return (Kbs_Ulong **)buckets;
}



/**
 * Constructs all buckets w.r.t. q-gram size q,
 * the up to prefix q sorted suffix array, and
 * the bucket-pointer table.
 * @param ustring        of characters that are inline mapped to 0..alphabetsize-1.
 * @param q              size of q-gram.
 * @param suffixArrayPtr ptr that will be filled with the suffix array.
 * @param sufPtrMapPtr   ptr that will be filled with the bucket-pointer table.
 * @return Buckets containing pointers into the suffix array.
 * @see determine_Buckets_Sarray_Sptrmap
 */
static Kbs_Ulong **determineAll_Buckets_Sarray_Sptrmap(const Kbs_Ustring *const ustring,
                                          register const Kbs_Ulong q,
                                          Kbs_Ulong **suffixArrayPtr,
                                          Kbs_Ulong ***sufPtrMapPtr) {

  register Kbs_Ulong **const buckets = determineAll_Buckets_Sarray(ustring, q, suffixArrayPtr);
  register Kbs_Ulong strLen = ustring->strLength;
  register Kbs_Ulong **const sufPtrMap = (Kbs_Ulong **)malloc(sizeof(Kbs_Ulong *) * (strLen+2*q+1));
  if (sufPtrMap == NULL) {
    KBS_ERROR(KBS_ERROR_MALLOC);
  }

  /* computation of first hashvalue */
  register const Kbs_Ulong alphabetSize = ustring->alphabet->alphaSize;
  register Kbs_Uchar *mappedUcharArray = ustring->str;
  register Kbs_Ulong tempPower = 1;
  register Kbs_Ulong hashCode = 0;
  register Kbs_Long i;
  for (i=q-1; i>=0; i--) {
    hashCode += mappedUcharArray[i] * tempPower;
    tempPower *= alphabetSize;
  }
  register const Kbs_Ulong tempModulo = kbs_power_Ulong(alphabetSize, q-1);
  mappedUcharArray += q;
  register Kbs_Ulong j;
  for (j=0; j<strLen-1; j++) {
    sufPtrMap[j] = (buckets[hashCode+1]) - 1;
    hashCode -= (*(mappedUcharArray-q)) * tempModulo;
    hashCode *= alphabetSize;
    hashCode += *mappedUcharArray;
    mappedUcharArray++;
  }
  sufPtrMap[j] = buckets[hashCode];
  /* set the values in sufPtrMap[strLen..strLen+2*d] to [-1, -2, ..., -2*d]*/
  Kbs_Ulong *beginPtr = *suffixArrayPtr - 1;
  for (j=strLen; j<=strLen+2*q; j++) {
    sufPtrMap[j] = beginPtr--;
  }
  *sufPtrMapPtr = sufPtrMap;
  return buckets;
}


/**
 * Constructs all buckets w.r.t. q-gram size q,
 * the up to prefix length q sorted suffix array, and
 * the bucket-pointer table.
 * @param ustring        of characters that are inline mapped to 0..alphabetsize-1.
 * @param q              size of q-gram.
 * @param suffixArrayPtr ptr that will be filled with the suffix array.
 * @param sufPtrMapPtr   ptr that will be filled with the bucket-pointer table.
 * @return Buckets containing pointers into the suffix array.
 * @see determine_Buckets_Sarray_Sptrmap
 */
static Kbs_Ulong **determinePower2Alpha_Buckets_Sarray_Sptrmap(const Kbs_Ustring *const ustring,
                                          register const Kbs_Ulong q,
                                          Kbs_Ulong **suffixArrayPtr,
                                          Kbs_Ulong ***sufPtrMapPtr) {

  register Kbs_Ulong strLen = ustring->strLength;
  const Kbs_Int exp2 = (Kbs_Int) kbs_getExp_Ulong(2, ustring->alphabet->alphaSize);
  if (exp2 < 0) {
    KBS_ERROR(KBS_ERROR_VALUEOUTOFBOUNDS);
    return NULL;
  }

  /* initialization of bucket table */
  register Kbs_Ulong **const buckets = determinePower2Alpha_Buckets_Sarray(ustring, q, suffixArrayPtr);
  register Kbs_Ulong **const sufPtrMap = (Kbs_Ulong **)malloc(sizeof(Kbs_Ulong *) * (strLen+2*q+1));
  if (sufPtrMap == NULL) {
    KBS_ERROR(KBS_ERROR_MALLOC);
  }

  /* computation of first hashvalue */
  register Kbs_Uchar *mappedUcharArray = ustring->str;
  register Kbs_Ulong hashCode = 0;
  register Kbs_Ulong j;
  for (j=0; j<q; j++) {
    hashCode = hashCode << exp2;
    hashCode += mappedUcharArray[j];
  }
  register Kbs_Ulong tempModulo = (Kbs_Ulong)0;
  tempModulo = ~tempModulo;
  tempModulo = tempModulo << (exp2*(q-1));
  tempModulo = ~tempModulo;
  mappedUcharArray += q;
  for (j=0; j<strLen-1; j++) {
    sufPtrMap[j] = (buckets[hashCode+1]) - 1;
    hashCode = hashCode & tempModulo;
    hashCode = hashCode << exp2;
    hashCode = hashCode | (Kbs_Ulong)(*mappedUcharArray);
    mappedUcharArray++;
  }
  sufPtrMap[j] = buckets[hashCode];
  Kbs_Ulong *beginPtr = *suffixArrayPtr - 1;
  for (j=strLen; j<=strLen+2*q; j++) {
    sufPtrMap[j] = beginPtr--;
  }
  *sufPtrMapPtr = sufPtrMap;
  return buckets;
}



/**
 * Constructs all buckets w.r.t. q-gram size q,
 * the up to prefix q sorted suffix array, and
 * the bucket-pointer table.
 * @param ustring        of characters that are inline mapped to 0..alphabetsize-1.
 * @param q              size of q-gram.
 * @param suffixArrayPtr ptr that will be filled with the suffix array.
 * @param sufPtrMapPtr   ptr that will be filled with the bucket-pointer table.
 * @return Buckets containing pointers into the suffix array.
 */
static Kbs_Ulong **determine_Buckets_Sarray_Sptrmap(const Kbs_Ustring *const ustring,
                                          register const Kbs_Ulong q,
                                          Kbs_Ulong **suffixArrayPtr,
                                          Kbs_Ulong ***sufPtrMapPtr) {
  if (kbs_getExp_Ulong(2, ustring->alphabet->alphaSize) >= 0) {
    return determinePower2Alpha_Buckets_Sarray_Sptrmap(ustring, q, suffixArrayPtr, sufPtrMapPtr);
  }
  else {
    return determineAll_Buckets_Sarray_Sptrmap(ustring, q, suffixArrayPtr, sufPtrMapPtr);
  }
}






/*--------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/
/**
 * Sorts the alphabet concerning some weight concerning cc bucket size and alphabet frequency
 * Only works for mapped string with alphabet [0,alphaSize]
 * @param alphabet - the alphabet of the string
 * @param buckets - the bucket table
 * @param bucketsInLevel2Bucket - number of subbuckets of level-2 buckets
 * @return the order of the alphabet according to the weight on buckets with same first and second character
 */
static Kbs_Uchar *getCharWeightedOrder_Alphabet(const Kbs_Alphabet *const alphabet,
                                                      const Kbs_Ulong **const buckets,
                                                      const Kbs_Ulong bucketsInLevel2Bucket){

  register const Kbs_Ulong alphabetSize = alphabet->alphaSize;
  Kbs_Ulong charWeight[alphabetSize];
  register const Kbs_Ulong tmpBucketFactor = bucketsInLevel2Bucket * (alphabetSize + 1);
  register Kbs_Ulong i;
  for (i=0; i<alphabetSize; i++) {
    charWeight[i] = (Kbs_Ulong)alphabet->charFreq[i];
    charWeight[i] -= buckets[i * tmpBucketFactor + bucketsInLevel2Bucket] - buckets[i * tmpBucketFactor];
  }

  register Kbs_Uchar *targetCharArray = (Kbs_Uchar *) malloc((alphabetSize + 1) * sizeof(Kbs_Uchar));
  for (i=0; i < alphabetSize; i++) {
    targetCharArray[i] = (Kbs_Uchar)i;
  }
  for (i=1; i < alphabet->alphaSize; i++) {
    register const Kbs_Ulong tmpWeight = charWeight[i];
    register Kbs_Long j = i;
    while(j>0 && tmpWeight < charWeight[targetCharArray[j-1]])  {
      targetCharArray[j] = targetCharArray[j-1];
      j--;
    }
    targetCharArray[j] = (Kbs_Uchar)i;
  }
  return targetCharArray;
}

/*--------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/
/**
 * Sorts the alphabet regarding some weight according to cc bucket size and alphabet frequency
 * Only works for mapped string with alphabet [0,alphaSize]
 * @param alphabet - the alphabet of the string
 * @param buckets - the bucket table
 * @param suffixArray - a pointer to the suffix array
 * @param q - the initial q-gram size
 * @return the rank of each character
 */
static Kbs_Uchar *getCharWeightedRank_Alphabet(const Kbs_Alphabet *const alphabet,
  const Kbs_Ulong **const buckets,
  const Kbs_Ulong *const suffixArray,
  const Kbs_Ulong q){
  
  register const Kbs_Ulong alphabetSize = alphabet->alphaSize;
  Kbs_Ulong charWeight[alphabetSize];
  register const Kbs_Ulong bucketsInLevel2Bucket = kbs_power_Ulong(alphabetSize, q-2);
  register const Kbs_Ulong tmpBucketFactor = bucketsInLevel2Bucket * (alphabetSize + 1);
  register Kbs_Ulong i;
  charWeight[0] = (Kbs_Ulong)alphabet->charFreq[0];
  charWeight[0] -= buckets[bucketsInLevel2Bucket - 1] - suffixArray;
  for (i=1; i<alphabetSize-1; i++) {
    charWeight[i] = (Kbs_Ulong)alphabet->charFreq[i];
    charWeight[i] -= buckets[i * tmpBucketFactor + bucketsInLevel2Bucket - 1] - buckets[i * tmpBucketFactor - 1];
  }
  charWeight[alphabetSize-1] = (Kbs_Ulong)alphabet->charFreq[i];
  charWeight[alphabetSize-1] -= buckets[(alphabetSize-1) * tmpBucketFactor + bucketsInLevel2Bucket - 1] - buckets[(alphabetSize-1) * tmpBucketFactor - 1];
  
  Kbs_Uchar targetCharArray[alphabetSize];
  for (i=0; i < alphabetSize; i++) {
    targetCharArray[i] = (Kbs_Uchar)i;
  }
  /* insertion sort by charWeight*/
  for (i=1; i < alphabet->alphaSize; i++) {
    register const Kbs_Ulong tmpWeight = charWeight[i];
    register Kbs_Long j = i;
    while(j>0 && tmpWeight < charWeight[targetCharArray[j-1]])  {
      targetCharArray[j] = targetCharArray[j-1];
      j--;
    }
    targetCharArray[j] = (Kbs_Uchar)i;
  }
  register Kbs_Uchar *charRank = (Kbs_Uchar *) malloc((alphabetSize + 1) * sizeof(Kbs_Uchar));
  for (i=0; i < alphabetSize; i++) {
    charRank[targetCharArray[i]] = i;
  }
  return charRank;
}


/*---------------------------------------------------------------------------------------*/
Kbs_SuffixArray *kbs_buildDstepUsePrePlusCopyFreqOrder_SuffixArray(Kbs_Ustring *const seq, register const Kbs_Ulong q) {
  if (seq == NULL) {
    KBS_ERROR(KBS_ERROR_NULLPOINTER);
    return NULL;
  }
  if (seq->alphabet == NULL) {
    KBS_ERROR(KBS_ERROR_NULLPOINTER);
    return NULL;
  }
  if (q < 3){
    printf("d=%lu is smaller than minimum 3, give larger d\n", (unsigned long) q);
    KBS_ERROR(KBS_ERROR_VALUEOUTOFBOUNDS);
    return NULL;
  }
  if (q > sizeof(Kbs_Ulong)*8){
    Kbs_Ulong wordlength = sizeof(Kbs_Ulong)*8;
    printf("q=%lu higher than word length %lu, give smaller q\n", (unsigned long) q, (unsigned long) wordlength);
    KBS_ERROR(KBS_ERROR_VALUEOUTOFBOUNDS);
    return NULL;
  }
  if ( q > determineMaxD(seq->alphabet->alphaSize) ){
    printf("q-gram size = %lu, results in to many buckets\n", q);
    KBS_ERROR(KBS_ERROR_VALUEOUTOFBOUNDS);
    return NULL;
  }

  /* computation of bucket table */
  Kbs_Ulong *suffixArray;
  Kbs_Ulong **sufPtrMap;
  const Kbs_Ulong **const buckets = (const Kbs_Ulong **)determine_Buckets_Sarray_Sptrmap(seq, q, &suffixArray, &sufPtrMap);
  if (buckets == NULL) {
    KBS_ERROR(KBS_ERROR_NULLPOINTER);
    return NULL;
  }

 /* Sorting of all buckets */
  register const Kbs_Uchar *const mappedCharPtr = seq->str;
  const Kbs_Ulong alphabetSize = seq->alphabet->alphaSize;
  const Kbs_Ulong bucketsInLevel3Bucket = kbs_power_Ulong(alphabetSize, q-3);
  const Kbs_Ulong bucketsInLevel2Bucket = bucketsInLevel3Bucket * alphabetSize;
  const Kbs_Ulong bucketsInLevel1Bucket = bucketsInLevel2Bucket * alphabetSize;
  const Kbs_Uchar *const alphaOrder = getCharWeightedOrder_Alphabet(seq->alphabet, buckets, bucketsInLevel2Bucket);
  Kbs_Uchar *const isNotSortedLevel1Char = (Kbs_Uchar *) malloc(sizeof(Kbs_Uchar) * alphabetSize);
  memset(isNotSortedLevel1Char, '\1', sizeof(Kbs_Uchar) * alphabetSize);

  /* Sort all level-1 buckets */
  Kbs_Ulong *leftPtrList[alphabetSize];    /* just for the copy phase maintains all pointers to the right of the small buckets */
  Kbs_Ulong *rightPtrList[alphabetSize];    /* just for the copy phase maintains all pointers to the right of the small buckets */
  Kbs_Ulong i;
  for (i=0; i < alphabetSize; i++) {
    register const Kbs_Ulong c1 = alphaOrder[i];
    /* sort buckets cd to cz */
    register Kbs_Ulong j;
    for (j=i+1; j < alphabetSize; j++) {
      register const Kbs_Ulong c2 = alphaOrder[j];
      register Kbs_Ulong l;
      for (l=i; l < alphabetSize; l++) {
        register const Kbs_Ulong c3 = alphaOrder[l];
        register const Kbs_Ulong tmpUlong = c1 * bucketsInLevel1Bucket + c2 * bucketsInLevel2Bucket + c3 * bucketsInLevel3Bucket;
        register Kbs_Ulong k;
        for (k = tmpUlong; k < tmpUlong + bucketsInLevel3Bucket; k++) {
          register Kbs_Ulong *const leftPtr = buckets[k];
          register Kbs_Ulong *const rightPtr = buckets[k+1] - 1;
          if (rightPtr - leftPtr > 0) {
            if (rightPtr - leftPtr < INSSORT_LIMIT)
              insSortUpdateRecurse_SaBucket(leftPtr, rightPtr, q, q, sufPtrMap);
            else
              partitionUpdateRecurse_SaBucket(leftPtr, rightPtr, q, q, sufPtrMap);
          }
        }
      }
    }

    Kbs_Ulong *leftPtrList2[alphabetSize * alphabetSize];    /* just for the copy phase maintains all pointers to the right of the small buckets */
    Kbs_Ulong *rightPtrList2[alphabetSize * alphabetSize];   /* just for the copy phase maintains all pointers to the right of the small buckets */


    /* copy left buckets of cx */
    for (j=i; j<alphabetSize; j++) {
      register const Kbs_Ulong cp1 = alphaOrder[j];
      leftPtrList[cp1] = buckets[cp1 * bucketsInLevel1Bucket + c1 * bucketsInLevel2Bucket];
      register Kbs_Ulong k;
      for (k=i+1; k < alphabetSize; k++) {
        register const Kbs_Ulong cp2 = alphaOrder[k];
        leftPtrList2[cp2 * alphabetSize + cp1] = buckets[cp2 * bucketsInLevel1Bucket + cp1 * bucketsInLevel2Bucket + c1 * bucketsInLevel3Bucket];
      }
    }
    if (c1 == 0) {
      register Kbs_Ulong cp1 = mappedCharPtr[seq->strLength-1];
      register Kbs_Ulong cp2 = mappedCharPtr[seq->strLength-2];
      if (isNotSortedLevel1Char[cp1]) {
        leftPtrList[cp1]++;
        *leftPtrList2[cp1 * alphabetSize]++;
        if (isNotSortedLevel1Char[cp2] && cp2 != c1) {
          *leftPtrList2[cp2 * alphabetSize + cp1] = seq->strLength-2;
          sufPtrMap[seq->strLength-2] = leftPtrList2[cp2 * alphabetSize + cp1];
          *leftPtrList2[cp2 * alphabetSize + cp1]++;
        }
      }
    }
    register Kbs_Ulong *leftPtr = buckets[c1 * bucketsInLevel1Bucket];
    while(leftPtr < leftPtrList[c1]) {
      register Kbs_Ulong cp1;
      register const Kbs_Ulong tmpUlong = *leftPtr;
      if (tmpUlong && isNotSortedLevel1Char[cp1 = mappedCharPtr[tmpUlong-1]]) {
        if (isNotSortedLevel1Char[mappedCharPtr[tmpUlong+1]]) {
          register Kbs_Ulong *const tmpUlongPtr = leftPtrList[cp1];
          sufPtrMap[tmpUlong-1] = tmpUlongPtr;
          *tmpUlongPtr = tmpUlong-1;
        }
        leftPtrList[cp1]++;
        register Kbs_Ulong cp2;
        if (tmpUlong > 1 && isNotSortedLevel1Char[cp2 = mappedCharPtr[tmpUlong-2]] && cp2 != c1)  {
          register Kbs_Ulong *const tmpUlongPtr = leftPtrList2[cp2*alphabetSize + cp1]++;
          sufPtrMap[tmpUlong-2] = tmpUlongPtr;
          *tmpUlongPtr = tmpUlong-2;
        }
      }
      leftPtr++;
    }

    /* copy right buckets of cx */
    for (j=i; j<alphabetSize; j++) {
      register const Kbs_Ulong cp1 = alphaOrder[j];
      register Kbs_Ulong k;
      rightPtrList[cp1] = buckets[cp1 * bucketsInLevel1Bucket + (c1+1) * bucketsInLevel2Bucket];
      for (k=i+1; k < alphabetSize; k++) {
        register const Kbs_Ulong cp2 = alphaOrder[k];
        rightPtrList2[cp2 * alphabetSize + cp1] = buckets[cp2 * bucketsInLevel1Bucket + cp1 * bucketsInLevel2Bucket + (c1+1) * bucketsInLevel3Bucket];
      }
    }
    register Kbs_Ulong *rightPtr = buckets[(c1+1)*bucketsInLevel1Bucket];
    while(leftPtr < rightPtr) {
      register Kbs_Ulong cp1;
      rightPtr--;
      register const Kbs_Ulong tmpUlong = *rightPtr;
      if (tmpUlong  && isNotSortedLevel1Char[cp1 = mappedCharPtr[tmpUlong-1]]) {
        rightPtrList[cp1]--;
        if (isNotSortedLevel1Char[mappedCharPtr[tmpUlong+1]]) {
          register Kbs_Ulong *const tmpUlongPtr = rightPtrList[cp1];
          sufPtrMap[tmpUlong-1] = tmpUlongPtr;
          *tmpUlongPtr = tmpUlong-1;
        }
        register Kbs_Ulong cp2;
        if (tmpUlong > 1 && isNotSortedLevel1Char[cp2 = mappedCharPtr[tmpUlong-2]] && cp2 != c1)  {
          register Kbs_Ulong *const tmpUlongPtr = --rightPtrList2[cp2*alphabetSize + cp1];
          sufPtrMap[tmpUlong-2] = tmpUlongPtr;
          *tmpUlongPtr = tmpUlong-2;
        }
      }
    }
    isNotSortedLevel1Char[c1] = 0;
  }
/* End Sorting of all buckets */

  free(alphaOrder);
  free(isNotSortedLevel1Char);
  Kbs_SuffixArray *const outputSA = (Kbs_SuffixArray *)malloc(sizeof(Kbs_SuffixArray));
  outputSA->str = seq;
  outputSA->posArray = suffixArray;
  outputSA->invPosArray = sufPtrMap;
  outputSA->lcpArray = NULL;
  outputSA->buckets = buckets;
  outputSA->qGramSize = q;
  return outputSA;
}


