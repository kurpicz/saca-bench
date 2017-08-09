#include "LinearSuffixSort.h"
#include <iostream>
#include <cstring>
#include <cstdlib>

/*********************************
Find the maximum and minimum value of an integer array.
Pre:    inputString is of length inputLength, Max and Min are
        integers.
Post:   Max will contain the value of the maximum element.
        Min will contain the value of the minimum element.
Modify: Max, Min.
*********************************/
inline void findMaxMin(const int* inputString, const int inputLength, 
		       int& Max, int& Min)
{
  int i, temp;

  if(inputLength <= 0)
  {
    std::cout << "In function findMaxMin(int*, int, int&, int&):" << std::endl;
    std::cout << "Length of input string cannot be less than 0." 
	 << std::endl;
    ::exit(0);
  }  

  Max = inputString[0];
  Min = inputString[0];

  for(i = 1; i < inputLength; i++)
  {
    temp = Max - inputString[i];
    temp = temp >> MAX_INT_INDEX;
    Max = Max + ((inputString[i] - Max) & temp);

    temp = inputString[i] - Min;
    temp = temp >> MAX_INT_INDEX;
    Min = Min - ((Min - inputString[i]) & temp);
  }
}


void suffix_type(const int* inputString, const int inputLength, 
		 int& numStype, int& numLtype, 
		 BooleanString& suffixType)
{
  int i, j, k;

  j = 0;
  numStype = 0;
  numLtype = 0;

  suffixType.setAll(true);

  for (i = 0; i < inputLength - 1; i++)
  {  
    /****************************************/
    //If character T[i] does not equal to T[i+1] make a 
    //decision and go back to mark all the previously 
    //undecided suffix a certain type. Also increment the
    //counter for that type. And set undecided suffix 
    //counter (j) to 0.
    /****************************************/
    if (inputString[i] < inputString[i+1])
    {
      for (k = i - j; k <= i; k++)
      {
	numStype++;
	suffixType.setValN(k, 1);
      }
      j = 0;
    }
    else if (inputString[i] > inputString[i+1])
    {
      for (k = i - j; k <= i; k++)
      {
	numLtype++;
	suffixType.setValN(k, 0);
      }
      j = 0;
    }
    else
    {
      /**************************************/
      //If two adjacent suffixes have the same first
      //character, move on, to the next, but remember the 
      //number of undecided suffixes by increment j.
      /**************************************/      
      j++;
    }
  }

  /******************************************/
  //The last suffix $ must be selected no matter we choose
  //to sort L or to sort S. So the type of the last suffix
  //is set to which ever type that is smaller in number.
  /******************************************/

  if (numStype < numLtype)
  {
    suffixType.setValN(inputLength - 1, 1);
    numStype++;
  }
  else
  {
    suffixType.setValN(inputLength - 1, 0);
    numLtype++;
  }
}

void suffix_type(const unsigned char* inputString, const int inputLength, 
		 int& numStype, int& numLtype, 
		 BooleanString& suffixType)
{
  int i, j, k;

  j = 0;
  numStype = 0;
  numLtype = 0;

  suffixType.setAll(true);

  for (i = 0; i < inputLength - 2; i++)
  {  
    /****************************************/
    //If character T[i] does not equal to T[i+1] make a 
    //decision and go back to mark all the previously 
    //undecided suffix a certain type. Also increment the
    //counter for that type. And set undecided suffix 
    //counter (j) to 0.
    /****************************************/
    if (inputString[i] < inputString[i+1])
    {
      for (k = i - j; k <= i; k++)
      {
	numStype++;
	suffixType.setValN(k, 1);
      }
      j = 0;
    }
    else if (inputString[i] > inputString[i+1])
    {
      for (k = i - j; k <= i; k++)
      {
	numLtype++;
	suffixType.setValN(k, 0);
      }
      j = 0;
    }
    else
    {
      /**************************************/
      //If two adjacent suffixes have the same first
      //character, move on, to the next, but remember the 
      //number of undecided suffixes by increment j.
      /**************************************/      
      j++;
    }
  }
  for (k = i - j; k <= i; k++)
  {
    numLtype++;
    suffixType.setValN(k, 0);
  }
  /******************************************/
  //The types of all characters, except the last one is set.
  //Because we are comparing inputString[i] and 
  //inputString[i+1] so even if the last few characters
  //are the same, the very last character will determine 
  //their type.
  /******************************************/

  /******************************************/
  //The last suffix $ must be selected no matter we choose
  //to sort L or to sort S. So the type of the last suffix
  //is set to which ever type that is smaller in number.
  /******************************************/

  if (numStype < numLtype)
  {
    suffixType.setValN(inputLength - 1, 1);
    numStype++;
  }
  else
  {
    suffixType.setValN(inputLength - 1, 0);
    numLtype++;
  }
}

int* LinearSuffixSort(const unsigned char* inputString, const int stringLength)
{
  int numS, numL, maxDistance;
  int* arrayA;
  int* arrayB;
  int* tPrime;
  BooleanString suffixType(stringLength);
  BooleanString BuckB;

  //Start of real program;
  suffix_type(inputString, stringLength, numS, numL, suffixType);

  if (suffixType.getValN(stringLength - 1) == 1 &&
      numS == 1)
  {
    arrayB = new int[1];
    arrayB[0] = stringLength - 1;
    arrayA = new int[stringLength];
    construct_SA_typeS(arrayB, numS, inputString, stringLength,
		       suffixType, arrayA);  
    delete[] arrayB;
    return(arrayA);
  }

  if (suffixType.getValN(stringLength - 1) == 1)
  {
#ifndef NDEBUG
    std::cout << "Sorting type S suffixes" << std::endl;
    std::cout << numS << " " << numL << " " << stringLength << std::endl;
#endif
    //Less type S suffixes
    arrayB = new int[numS];
    BuckB.initialize(numS);
  	           
    s_distanceR(suffixType, stringLength, maxDistance);
   
    int j = 0;
    for (int i = 0; i < stringLength; i++)
    {
      assert(j <= numS);
      arrayB[j] = i;
      j = j + suffixType.getValN(i);
    }
      
    sort_s_substringC(inputString, stringLength, arrayB, BuckB, numS, 
		      suffixType, maxDistance);

    if (BuckB.isAllTrue())
    {
      arrayA = new int[stringLength];
      construct_SA_typeS(arrayB, numS, inputString, stringLength,
			 suffixType, arrayA);
      delete[] arrayB;
      return (arrayA);
    }
    
    tPrime = new int[numS];
    construct_TPrime_typeS(arrayB, numS, BuckB, tPrime, 
			   stringLength, suffixType); 
   
    delete[] arrayB;

    arrayB = LinearSuffixSort(tPrime, numS);
    delete[] tPrime;
    
    reconstruct_B_typeS(arrayB, numS, suffixType, stringLength);
    arrayA = new int[stringLength];
    construct_SA_typeS(arrayB, numS, inputString, stringLength,
		       suffixType, arrayA);
    delete[] arrayB;
    return (arrayA);
  }
  else
  {
#ifndef NDEBUG
    std::cout << "Sorting type L suffixes" << std::endl;
    std::cout << numS << " " << numL << " " << stringLength << std::endl;
#endif
    //Less type L suffixes
    arrayB = new int[numL];
    BuckB.initialize(numL);
    
    l_distanceR(suffixType, stringLength, maxDistance);

    int j = 0;
    for (int i = 0; i < stringLength; i++)
    {
      assert(j <= numL);
      arrayB[j] = i;
      j = j - (suffixType.getValN(i) - 1);
    }

    sort_l_substringC(inputString, stringLength, arrayB, BuckB, numL, 
		      suffixType, maxDistance);

    if (BuckB.isAllTrue())
    {
      arrayA = new int[stringLength];
      construct_SA_typeL(arrayB, numL, inputString, stringLength,
			 suffixType, arrayA);
      delete[] arrayB;
      return (arrayA);
    }
    
    tPrime = new int[numL];
    construct_TPrime_typeL(arrayB, numL, BuckB, tPrime, 
			   stringLength, suffixType); 
   
    delete[] arrayB;

    arrayB = LinearSuffixSort(tPrime, numL);
    delete[] tPrime;
    
    reconstruct_B_typeL(arrayB, numL, suffixType, stringLength);
    arrayA = new int[stringLength];
    construct_SA_typeL(arrayB, numL, inputString, stringLength,
		       suffixType, arrayA);
    delete[] arrayB;
    return (arrayA);
  }
}

void s_distanceR(const BooleanString& suffixType, 
		 const int inputLength, int& maxDist)
{
  int i, temp, prevDist;

  //Set the reverse S-distance for the last suffix to 0.

  maxDist = 0;

  prevDist = 1;
  for (i = inputLength - 2; i >= 0; i--)
  {
    //Calculate maxDist

    temp = maxDist - prevDist;
    temp = temp >> MAX_INT_INDEX;
    maxDist = maxDist + ((prevDist - maxDist) & temp);

    //If suffix i is type L, then increment prevDist by 1.
    //else set prevDist to 1.
 
    temp = suffixType.getValN(i) - 1;
    prevDist = (prevDist & temp) + 1;
  }
}  


//This function computes the reverse l_distance. 

void l_distanceR(const BooleanString& suffixType, 
		 const int inputLength, int& maxDist)
{
  int i, temp, prevDist;
  
  //Set the reverse l-distance for the last suffix to 0;

  maxDist = 0;

  prevDist = 1;
  for (i = inputLength - 2; i >= 0; i--)
  {
    //Calculate maxDist.

    temp = maxDist - prevDist;
    temp = temp >> MAX_INT_INDEX;
    maxDist = maxDist + ((prevDist - maxDist) & temp);

    //If suffix i is type S, then increment prevDist by 1.
    //else set prevDist to 1.

    temp = 0 - suffixType.getValN(i);
    prevDist = (prevDist & temp) + 1;
  }
}  
 

void sort_s_substringC(const unsigned char* inputString, const int inputLength, 
		       int* A, BooleanString& BuckA, const int ALength, 
		       const BooleanString& suffixType, const int maxDist)
{
  int i, prevCount, temp, tmpIndex, offset;
  int start, end, prevPos, bufferLen;
  int* buffer;
  int* skipVal;
  int* tempBucket;

  //set BuckA to all false.
  //invALength is the negative value of ALength.
  //and other initializations.
  
  bufferLen = MAX_NUM_CHAR * 2;
  BuckA.setAll(false);
  buffer = new int[bufferLen];
  skipVal = new int[ALength];;
  ::memset ((void*)skipVal, 0, sizeof(int) * ALength);
  tempBucket = new int[ALength];
 
  //Set skipVal[0] to ALength, i.e. the entire array is an 
  //unsorted bucket.

  skipVal[0] = ALength;

  //while it is not all sorted. continue to sort.

  for (offset = 0; offset <= maxDist; offset++)
  {
    start = 0;
    prevPos = 0;
    //continue sort until reach the end of array A.

    while (start < ALength)
    {
      //identify the first bucket need work done. 
      //also go back to previous bucket to increase
      //its skipVal. 
      //start is the index where the bucket starts.
      //end is the index where the bucket ends, i.e.
      //the index of the last element in the array.

      prevPos = start;
      while ((start < ALength) && (skipVal[start] < 0))
      {
	start = -skipVal[start];
      }
      end = skipVal[start] - 1;
      skipVal[prevPos] = -start;
      ::memset((void*)buffer, 0, sizeof(int) * bufferLen);

      //Sort the bucket if start <= ALength - 1. 

      if (start < ALength)
      {
	//count the occurence, also copy the elements
	//into a temporary bucket. 
	//If the next character is type S. Then add 1 to 2*c+1. 
	//Otherwise add 1 to 2*c.
	int sentinel_check = 0;

	for (i = start; i <= end; i++)
	{
	  //copy the element into temporary bucket.
	  tempBucket[i] = A[i];

	  //count the occurence.
	  tmpIndex = A[i] + offset;
	  if(tmpIndex < (inputLength - 1)) {
	    temp = ((int) inputString[tmpIndex]) << 1;
	    temp = temp + suffixType.getValN(tmpIndex);
	    buffer[temp]++;
	  } else {
	    sentinel_check = 1;
	  }
	}

	//calculate the new starting value of each bucket.
	prevCount = buffer[0];
	buffer[0] = start + sentinel_check;
	
	for (i = 1; i < bufferLen; i++)
	{
	  temp = buffer[i];
	  buffer[i] = buffer[i - 1] + prevCount;
	  prevCount = temp;
	}

	//put the elements in there correct positions.
	//must read from the temporary bucket, because array
	//A is not reliable.

	for (i = start; i <= end; i++)
	{
	  tmpIndex = tempBucket[i] + offset;
	  if(tmpIndex < (inputLength - 1)) {
	    temp = ((int) inputString[tmpIndex]) << 1;
	    temp = temp + suffixType.getValN(tmpIndex);
	    A[buffer[temp]] = tempBucket[i];
	    buffer[temp]++;
	  } else {
	    A[start] = tempBucket[i];
	  }
	}

	if (sentinel_check != 0)
	{
	  BuckA.setValN(start, 1);
	  skipVal[start] = - (start + 1);
	}
	if (buffer[0] > (start + sentinel_check))
	{
	  BuckA.setValN(buffer[0] - 1, 1);
	  skipVal[start + sentinel_check] = - buffer[0];
	}
       
	//set the bucket boundary of array A, and the correct
	//skipVal. skipVal[i] is negative if no more work required for
	//the bucket anymore (all the buckets whose last character is
	//type S). 
	//An empty bucket will not affect a previously non-empty
	//bucket. Because suppose the empty bucket is i, then 
	//buffer[i] actually points to the beginning of the 
	//next bucket. buffer[i-1] points to either the beginning
	//of the previously non-empty bucket, or it points to
	//the beginning of the next bucket. In either case, it
	//is set correctly. 
	
	i = 1;
	if (offset > 0)
	{
	  //for all other buckets if its not empty, its value is greater
	  //than the previous bucket. 
	  for (i = 1; i < bufferLen; i++)
	  {
	    //if the bucket only have 1 element, then skipVal is negative
	    if (buffer[i] == buffer[i-1] + 1)
	    {
	      BuckA.setValN(buffer[i] - 1, 1);
	      skipVal[buffer[i-1]] = - buffer[i]; 
	    } 
	    else if (buffer[i] > buffer[i-1] + 1)
	    {
	      BuckA.setValN(buffer[i] - 1, 1);
	      temp = - (i & 1);
	      temp = (buffer[i] ^ temp) - temp;
	      skipVal[buffer[i-1]] = temp; 
	    } 
	  }
	}
	else
	{
	  //for all other buckets if its not empty, its value is greater
	  //than the previous bucket. 
	  for (i = 1; i < bufferLen; i++)
	  {
	    //if the bucket only have 1 element, then skipVal is negative
	    if (buffer[i] == buffer[i-1] + 1)
	    {
	      BuckA.setValN(buffer[i] - 1, 1);
	      skipVal[buffer[i-1]] = - buffer[i]; 
	    } 
	    else if (buffer[i] > buffer[i-1] + 1)
	    {
	      BuckA.setValN(buffer[i] - 1, 1);
	      skipVal[buffer[i-1]] = buffer[i]; 
	    }
	  }
	}

	//the above while loop will not be executed if the 
	//bucket contains only 1 element. In that case we set
	//the values in the if statement below.
	
	if (start == end)
	{
	  skipVal[start] = -(end - 1);
	  BuckA.setValN(start,1);
	}
	
	//set the start to the point to the next bucket.
	
	start = end + 1;
      }
    }
  }

  delete[] skipVal;
  delete[] tempBucket;
  delete[] buffer;
}

void sort_l_substringC(const unsigned char* inputString, const int inputLength, 
		       int* A, BooleanString& BuckA, const int ALength, 
		       const BooleanString& suffixType, const int maxDist)
{
  int i, prevCount, temp, tmpIndex, offset;
  int start, end, prevPos, bufferLen;
  int* buffer;
  int* skipVal;
  int* tempBucket;

  //set BuckA to all false.
  //invALength is the negative value of ALength.
  //and other initializations.

  bufferLen = MAX_NUM_CHAR * 2;
  BuckA.setAll(false);
  buffer = new int[bufferLen];
  skipVal = new int[ALength];
  ::memset ((void*)skipVal, 0, sizeof(int) * ALength);
  tempBucket = new int[ALength];

  //set value for A[0] which is the first position in
  //array A. Which should be occupied by '$'.

  skipVal[0] = ALength;

  //while it is not all sorted. continue to sort.

  for (offset = 0; offset <= maxDist; offset++)
  {
    start = 0;
    prevPos = 0;
    //continue sort until reach the end of array A.

    while (start < ALength)
    {
      //identify the first bucket need work done. 
      //also go back to previous bucket to increase
      //its skipVal. 
      //start is the index where the bucket starts.
      //end is the index where the bucket ends, i.e.
      //the index of the last element in the array.

      prevPos = start;
      while ((start < ALength) && (skipVal[start] < 0))
      {
	start = -skipVal[start];
      }
      end = skipVal[start] - 1;
      skipVal[prevPos] = -start;
      ::memset((void*)buffer, 0, sizeof(int) * bufferLen);

      //Sort the bucket if start <= ALength - 1. 

      if (start < ALength)
      {
	//count the occurence, also copy the elements
	//into a temporary bucket. 
	//If the next character is type S. Then add 1 to 2*c+1. 
	//Otherwise add 1 to 2*c.

	int sentinel_check = 0;
	for (i = start; i <= end; i++)
	{
	  //copy the element into temporary bucket.
	  tempBucket[i] = A[i];

	  //count the occurence.
	  tmpIndex = A[i] + offset;
	  if(tmpIndex < (inputLength - 1)) {
	    temp = ((int) inputString[tmpIndex]) << 1;
	    temp = temp + suffixType.getValN(tmpIndex);
	    buffer[temp]++;
	  } else {
	    sentinel_check = 1;
	  }
	}

	//calculate the new starting value of each bucket.
	prevCount = buffer[0];
	buffer[0] = start + sentinel_check;
	
	for (i = 1; i < bufferLen; i++)
	{
	  temp = buffer[i];
	  buffer[i] = buffer[i - 1] + prevCount;
	  prevCount = temp;
	}

	//put the elements in there correct positions.
	//must read from the temporary bucket, because array
	//A is not reliable.

	for (i = start; i <= end; i++)
	{
	  tmpIndex = tempBucket[i] + offset;
	  if(tmpIndex < (inputLength - 1)) {
	    temp = ((int) inputString[tmpIndex]) << 1;
	    temp = temp + suffixType.getValN(tmpIndex);
	    A[buffer[temp]] = tempBucket[i];
	    buffer[temp]++;
	  } else {
	    A[start] = tempBucket[i];
	  }
	}

	if (sentinel_check != 0)
	{
	  BuckA.setValN(start, 1);
	  skipVal[start] = - (start + 1);
	}
	if (buffer[0] > (start + sentinel_check))
	{
	  BuckA.setValN(buffer[0] - 1, 1);
	  skipVal[start + sentinel_check] = - buffer[0];
	}

	//set the bucket boundary of array A, and the correct
	//skipVal. skipVal[i] is negative if no more work required for
	//the bucket anymore (all the buckets whose last character is
	//type L). 
	//An empty bucket will not affect a previously non-empty
	//bucket. Because suppose the empty bucket is i, then 
	//buffer[i] actually points to the beginning of the 
	//next bucket. buffer[i-1] points to either the beginning
	//of the previously non-empty bucket, or it points to
	//the beginning of the next bucket. In either case, it
	//is set correctly. 
	
	if (offset > 0)
	{
	  //for all other buckets if its not empty, its value is greater
	  //than the previous bucket. 
	  for (i = 1; i < bufferLen; i++)
	  {
	    //if the bucket only have 1 element, then skipVal is negative
	    if (buffer[i] == buffer[i-1] + 1)
	    {
	      BuckA.setValN(buffer[i] - 1, 1);
	      skipVal[buffer[i-1]] = - buffer[i]; 
	    } 
	    else if (buffer[i] > buffer[i-1] + 1)
	    {
	      BuckA.setValN(buffer[i] - 1, 1);
	      temp = - ((i & 1) ^ 1);
	      temp = (buffer[i] ^ temp) - temp;
	      skipVal[buffer[i-1]] = temp; 
	    } 
	  }
	}
	else
	{
	  //for all other buckets if its not empty, its value is greater
	  //than the previous bucket. 
	  for (i = 1; i < bufferLen; i++)
	  {
	    //if the bucket only have 1 element, then skipVal is negative
	    if (buffer[i] == buffer[i-1] + 1)
	    {
	      BuckA.setValN(buffer[i] - 1, 1);
	      skipVal[buffer[i-1]] = - buffer[i]; 
	    } 
	    else if (buffer[i] > buffer[i-1] + 1)
	    {
	      BuckA.setValN(buffer[i] - 1, 1);
	      skipVal[buffer[i-1]] = buffer[i]; 
	    }
	  }
	}

	//set the start to the point to the next bucket.
	
	start = end + 1;
      }
    }
  }
  delete[] skipVal;
  delete[] tempBucket;
  delete[] buffer;
}


void construct_TPrime_typeS(int* ArrayB, const int ArrayBLength,
			    const BooleanString& BuckB,
			    int* tPrime, const int inputLength,
			    const BooleanString& suffixType)
{
  int* Buckets;
  int i, j, currBuck;
  int tempVal, tempValInv;

  // initalize array Buckets, and for each type
  // S suffix calculate their bucket number
  
  Buckets = new int[inputLength];
 
  currBuck = 0;
  for (i = 0; i < ArrayBLength; i++)
  {
    Buckets[ArrayB[i]] = currBuck;
    currBuck = currBuck + BuckB.getValN(i);
  }

  // construct tPrime
  j = 0;
  for (i = 0; i < inputLength; i++)
  {
    tempVal = - suffixType.getValN(i);
    tempValInv = ~tempVal;
    tPrime[j] = (tPrime[j] & tempValInv) | (Buckets[i] & tempVal);
    j = j + (1 & tempVal);
  }
  delete[] Buckets;
}

void construct_TPrime_typeL(int* ArrayB, const int ArrayBLength,
			    const BooleanString& BuckB,
			    int* tPrime, const int inputLength,
			    const BooleanString& suffixType)
{
  int* Buckets;
  int i, j, currBuck;
  int tempVal, tempValInv;

  // initalize array Buckets, and for each type
  // L suffix calculate their bucket number

  Buckets = new int[inputLength];
 
  currBuck = 0;
  for (i = 0; i < ArrayBLength; i++)
  {
    Buckets[ArrayB[i]] = currBuck;
    currBuck = currBuck + BuckB.getValN(i);
  }

  // construct tPrime
  j = 0;
  for (i = 0; i < inputLength; i++)
  {
    tempVal = (suffixType.getValN(i) - 1);
    tempValInv = ~tempVal;
    tPrime[j] = (tPrime[j] & tempValInv) | (Buckets[i] & tempVal);
    j = j + (1 & tempVal);
  }
  delete[] Buckets;
}


void construct_ArrayB_typeS(const int* ArrayA, const int inputLength, 
			    const BooleanString& BuckA, 
			    int* ArrayB, BooleanString& BuckB, 
			    const BooleanString& suffixType)
{
  int i, j;
  int temp;

  BuckB.setAll(false);

  j = 0;
 
  for (i = 0; i < inputLength; i++)
  {
    temp = ArrayA[i];
    if (suffixType.getValN(temp) == 1)
    {
      ArrayB[j] = ArrayA[i];
      j++;
    }
    if (BuckA.getValN(i) == 1 && j - 1 >= 0)
    {
      BuckB.setValN(j - 1, 1);
    }
  }
}

void construct_ArrayB_typeL(const int* ArrayA, const int inputLength, 
			    const BooleanString& BuckA, 
			    int* ArrayB, BooleanString& BuckB, 
			    const BooleanString& suffixType)
{
  int i, j;
  int temp;

  BuckB.setAll(false);

  j = 0;
 
  for (i = 0; i < inputLength; i++)
  {
    temp = ArrayA[i];
    if (suffixType.getValN(temp) == 0)
    {
      ArrayB[j] = ArrayA[i];
      j++;
    }
    if (BuckA.getValN(i) == 1 && j - 1 >= 0)
    {
      BuckB.setValN(j - 1, 1);
    }
  }
}


void construct_SA_typeS(int* ArrayB, const int ArrayBLength,
			const int* stringT, const int inputLength,
			const BooleanString& suffixType,
			int* suffixArray)
{
  int* count;
  int sigma, maxChar, minChar;
  int i, j, temp, prevCount, prevChar, charBuck;

  // count the size of the alphabet in stringT.

  findMaxMin(stringT, inputLength, maxChar, minChar);
  sigma = maxChar - minChar + 1;

  // for each character in the alphabet calculate
  // the beginning position of their bucket.

  count = new int[sigma];
  ::memset((void*)count, 0, sigma * sizeof(int));

  for (i = 0; i < inputLength; i++)
  {
    temp = stringT[i] - minChar;
    count[temp]++;
  }
  
  prevCount = count[0];
  count[0] = 0;

  for (i = 1; i < sigma; i++)
  {
    temp = count[i];
    count[i] = count[i-1] + prevCount;
    prevCount = temp;
  }

  // initalize the suffix array

  ::memset((void*)suffixArray, -1, inputLength * sizeof(int));

  // initalize j, which means the at the beginning of array B

  j = 0;

  // move all the suffixes into the suffix array in
  // order

  for (i = 0; i < inputLength; i++)
  {
    // if suffixArray[i] is -1, then an element of
    // array B should be put in that place. And its
    // previous suffix moved to the approperiate 
    // place. 
    // Otherwise just move the previous suffix to
    // the approperiate place. 

    if (suffixArray[i] == -1)
    {
      assert(j < ArrayBLength);
      suffixArray[i] = ArrayB[j];
      j++;
    }
    prevChar = suffixArray[i] - 1;
    if (prevChar >= 0)
    {
      if (suffixType.getValN(prevChar) == 0)
      {
	charBuck = stringT[prevChar];
	if (count[charBuck] > i)
	{
	  suffixArray[count[charBuck]] = prevChar;
	  count[charBuck]++;
	}
      }
    }
  }  
  delete[] count;
}

void construct_SA_typeL(int* ArrayB, const int ArrayBLength,
			const int* stringT, const int inputLength,
			const BooleanString& suffixType,
			int* suffixArray)
{
  int* count;
  int sigma, maxChar, minChar, prevChar, charBuck;
  int i, j, temp;

  // count the size of the alphabet in stringT.

  findMaxMin(stringT, inputLength, maxChar, minChar);
  sigma = maxChar - minChar + 1;

  // for each character in the alphabet calculate
  // the ending position of their bucket.

  count = new int[sigma];
  ::memset((void*)count, 0, sigma * sizeof(int));

  for (i = 0; i < inputLength; i++)
  {
    temp = stringT[i] - minChar;
    count[temp]++;
  }

  count[0] = count[0] - 1;

  for (i = 1; i < sigma; i++)
  {
    count[i] = count[i-1] + count[i];
  }

  // initalize the suffix array

  ::memset((void*)suffixArray, -1, inputLength * sizeof(int));

  // initalize j to be at the end of array B.

  j = ArrayBLength - 1;

  // move all the suffixes into the suffix array in
  // order

  for (i = inputLength - 1; i >= 0; i--)
  {
    // if suffixArray[i] is -1, then an element of
    // array B should be put in that place. And its
    // previous suffix moved to the approperiate 
    // place. 
    // Otherwise just move the previous suffix to
    // the approperiate place. 

    if (suffixArray[i] == -1)
    {
      assert(j < ArrayBLength);
      suffixArray[i] = ArrayB[j];
      j--;
    }
    prevChar = suffixArray[i] - 1;
    if (prevChar >= 0)
    {
      if (suffixType.getValN(prevChar) == 1)
      {
	charBuck = stringT[prevChar];
	if (count[charBuck] < i)
	{
	  suffixArray[count[charBuck]] = prevChar;
	  count[charBuck]--;
	}
      }
    }
  } 
  delete[] count; 
}


void construct_SA_typeS(int* ArrayB, const int ArrayBLength,
			const unsigned char* stringT, const int inputLength,
			const BooleanString& suffixType,
			int* suffixArray)
{
  int* count;
  int i, j, temp, prevCount, prevChar, charBuck;

  // for each character in the alphabet calculate
  // the beginning position of their bucket.

  count = new int[MAX_NUM_CHAR];
  ::memset((void*)count, 0, MAX_NUM_CHAR * sizeof(int));

  for (i = 0; i < inputLength - 1; i++)
  {
    temp = (int) stringT[i];
    count[temp]++;
  }
  
  prevCount = count[0];
  count[0] = 1;

  for (i = 1; i < MAX_NUM_CHAR; i++)
  {
    temp = count[i];
    count[i] = count[i-1] + prevCount;
    prevCount = temp;
  }

  // initalize the suffix array

  ::memset((void*)suffixArray, -1, inputLength * sizeof(int));

  // initalize j, which means the at the beginning of array B

  j = 0;

  // move all the suffixes into the suffix array in
  // order

  for (i = 0; i < inputLength; i++)
  {
    // if suffixArray[i] is -1, then an element of
    // array B should be put in that place. And its
    // previous suffix moved to the approperiate 
    // place. 
    // Otherwise just move the previous suffix to
    // the approperiate place. 

    if (suffixArray[i] == -1)
    {
      assert(j < ArrayBLength);
      suffixArray[i] = ArrayB[j];
      j++;
    }
    prevChar = suffixArray[i] - 1;
    if (prevChar >= 0)
    {
      if (suffixType.getValN(prevChar) == 0)
      {
	charBuck = (int) stringT[prevChar];
	if (count[charBuck] > i)
	{
	  suffixArray[count[charBuck]] = prevChar;
	  count[charBuck]++;
	}
      }
    }
  }  
  delete[] count;
}

void construct_SA_typeL(int* ArrayB, const int ArrayBLength,
			const unsigned char* stringT, const int inputLength,
			const BooleanString& suffixType,
			int* suffixArray)
{
  int* count;
  int prevChar, charBuck;
  int i, j, temp;

  // for each character in the alphabet calculate
  // the ending position of their bucket.

  count = new int[MAX_NUM_CHAR];
  ::memset((void*)count, 0, MAX_NUM_CHAR * sizeof(int));

  // count the occurrence. 
  for (i = 0; i < inputLength - 1; i++)
  {
    temp = (int) stringT[i];
    count[temp]++;
  }

  count[0] = count[0];

  for (i = 1; i < MAX_NUM_CHAR; i++)
  {
    count[i] = count[i-1] + count[i];
  }

  // initalize the suffix array

  ::memset((void*)suffixArray, -1, inputLength * sizeof(int));

  // initalize j to be at the end of array B.

  j = ArrayBLength - 1;

  // move all the suffixes into the suffix array in
  // order

  for (i = inputLength - 1; i >= 0; i--)
  {
    // if suffixArray[i] is -1, then an element of
    // array B should be put in that place. And its
    // previous suffix moved to the approperiate 
    // place. 
    // Otherwise just move the previous suffix to
    // the approperiate place. 

    if (suffixArray[i] == -1)
    {
      assert(j < ArrayBLength);
      suffixArray[i] = ArrayB[j];
      j--;
    }
    prevChar = suffixArray[i] - 1;
    if (prevChar >= 0)
    {
      if (suffixType.getValN(prevChar) == 1)
      {
	charBuck = (int) stringT[prevChar];
	if (count[charBuck] < i)
	{
	  suffixArray[count[charBuck]] = prevChar;
	  count[charBuck]--;
	}
      }
    }
  } 
  delete[] count; 
}


int* LinearSuffixSort(const int* inputString, const int stringLength)
{
  BooleanString suffixType(stringLength);
  BooleanString BuckA(stringLength);
  BooleanString BuckB, BuckList;
  int numS, numL, maxDist;
  int* ArrayA; 
  int* ArrayB;
  int* Dist; 
  int* DistCount;
  int* theList;
  int* tPrime;
  int* int_buffer;
  int listLength;

  if(stringLength <= 0)
  {
    std::cout << "In function LinearSuffixSort(int*, int):" << std::endl;
    std::cout << "Length of input string cannot be less than 0." 
	 << std::endl;
    ::exit(0);
  }  
#ifndef NDEBUG
  std::cout << std::endl;
#endif

  suffix_type(inputString, stringLength, numS, numL, suffixType);

  if (suffixType.getValN(stringLength - 1) == 1 &&
      numS == 1)
  {
    ArrayB = new int[1];
    ArrayB[0] = stringLength - 1;
    ArrayA = new int[stringLength];
    construct_SA_typeS(ArrayB, numS, inputString, stringLength,
		       suffixType, ArrayA);  
    delete[] ArrayB;
    return(ArrayA);
  }

  ArrayA = new int[stringLength];
  int_buffer = new int[stringLength];
  counting_sort(inputString, stringLength, ArrayA, BuckA, int_buffer);
	delete[] int_buffer;

  if (suffixType.getValN(stringLength - 1) == 1)
  {
#ifndef NDEBUG
    std::cout << "Sorting type S suffixes" << std::endl;
    std::cout << numS << " " << numL << " " << stringLength << std::endl;
#endif
    ArrayB = new int[numS];
    BuckB.initialize(numS);
    construct_ArrayB_typeS(ArrayA, stringLength, BuckA, ArrayB,
			   BuckB, suffixType); 

    Dist = new int[stringLength];

    s_distance(suffixType, stringLength, Dist, maxDist);
    construct_distcount(stringLength, Dist, DistCount, maxDist);

    BuckList.initialize(DistCount[maxDist]);
    listLength = DistCount[maxDist];

    theList = construct_list_typeS(ArrayA, stringLength, Dist, 
				   DistCount, maxDist, BuckA, BuckList, 
				   listLength);

    delete[] DistCount;
    delete[] Dist;

    sort_by_list_typeS(ArrayB, BuckB, theList, BuckList, stringLength,
		       listLength, numS);

    delete[] theList;

    if (BuckB.isAllTrue())
    {
      ArrayA = new int[stringLength];
      construct_SA_typeS(ArrayB, numS, inputString, stringLength,
			 suffixType, ArrayA);
      delete[] ArrayB;
      return (ArrayA);
    }
 
    tPrime = new int[numS];

    construct_TPrime_typeS(ArrayB, numS, BuckB, tPrime, 
			   stringLength, suffixType);
    delete[] ArrayB;

    ArrayB = LinearSuffixSort(tPrime, numS);
    delete[] tPrime;

    reconstruct_B_typeS(ArrayB, numS, suffixType, stringLength);
    ArrayA = new int[stringLength];
    construct_SA_typeS(ArrayB, numS, inputString, stringLength,
		       suffixType, ArrayA);
    delete[] ArrayB;
    return (ArrayA);
  }
  else 
  {
#ifndef NDEBUG
    std::cout << "Sorting type L suffixes" << std::endl;
    std::cout << numS << " " << numL << " " << stringLength << std::endl;
#endif
    ArrayB = new int[numL];
    BuckB.initialize(numL);

    construct_ArrayB_typeL(ArrayA, stringLength, BuckA, ArrayB,
			   BuckB, suffixType);

    Dist = new int[stringLength];

    l_distance(suffixType, stringLength, Dist, maxDist);
    construct_distcount(stringLength, Dist, DistCount, maxDist);

    BuckList.initialize(DistCount[maxDist]);
    listLength = DistCount[maxDist];

    theList = construct_list_typeL(ArrayA, stringLength, Dist, 
				   DistCount, maxDist, BuckA, BuckList, 
				   listLength);
    delete[] DistCount;
    delete[] Dist;

    sort_by_list_typeL(ArrayB, BuckB, theList, BuckList, stringLength,
		       listLength, numL);
    delete[] theList;
   
    if (BuckB.isAllTrue())
    {
      ArrayA = new int[stringLength];
      construct_SA_typeL(ArrayB, numL, inputString, stringLength,
			 suffixType, ArrayA);
      delete[] ArrayB;
      return (ArrayA);
    }

    tPrime = new int[numL];

    construct_TPrime_typeL(ArrayB, numL, BuckB, tPrime, 
			   stringLength, suffixType);
    delete[] ArrayB;

    ArrayB = LinearSuffixSort(tPrime, numL);
    delete[] tPrime;
    
    reconstruct_B_typeL(ArrayB, numL, suffixType, stringLength);
    ArrayA = new int[stringLength];
    construct_SA_typeL(ArrayB, numL, inputString, stringLength,
		       suffixType, ArrayA);
    delete[] ArrayB;
    return (ArrayA);
  }
}


void reconstruct_B_typeS(int* ArrayB, const int ArrayBLength,
			 const BooleanString& suffixType,
			 const int inputLength)
{
  int* convertion;
  int i, j, tempVal, tempValInv;
  
  // build a conversion array, such that conversion[i] = j
  // where i is the index of a suffix of T', and j is
  // the index of the corresponding type S suffix in T

  convertion = new int[ArrayBLength];

  j = 0;
  for (i = 0; i < inputLength; i++)
  {
    tempVal = suffixType.getValN(i);
    tempVal = tempVal << MAX_INT_INDEX;
    tempVal = tempVal >> MAX_INT_INDEX;
    tempValInv = ~tempVal;

    convertion[j] = (i & tempVal) | (convertion[j] & tempValInv);
    j = j + (1 & tempVal);
  }

  // use the conversion array to calculate the actual 
  // index of the type S suffix of ArrayB[i]

  for (i = 0; i < ArrayBLength; i++)
  {
    tempVal = ArrayB[i];
    ArrayB[i] = convertion[tempVal];
  }
  delete[] convertion;
}

void reconstruct_B_typeL(int* ArrayB, const int ArrayBLength,
			 const BooleanString& suffixType,
			 const int inputLength)
{
  int* convertion;
  int i, j, tempVal, tempValInv;

  // build a conversion array, such that conversion[i] = j
  // where i is the index of a suffix of T', and j is
  // the index of the corresponding type L suffix in T

  convertion = new int[ArrayBLength];

  j = 0;
  for (i = 0; i < inputLength; i++)
  {
    tempVal = suffixType.getValN(i);
    tempVal = tempVal << MAX_INT_INDEX;
    tempVal = tempVal >> MAX_INT_INDEX;
    tempValInv = ~tempVal;

    convertion[j] = (i & tempValInv) | (convertion[j] & tempVal);
    j = j + (1 & tempValInv);
  }

  // use the conversion array to calculate the actual 
  // index of the type S suffix of ArrayB[i]

  for (i = 0; i < ArrayBLength; i++)
  {
    tempVal = ArrayB[i];
    ArrayB[i] = convertion[tempVal];
  }
  delete[] convertion;
}


int* construct_list_typeS(int*& ArrayA, const int inputLength, int* Dist, 
		          int* DistCount, const int maxDist, 
		          const BooleanString& BuckA, 
		          BooleanString& BuckList, const int listLength)
{
  int i, j;
  int posList;
  int temp;
  int startB, endB;

  BuckList.setAll(false);
  i = 0;

  while (i < inputLength)
  {
    startB = i;
    while (BuckA.getValN(i) != 1 && i < inputLength)
    {
      //use Dist to be the temporary reverse mapping array of
      //the List.
      temp = Dist[ArrayA[i]];
      if (temp > 0)
      {
	posList = DistCount[temp - 1];
	Dist[ArrayA[i]] = posList;
	BuckList.setValN(posList, 1);
	DistCount[temp - 1]++;
      }
      else
      {
	Dist[ArrayA[i]] = -1;
      }
      i++;
    }
  
    //set the values for the last member in the bucket. 

    temp = Dist[ArrayA[i]];
    if (temp != 0)
    {
      posList = DistCount[temp - 1];
      Dist[ArrayA[i]] = posList;
      BuckList.setValN(posList, 1);
      DistCount[temp - 1]++;
    }
    else
    {
      Dist[ArrayA[i]] = -1;
    }
    endB = i;

    //for all the member in the middle of the bucket in list
    //reset its BuckList to false. i.e. show that there is
    //no boundary between i and i+1 in list. It is possible
    //that the boundary of the last bucket of a S-distance
    //is not drawn properly. 
  
    for (j = startB; j < endB; j++)
    {
      posList = Dist[ArrayA[j]];  
      if (posList >= 0 && posList != listLength - 1)
      {
	if (BuckList.getValN(posList+1) == 1)
        {
	  BuckList.setValN(posList, 0);
	}
      }
    }
    i++;
  }
  //reconstruct the list from Dist

  for (i = 0; i < inputLength; i++)
  {
    if (Dist[i] >= 0)
    {
      ArrayA[Dist[i]] = i;
    }
  }

  //At this point DistCount[i] give the total number
  //of suffixes having S-distance of i+1. i.e. 
  //DistCount[0] give the number of suffixes with
  //S-distance of 1.

  for (i = 0; i < maxDist; i++)
  {
    BuckList.setValN(DistCount[i]-1, 1);
  }

  //Calculate the index of the beginning of the type S
  //substring.

  for (i = 0; i < maxDist; i++)
  {
    if (i == 0)
    {
      j = 0;
    }
    else
    {
      j = DistCount[i-1];
    }
    while (j < DistCount[i])
    {
      ArrayA[j] = ArrayA[j] - i - 1;
      j++;
    }
  }
  return (ArrayA);
}

int* construct_list_typeL(int*& ArrayA, const int inputLength, int* Dist, 
		          int* DistCount, const int maxDist, 
		          const BooleanString& BuckA, 
		          BooleanString& BuckList, const int listLength)
{
  int i, j;
  int posList;
  int temp;
  int startB, endB;
  bool firstElement;

  BuckList.setAll(false);
  i = inputLength - 1;

  while (i >= 0)
  {
    endB = i;
    if (i > 0)
    {
      if (BuckA.getValN(i-1) != 1)
      {
	firstElement = false;
      }
      else
      {
	firstElement = true;
      }
    }
    else
    {
      firstElement = true;
    }

    while (!firstElement)
    {
 
      //use Dist to be the temporary reverse mapping array of
      //the List. 
 
      temp = Dist[ArrayA[i]];
      if (temp > 0)
      {
	posList = DistCount[temp - 1];
	Dist[ArrayA[i]] = posList;
	BuckList.setValN(posList, 1);
	DistCount[temp - 1]++;
      }
      else
      {
	Dist[ArrayA[i]] = -1;
      }

      // Decrement i, and determine if i is the first element
      // of the bucket. 

      i--;
      if (i > 0)
      {
	if (BuckA.getValN(i-1) != 1)
	{
	  firstElement = false;
	}
	else
	{
	  firstElement = true;
	}
      }
      else
      {
	firstElement = true;
      }
    }

    //set the values for the first member in the bucket. 

    temp = Dist[ArrayA[i]];
    if (temp != 0)
    {
      posList = DistCount[temp - 1];
      Dist[ArrayA[i]] = posList;
      BuckList.setValN(posList, 1);
      DistCount[temp - 1]++;
    }
    else
    {
      Dist[ArrayA[i]] = -1;
    }

    startB = i;
    
    //for all the member in the middle of the bucket in list
    //reset its BuckList to false. i.e. show that there is
    //no boundary between i and i+1 in list. It is possible
    //that the boundary of the last bucket of a L-distance
    //is not drawn properly. 
   
    for (j = endB; j >= startB; j--)
    {
      posList = Dist[ArrayA[j]];
      if (posList >= 0 && posList != listLength - 1)
      {
	if (BuckList.getValN(posList+1) == 1)
	{
	  BuckList.setValN(posList, 0);
	}
      }
    }
    i--;
  }

  //reconstruct the list from Dist

  for (i = 0; i < inputLength; i++)
  {
    if (Dist[i] >= 0)
    { 
      ArrayA[Dist[i]] = i;
    }
  }

  //At this point DistCount[i] give the total number
  //of suffixes having L-distance of i+1. i.e. 
  //DistCount[0] give the number of suffixes with
  //L-distance of 1.

  for (i = 0; i < maxDist; i++)
  {
    BuckList.setValN(DistCount[i]-1, 1);
  }

  //Calculate the index of the beginning of the type L
  //substring.

  for (i = 0; i < maxDist; i++)
  {
    if (i == 0)
    {
      j = 0;
    }
    else
    {
      j = DistCount[i-1];
    }
    while (j < DistCount[i])
    {
      ArrayA[j] = ArrayA[j] - i - 1;
      j++;
    }
  }
  return (ArrayA);
}


void sort_by_list_typeS(int* ArrayB, BooleanString& BuckB,
			int* List, const BooleanString& BuckList,
			const int inputLength, const int listLength, 
			const int ArrayBLength)
{
  int* Rev;
  int* Left;
  int i, j, newBuckNum, BucketNum;
  int BucketRight;
 
  Rev = new int[inputLength];
  Left = new int[ArrayBLength];

  ::memset((void*)Rev, -1, inputLength*sizeof(int));
  ::memset((void*)Left, -1, ArrayBLength*sizeof(int));

  // initalize Rev and Left

  BucketRight = ArrayBLength - 1;
  for (i = ArrayBLength - 1; i > 0; i--)
  {
    Rev[ArrayB[i]] = BucketRight;
    if (BuckB.getValN(i - 1) == 1)
    {
      Left[BucketRight] = i;
      BucketRight = i - 1;
    }
  }
  
  // initalize Rev and Left for the first element of ArrayB

  Rev[ArrayB[0]] = BucketRight;  
  Left[BucketRight] = 0;

  // sort the type S substrings according to the list
  // bucket by bucket.

  i = 0;
  while (i < listLength)
  {

    // count the number of elements to move in each bucket
    // and also set the value of Left for all the elements
    // that being moved.
 
    j = i;
    while (BuckList.getValN(j) == 0)
    {
      Left[Rev[List[j]]]++;
      j++;
    }

    // count for the last element of the bucket.

    Left[Rev[List[j]]]++;

    // moving the elements by re-assigning the Rev 
    // after this we still need to update Left

    j = i;
    while (BuckList.getValN(j) == 0)
    {
      newBuckNum = Left[Rev[List[j]]] - 1;
      Rev[List[j]] = newBuckNum;
      j++;
    }

    // update Rev for the last element of the bucket
  
    newBuckNum = Left[Rev[List[j]]] - 1;
    Rev[List[j]] = newBuckNum;

    // correct the values of Left for all affected
    // buckets

    j = i;
    while (BuckList.getValN(j) == 0)
    {
      newBuckNum = Rev[List[j]];
      if (Left[newBuckNum] == -1)
      {
	Left[newBuckNum] = newBuckNum;
      }
      else
      {
	Left[newBuckNum]--;
      }
      BuckB.setValN(newBuckNum, 1);
      j++;
    }

    // correct the values of Left for the last element

    newBuckNum = Rev[List[j]];
    if (Left[newBuckNum] == -1)
    {
      Left[newBuckNum] = newBuckNum;
    }
    else
    {
      Left[newBuckNum]--;
    }
    BuckB.setValN(newBuckNum, 1);

    // set i to point to the first element of the next bucket

    i = j+1;
  }
  
  // Reconsturct ArrayB from Rev.

  for (i = 0; i < inputLength; i++)
  {
    BucketNum = Rev[i];
    if (BucketNum > -1)
    {
      ArrayB[Left[BucketNum]] = i;
      Left[BucketNum]++;
    }
  }
  delete[] Rev;
  delete[] Left;      
}

void sort_by_list_typeL(int* ArrayB, BooleanString& BuckB,
			int* List, const BooleanString& BuckList,
			const int inputLength, const int listLength, 
			const int ArrayBLength)
{
  int* Rev;
  int* Right;
  int i, j, newBuckNum, BucketNum;
  int BucketLeft;
 
  Rev = new int[inputLength];
  Right = new int[ArrayBLength];

  ::memset((void*)Rev, -1, inputLength*sizeof(int));
  ::memset((void*)Right, -1, ArrayBLength*sizeof(int));

  // initalize Rev and Right

  BucketLeft = 0;
  for (i = 0; i < ArrayBLength; i++)
  {
    Rev[ArrayB[i]] = BucketLeft;
    if (BuckB.getValN(i) == 1)
    {
      Right[BucketLeft] = i;
      BucketLeft = i + 1;
    }
  }
	   
  // sort the type L substrings according to the list
  // bucket by bucket.

  i = 0;
  while (i < listLength)
  {

    // count the number of elements to move in each bucket
    // and also set the value of Right for all the elements
    // that being moved.
 
    j = i;
    while (BuckList.getValN(j) == 0)
    {
      Right[Rev[List[j]]]--;
      j++;
    }

    // count for the last element of the bucket

    Right[Rev[List[j]]]--;

    // moving the elements by re-assigning the Rev
    // after this we still need to update Right

    j = i;
    while (BuckList.getValN(j) == 0)
    {
      newBuckNum = Right[Rev[List[j]]] + 1;
      Rev[List[j]] = newBuckNum;
      j++;
    }

    // update Rev for the last element of the bucket

    newBuckNum = Right[Rev[List[j]]] + 1;
    Rev[List[j]] = newBuckNum;

    // correct the values of Right for all affected
    // buckets

    j = i;
    while (BuckList.getValN(j) == 0)
    {
      newBuckNum = Rev[List[j]];
      if (Right[newBuckNum] == -1)
      {
	Right[newBuckNum] = newBuckNum;
      }
      else
      {
	Right[newBuckNum]++;
      }
      if (newBuckNum > 0)
      {
	BuckB.setValN(newBuckNum - 1, 1);
      }
      j++;
    }

    // correct the values of Right for the last element

    newBuckNum = Rev[List[j]];
    if (Right[newBuckNum] == -1)
    {
      Right[newBuckNum] = newBuckNum;
    }
    else
    {
      Right[newBuckNum]++;
    }
    if (newBuckNum > 0)
    {
      BuckB.setValN(newBuckNum - 1, 1);
    }

    // set i to point to the first element of the next bucket

    i = j+1;
  }

  // Reconsturct ArrayB from Rev.

  for (i = 0; i < inputLength; i++)
  {
    BucketNum = Rev[i];
    if (BucketNum > -1)
    {
      ArrayB[Right[BucketNum]] = i;
      Right[BucketNum]--;
    }
  }
  delete[] Rev;
  delete[] Right;
}


void counting_sort(const int* inputString, const int inputLength, 
		   int* A, BooleanString& BuckA, int* buffer)
{
  //The int array buffer is used as a array of counters for
  //each character in the alphabet. 

  int max, min, sigmaSize;
  int Temp, i, prevCount;

  assert(inputLength > 0);

  findMaxMin(inputString, inputLength, max, min);

  sigmaSize = max - min + 1;

  ::memset((void*) buffer, 0, sizeof(int)*sigmaSize);

  //Count the number of occurences of each character;

  for (i = 0; i < inputLength; i++)
  {
    Temp = inputString[i] - min;
    buffer[Temp]++;
  }

  //Convert buffer from a list that have the count of each
  //characters in the alphabet to a list that points to the
  //left boundaries of A. So it can be used when making A. 

  prevCount = buffer[0];
  buffer[0] = 0;

  for (i = 1; i < sigmaSize; i++)
  {
    Temp = buffer[i];
    buffer[i] = prevCount + buffer[i-1];
    prevCount = Temp;
  }
 
  //Constructing A. First find out which bucket a suffix goes,
  //then using buffer to calculate where should the suffix
  //be put in A.

  for (i = 0; i < inputLength; i++)
  {
    Temp = inputString[i] - min;
    A[buffer[Temp]] = i;
    buffer[Temp]++;
  }

  //Draw the bucket boundaries. BuckA[i] is defined to be true,
  //if there is an boundary between A[i] and A[i+1].

  BuckA.setAll(false);

  for (i = 0; i < sigmaSize; i++)
  {
    BuckA.setValN(buffer[i] - 1, 1);
  }
}


void s_distance(const BooleanString& suffixType, 
		const int inputLength, int* Dist, 
		int& maxDist)
{
  int i, j;
  int temp, prevDist;

  Dist[0] = 0;
  maxDist = 0;

  //Find the first S type, everything before that have Dist[i]=0.

  i = 0;
  while (suffixType.getValN(i) == 0)
  {
    Dist[i] = 0;
    i++;
  }
  Dist[i] = 0;

  if (i < inputLength - 1)
  {
    maxDist = 1;
  }

  //Find Dist for the rest of the string.

  j = i + 1;
  prevDist = 1;

  for (i = j; i < inputLength; i++)
  {
    Dist[i] = prevDist;

    //If suffix i is type L, then increment prevDist by 1.
    //else set prevDist to 1.

    prevDist = prevDist - prevDist * suffixType.getValN(i) + 1;

    //Calculate maxDist.

    temp = maxDist - Dist[i];
    temp = temp >> MAX_INT_INDEX;
    maxDist = maxDist + ((Dist[i] - maxDist) & temp);
  }
}  


//This function computes the l_distance. 

void l_distance(const BooleanString& suffixType, 
		const int inputLength, int* Dist, 
		int& maxDist)
{
  int i, j;
  int temp, prevDist;
  
  Dist[0] = 0;
  maxDist = 0;

  //Find the first L type, everything before that have Dist[i]=0.

  i = 0;
  while (suffixType.getValN(i) == 1)
  {
    Dist[i] = 0;
    i++;
  }
  Dist[i] = 0;

  if (i < inputLength - 1)
  {
    maxDist = 1;
  }

  //Find Dist for the rest of the string.
 
  j = i + 1;
  prevDist = 1;

  for (i = j; i < inputLength; i++)
  {
    Dist[i] = prevDist;

    //If suffix i is type S, then increment prevDist by 1.
    //else set prevDist to 1.

    temp = suffixType.getValN(i) - 1;
    prevDist = prevDist - (prevDist & temp) + 1;

    //Calculate maxDist.

    temp = maxDist - Dist[i];
    temp = temp >> MAX_INT_INDEX;
    maxDist = maxDist + ((Dist[i] - maxDist) & temp);
  }
}  

void construct_distcount(const int inputLength, int* Dist, int*& DistCount, const int maxDist)
{
  int i, j;
  int prevCount, temp;

  DistCount = new int[maxDist + 1];

  for (i = 0; i <= maxDist; i++)
  {
    DistCount[i] = 0;
  }

  j = 0;
  while (Dist[j] == 0)
  {
    j++;
  }

  for(i = j; i < inputLength; i++)
  {
    DistCount[Dist[i] - 1]++;
  }

  prevCount = DistCount[0];
  DistCount[0] = 0;
  
  for (i = 1; i <= maxDist; i++)
  {
    temp = DistCount[i];
    DistCount[i] = prevCount + DistCount[i-1];
    prevCount = temp;
  } 
}  

