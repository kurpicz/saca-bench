/*
The MIT License (MIT)

Copyright (c) 2016 Michael A Maniscalco

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#define USE_EXPERIMENTAL_INVERSE_BWT


#include "./msufsort.h"
#include <include/endian.h>
#include <atomic>
#include <iostream>
#include <thread>
#include <algorithm>
#include <limits>


//==============================================================================
maniscalco::msufsort::msufsort
(
    int32_t numThreads
):
    inputBegin_(nullptr),
    inputEnd_(nullptr),
    inputSize_(),
    getValueEnd_(nullptr),
    getValueMaxIndex_(),
    copyEnd_(),
    suffixArrayBegin_(nullptr),
    suffixArrayEnd_(nullptr),
    inverseSuffixArrayBegin_(nullptr),
    inverseSuffixArrayEnd_(nullptr),
    frontBucketOffset_(),
    backBucketOffset_(new suffix_index *[0x10000]{}),
    aCount_(),
    bCount_(),
    workerThreads_(new worker_thread[numThreads - 1]),
    numWorkerThreads_(numThreads - 1)
{
}


//==============================================================================
maniscalco::msufsort::~msufsort
(
)
{
}


//==============================================================================
template <typename F, typename ... argument_types>
void maniscalco::msufsort::post_task_to_thread
(
    // private:
    // post task to the specified worker thread.  
    // if threadId == the number of worker threads then use this thread instead
    int32_t threadId,
    F && function,
    argument_types && ... arguments
)
{
    if (threadId == numWorkerThreads_)
        std::bind(std::forward<F>(function), std::forward<argument_types>(arguments) ...)();
    else
        workerThreads_[threadId].post_task(function, std::forward<argument_types>(arguments) ...);
}


//==============================================================================
void maniscalco::msufsort::wait_for_all_tasks_completed
(
    // private:
    // wait for all currently posted tasks to be completed.
) const
{
    for (auto threadId = 0; threadId < numWorkerThreads_; ++threadId)
        workerThreads_[threadId].wait();
}


//==============================================================================
inline auto maniscalco::msufsort::get_suffix_type
(
    uint8_t const * suffix
) -> suffix_type 
{
    if ((suffix + 1) >= inputEnd_)
        return a;
    if (suffix[0] >= suffix[1])
    {
        auto p = suffix + 1;
        while ((p < inputEnd_) && (*p == suffix[0]))
            ++p;
        if ((p == inputEnd_) || (suffix[0] > p[0]))
            return a;
        return b;
    }
    auto p = suffix + 2;
    while ((p < inputEnd_) && (*p == suffix[1]))
        ++p;
    if ((p == inputEnd_) || (suffix[1] > p[0]))
        return bStar;
    return b;
}


//==============================================================================
inline uint64_t maniscalco::msufsort::get_value
(
    uint8_t const * inputCurrent,
    suffix_index index
) const
{
    inputCurrent += (index & sa_index_mask);
    if (inputCurrent >= getValueEnd_)
    {
        if (inputCurrent >= inputEnd_)
            return 0;
        inputCurrent = (copyEnd_ + (sizeof(uint64_t) - std::distance(inputCurrent, inputEnd_)));
    }
    return endian_swap<host_order_type, big_endian_type>(*(uint64_t const *)(inputCurrent));
}


//==============================================================================
inline bool maniscalco::msufsort::compare_suffixes
(
    uint8_t const * inputBegin,
    suffix_index indexA,
    suffix_index indexB
) const
{
    indexA &= sa_index_mask;
    indexB &= sa_index_mask;

    auto valueA = get_value(inputBegin, indexA);
    auto valueB = get_value(inputBegin, indexB);
    while (valueA == valueB)
    {
        inputBegin += sizeof(valueA);
        valueA = get_value(inputBegin, indexA);
        valueB = get_value(inputBegin, indexB);
    }
    return (valueA > valueB);
}


//==============================================================================
inline void maniscalco::msufsort::insertion_sort
(
    // private:
    // sorts the suffixes by insertion sort
    suffix_index * partitionBegin,
    suffix_index * partitionEnd,
    int32_t currentMatchLength,
    uint64_t startingPattern
)
{
    struct partition_info
    {
        int32_t matchLength_;
        int32_t size_;
    };

    partition_info partitionStack[insertion_sort_threshold];
    partitionStack[0] = {currentMatchLength, (int32_t)std::distance(partitionBegin, partitionEnd)};
    int32_t stackSize = 1;

    while (stackSize > 0)
    {
        auto & partitionInfo = partitionStack[--stackSize];
        auto matchLength = partitionInfo.matchLength_;
        auto size = partitionInfo.size_;

        if (size <= 2)
        {
            if (size == 1)
            {
                ++partitionBegin;
            }
            else
            {
                if (compare_suffixes(inputBegin_, partitionBegin[0], partitionBegin[1]) > 0)
                    std::swap(partitionBegin[0], partitionBegin[1]);
                partitionBegin += 2;
            }
            continue;
        }

        if (tandemRepeatSortEnabled_)
        {

            bool potentialTandemRepeats = false;
            int64_t suffixIndex = ((*partitionBegin) & sa_index_mask);
            int64_t curIndex = (suffixIndex + currentMatchLength);
            int64_t endIndex = (curIndex - ((currentMatchLength > 2) ? sizeof(uint64_t) : 2));
            while ((!potentialTandemRepeats) && (--curIndex >= endIndex))
                potentialTandemRepeats = (get_value(inputBegin_, curIndex) == startingPattern);
            if (potentialTandemRepeats)
            {
                // potential for tandem repeats found within this partition
                if (tandem_repeat_sort(partitionBegin, partitionBegin + size, matchLength, startingPattern))
                {
                    partitionBegin += size;
                    continue;
                }
            }
        }

        uint64_t value[insertion_sort_threshold];
        value[0] = get_value(inputBegin_ + matchLength, partitionBegin[0]);
        for (int32_t i = 1; i < size; ++i)
        {
            auto currentIndex = partitionBegin[i];
            uint64_t currentValue = get_value(inputBegin_ + matchLength, partitionBegin[i]);
            auto j = i;
            while ((j > 0) && (value[j - 1] > currentValue))
            {
                value[j] = value[j - 1];
                partitionBegin[j] = partitionBegin[j - 1];
                --j;
            }
            value[j] = currentValue;
            partitionBegin[j] = currentIndex;
        }

        int32_t i = size - 1;
        while (i >= 0)
        {
            int32_t start = i--;
            while ((i >= 0) && (value[i] == value[start]))
                --i;
            partitionStack[stackSize++] = partition_info{matchLength + (int32_t)sizeof(uint64_t), start - i};
        }
    }
}


//==============================================================================
bool maniscalco::msufsort::tandem_repeat_sort
(
    // private:
    // the tandem repeat sort.  determines if the suffixes provided are tandem repeats
    // of other suffixes from within the same group.  If so, sorts the non tandem
    // repeat suffixes and then induces the sorted order of the suffixes which are
    // tandem repeats.
    suffix_index * partitionBegin,
    suffix_index * partitionEnd,
    int32_t currentMatchLength,
    uint64_t startingPattern
)
{
    std::sort(partitionBegin, partitionEnd, [](suffix_index a, suffix_index b)->bool{return ((a & sa_index_mask) < (b & sa_index_mask));});
    int32_t tandemRepeatLength = 0;
    int32_t previousSuffixIndex = std::numeric_limits<int32_t>::max();
    suffix_index * terminatorsBegin = partitionEnd;

    for (auto cur = partitionEnd - 1; cur >= partitionBegin; --cur)
    {
	    auto currentSuffixIndex = (*cur & sa_index_mask);
	    auto distanceBetweenSuffixes = (previousSuffixIndex - currentSuffixIndex);
	    if (distanceBetweenSuffixes < (currentMatchLength >> 1))
	    {
		    // suffix is a tandem repeat
		    tandemRepeatLength = distanceBetweenSuffixes;
            auto flags = (inverseSuffixArrayBegin_[currentSuffixIndex >> 1] & isa_flag_mask);
            inverseSuffixArrayBegin_[currentSuffixIndex >> 1] = (tandemRepeatLength | flags | is_tandem_repeat_flag);
	    }
	    else
	    {
		    // Suffix is a terminator
		    *(--terminatorsBegin) = *cur;
	    }
	    previousSuffixIndex = currentSuffixIndex;
    }
    if (tandemRepeatLength == 0)
        return false;

    // tandem repeats found
    // first we sort all of the tandem repeat terminator suffixes
    int32_t numTerminators = (int64_t)std::distance(terminatorsBegin, partitionEnd);
    multikey_quicksort(terminatorsBegin, partitionEnd, currentMatchLength, startingPattern);

    // now use sorted order of terminators to determine sorted order of repeats.
    // figure out how many terminators sort before the repeat and how
    // many sort after the repeat.  put them on left and right extremes of the array.
    int32_t m = 0;
    int32_t a = 0;
    int32_t b = numTerminators - 1;
    int32_t numTypeA = 0;
    while (a <= b)
    {
	    m = (a + b) >> 1;
        if (!compare_suffixes(inputBegin_, terminatorsBegin[m], terminatorsBegin[m] + tandemRepeatLength))
	    {
		    numTypeA = m;
		    b = m - 1;
	    }
	    else
	    {
		    numTypeA = m + 1;
		    a = m + 1;
	    }
    }
    if (numTypeA > numTerminators)
	    numTypeA = numTerminators;
    int32_t numTypeB = (numTerminators - numTypeA);

    for (int32_t i = 0; i < numTypeA; ++i)
	    partitionBegin[i] = terminatorsBegin[i];

    // type A repeats
    auto current = partitionBegin;
    auto currentEnd = current + numTypeA;
    auto next = currentEnd;
    while (current != currentEnd)
    { 
        while (current != currentEnd)
        {
            auto index = (*current++ & sa_index_mask);
            if (index >= tandemRepeatLength)
            {
                auto potentialTandemRepeatIndex = index - tandemRepeatLength;
                auto isaValue = inverseSuffixArrayBegin_[potentialTandemRepeatIndex >> 1];
                if ((isaValue & is_tandem_repeat_flag) && ((isaValue & isa_index_mask) == tandemRepeatLength))
                {     
                    auto flag = ((potentialTandemRepeatIndex > 0) && (inputBegin_[potentialTandemRepeatIndex - 1] <= inputBegin_[potentialTandemRepeatIndex])) ? 0 : preceding_suffix_is_type_a_flag;
                    *(next++) = (potentialTandemRepeatIndex | flag);
                    inverseSuffixArrayBegin_[potentialTandemRepeatIndex >> 1] &= ~is_tandem_repeat_flag;
                }           
            }
        }
        currentEnd = next;
    }

    // type B repeats
    current = partitionEnd - 1;
    currentEnd = current - numTypeB;
    next = currentEnd;
    while (current != currentEnd)
    { 
        while (current != currentEnd)
        {
            auto index = (*current-- & sa_index_mask);
            if (index >= tandemRepeatLength)
            {
                auto potentialTandemRepeatIndex = index - tandemRepeatLength;
                auto isaValue = inverseSuffixArrayBegin_[potentialTandemRepeatIndex >> 1];
                if ((isaValue & is_tandem_repeat_flag) && ((isaValue & isa_index_mask) == tandemRepeatLength))
                {     
                    auto flag = ((potentialTandemRepeatIndex > 0) && (inputBegin_[potentialTandemRepeatIndex - 1] <= inputBegin_[potentialTandemRepeatIndex])) ? 0 : preceding_suffix_is_type_a_flag;
                    *(next--) = (potentialTandemRepeatIndex | flag);
                    inverseSuffixArrayBegin_[potentialTandemRepeatIndex >> 1] &= ~is_tandem_repeat_flag;
                }           
            }
        }
        currentEnd = next;
    }
    return true;
}


//==============================================================================
void maniscalco::msufsort::multikey_quicksort
(
    // private:
    // multi key quicksort on the input data provided
    suffix_index * suffixArrayBegin,
    suffix_index * suffixArrayEnd,
    int32_t currentMatchLength,
    uint64_t startingPattern
)
{
    auto partitionSize = std::distance(suffixArrayBegin, suffixArrayEnd);
    if (partitionSize < insertion_sort_threshold)
    {
        if (partitionSize > 1)
		    insertion_sort(suffixArrayBegin, suffixArrayEnd, currentMatchLength, startingPattern);
        return;
    }

    // TODO: stack size is fixed.  make it dynamic in the unlikely case of overflow.
    auto partitionStackSize = (1 << 15);
    partition_info partitionStack[partitionStackSize];
    auto partitionStackTop = partitionStack;
    auto partitionStackEnd = (partitionStack + partitionStackSize);

    auto partitionBegin = suffixArrayBegin;
    auto partitionEnd = suffixArrayEnd;
    auto potentialTandemRepeats = false;

    while (true)
    {
        auto partitionSize = std::distance(partitionBegin, partitionEnd);
        if (partitionSize < insertion_sort_threshold)
        {
            if (partitionSize > 1)
    		    insertion_sort(partitionBegin, partitionEnd, currentMatchLength, startingPattern);
            partitionBegin = partitionEnd;
        }
        else
        {
	        if ((tandemRepeatSortEnabled_) && (potentialTandemRepeats))
	        {
                potentialTandemRepeats = false;
		        if (!tandem_repeat_sort(partitionBegin, partitionEnd, currentMatchLength, startingPattern))
                    continue;
			    partitionBegin = partitionEnd;
	        }
            else
            {
                // median indexes
                auto offsetInputBegin = inputBegin_ + currentMatchLength;
                auto oneSixthOfPartitionSize = partitionSize / 6;
                auto pivotCandidate1 = partitionBegin + oneSixthOfPartitionSize;
                auto pivotCandidate2 = pivotCandidate1 + oneSixthOfPartitionSize;
                auto pivotCandidate3 = pivotCandidate2 + oneSixthOfPartitionSize;
                auto pivotCandidate4 = pivotCandidate3 + oneSixthOfPartitionSize;
                auto pivotCandidate5 = pivotCandidate4 + oneSixthOfPartitionSize;

                auto pivotCandidateValue1 = get_value(offsetInputBegin, *pivotCandidate1);
                auto pivotCandidateValue2 = get_value(offsetInputBegin, *pivotCandidate2);
                auto pivotCandidateValue3 = get_value(offsetInputBegin, *pivotCandidate3);
                auto pivotCandidateValue4 = get_value(offsetInputBegin, *pivotCandidate4);
                auto pivotCandidateValue5 = get_value(offsetInputBegin, *pivotCandidate5);

                if (pivotCandidateValue1 > pivotCandidateValue2)
	                std::swap(*pivotCandidate1, *pivotCandidate2), std::swap(pivotCandidateValue1, pivotCandidateValue2);
                if (pivotCandidateValue4 > pivotCandidateValue5)
	                std::swap(*pivotCandidate4, *pivotCandidate5), std::swap(pivotCandidateValue4, pivotCandidateValue5);
                if (pivotCandidateValue1 > pivotCandidateValue3)
	                std::swap(*pivotCandidate1, *pivotCandidate3), std::swap(pivotCandidateValue1, pivotCandidateValue3);
                if (pivotCandidateValue2 > pivotCandidateValue3)
	                std::swap(*pivotCandidate2, *pivotCandidate3), std::swap(pivotCandidateValue2, pivotCandidateValue3);
                if (pivotCandidateValue1 > pivotCandidateValue4)
	                std::swap(*pivotCandidate1, *pivotCandidate4), std::swap(pivotCandidateValue1, pivotCandidateValue4);
                if (pivotCandidateValue3 > pivotCandidateValue4)
	                std::swap(*pivotCandidate3, *pivotCandidate4), std::swap(pivotCandidateValue3, pivotCandidateValue4);
                if (pivotCandidateValue2 > pivotCandidateValue5)
	                std::swap(*pivotCandidate2, *pivotCandidate5), std::swap(pivotCandidateValue2, pivotCandidateValue5);
                if (pivotCandidateValue2 > pivotCandidateValue3)
	                std::swap(*pivotCandidate2, *pivotCandidate3), std::swap(pivotCandidateValue2, pivotCandidateValue3);
                if (pivotCandidateValue4 > pivotCandidateValue5)
	                std::swap(*pivotCandidate4, *pivotCandidate5), std::swap(pivotCandidateValue4, pivotCandidateValue5);

                auto pivot1 = pivotCandidateValue1;
                auto pivot2 = pivotCandidateValue3;
                auto pivot3 = pivotCandidateValue5;

                auto ptrA = partitionBegin;
                auto ptrB = partitionBegin;
                auto ptrC = partitionBegin;
                auto partitionBack = partitionEnd - 1;
                auto ptrD = partitionBack;
                auto ptrE = partitionBack;
                auto ptrF = partitionBack;
                auto ptr = partitionBegin;

                std::swap(*ptr++, *pivotCandidate1);
                ptrC += (pivot1 != pivot2);
                ptrB += (pivot1 != pivot2);
                std::swap(*ptr++, *pivotCandidate3);
                if (pivot2 != pivot3)
                {
	                std::swap(*ptrD--, *pivotCandidate5);
	                --ptrE;
                }			

                while (ptr < ptrD)
                {
	                auto temp = get_value(offsetInputBegin, *ptr);
	                if (temp <= pivot2)
	                {
		                if (temp < pivot2)
		                {
			                std::swap(*ptrC++, *ptr);
			                if (temp <= pivot1)
			                {
				                if (temp < pivot1)
					                std::swap(*ptrA++, *(ptrC - 1));
				                std::swap(*ptrB++, *(ptrC - 1));
			                }
		                }
		                ++ptr;
	                }
	                else
	                {
		                std::swap(*ptrD--, *ptr);
		                if (temp >= pivot3)
		                {
			                if (temp > pivot3)
				                std::swap(*(ptrD + 1), *ptrF--);
			                std::swap(*(ptrD + 1), *ptrE--);
		                }
	                }
                }

                if (ptr == ptrD)
                {
	                auto temp = get_value(offsetInputBegin, *ptr);
	                if (temp <= pivot2)
	                {
		                if (temp < pivot2)
		                {
			                std::swap(*ptrC++, *ptr);
			                if (temp <= pivot1)
			                {
				                if (temp < pivot1)
					                std::swap(*ptrA++, *(ptrC - 1));
				                std::swap(*ptrB++, *(ptrC - 1));
			                }
		                }
		                ++ptr;
	                }
	                else
	                {
		                if (temp >= pivot3)
		                {
			                if (temp == pivot3)
			                {
				                std::swap(*ptrD, *ptrE);
			                }
			                else
			                {
				                auto temp2 = *ptr;
				                *ptrD = *ptrE;
				                *ptrE = *ptrF;
				                *ptrF-- = temp2;
			                }
			                --ptrE;
		                }
		                --ptrD;
	                }
                }

                int32_t szNextMatchLength = (currentMatchLength + sizeof(uint64_t));
                if (partitionBack != ptrF)
	                *partitionStackTop++ = partition_info{(int32_t)std::distance(ptrF, partitionBack), currentMatchLength, startingPattern, false};

                if (ptrE != ptrF)
                {
                    if (currentMatchLength == 2)
                        startingPattern = get_value(inputBegin_, *ptrE);
                    potentialTandemRepeats = false;
                    int64_t suffixIndex = ((*ptrE) & sa_index_mask);
                    int64_t curIndex = (suffixIndex + currentMatchLength);
                    int64_t endIndex = (curIndex - ((currentMatchLength > 2) ? sizeof(uint64_t) : 2));
                    while ((!potentialTandemRepeats) && (--curIndex >= endIndex))
                        potentialTandemRepeats = (get_value(inputBegin_, curIndex) == startingPattern);
	                *partitionStackTop++ = partition_info{(int32_t)std::distance(ptrE, ptrF), szNextMatchLength, startingPattern, potentialTandemRepeats};
                }

                if (ptrD != ptrE)
	                *partitionStackTop++ = partition_info{(int32_t)std::distance(ptrD, ptrE), currentMatchLength, startingPattern, false};

                if (ptrC <= ptrD)
                {
                    if (currentMatchLength == 2)
                        startingPattern = get_value(inputBegin_, *ptrC);
                    potentialTandemRepeats = false;
                    int64_t suffixIndex = ((*ptrC) & sa_index_mask);
                    int64_t curIndex = (suffixIndex + currentMatchLength);
                    int64_t endIndex = (curIndex - ((currentMatchLength > 2) ? sizeof(uint64_t) : 2));
                    while ((!potentialTandemRepeats) && (--curIndex >= endIndex))
                        potentialTandemRepeats = (get_value(inputBegin_, curIndex) == startingPattern);
	                *partitionStackTop++ = partition_info{(int32_t)std::distance(ptrC, ptrD) + 1, szNextMatchLength, startingPattern, potentialTandemRepeats};
                }

                if (ptrC != ptrB)
	                *partitionStackTop++ = partition_info{(int32_t)std::distance(ptrB, ptrC), currentMatchLength, startingPattern, false};

                if (ptrA != ptrB)
                {
                    if (currentMatchLength == 2)
                        startingPattern = get_value(inputBegin_, *ptrA);
                    potentialTandemRepeats = false;
                    int64_t suffixIndex = ((*ptrA) & sa_index_mask);
                    int64_t curIndex = (suffixIndex + currentMatchLength);
                    int64_t endIndex = (curIndex - ((currentMatchLength > 2) ? sizeof(uint64_t) : 2));
                    while ((!potentialTandemRepeats) && (--curIndex >= endIndex))
                        potentialTandemRepeats = (get_value(inputBegin_, curIndex) == startingPattern);
	                *partitionStackTop++ = partition_info{(int32_t)std::distance(ptrA, ptrB), szNextMatchLength, startingPattern, potentialTandemRepeats};
                }

                if (partitionBegin != ptrA)
	                *partitionStackTop++ = partition_info{(int32_t)std::distance(partitionBegin, ptrA), currentMatchLength, startingPattern, false};
            }
        }

        if (partitionStackTop == partitionStack)
	        break;	// sorted completed

        if (partitionStackTop >= partitionStackEnd)
        {
            // TODO: partition stack overflow ...
            std::cout << "msufsort: multikey quicksort stack overflow" << std::endl;
            throw std::exception();
        }

        auto partitionInfo = *--partitionStackTop;
        partitionEnd = partitionBegin + partitionInfo.size_;
        currentMatchLength = partitionInfo.matchLength_;
        startingPattern = partitionInfo.startingPattern_;
        potentialTandemRepeats = partitionInfo.potentialTandemRepeats_;
    }			
}


//==============================================================================
void maniscalco::msufsort::second_stage_its_right_to_left_pass_multi_threaded
(
    // private:
    // induce sorted position of B suffixes from sorted B* suffixes
    // This is the first half of the second stage of the ITS ... the 'right to left' pass
)
{
    auto numThreads = (int32_t)(numWorkerThreads_ + 1); // +1 for main thread
    auto max_cache_size = (1 << 12);
    struct entry_type
    {
        uint8_t precedingSuffix_;
        int32_t precedingSuffixIndex_;
    };
    std::unique_ptr<entry_type []> cache[numThreads];
    for (auto i = 0; i < numThreads; ++i)
        cache[i].reset(new entry_type[max_cache_size]);
    int32_t numSuffixes[numThreads] = {};
    int32_t sCount[numThreads][0x100] = {};
    suffix_index * dest[numThreads][0x100] = {};

    auto currentSuffix = suffixArrayBegin_ + inputSize_;
    for (auto symbol = 0xff; symbol >= 0; --symbol)
    {
        auto backBucketOffset = &backBucketOffset_[symbol << 8];
        auto endSuffix = currentSuffix - bCount_[symbol];
        
        while (currentSuffix > endSuffix)
        {
            // determine how many B/B* suffixes are safe to process during this pass
            auto maxEnd = currentSuffix - (max_cache_size * numThreads);
            if (maxEnd < suffixArrayBegin_)
                maxEnd = suffixArrayBegin_;
            if (maxEnd < endSuffix)
                maxEnd = endSuffix;
            auto temp = currentSuffix;
            while ((temp > maxEnd) && (*temp != suffix_is_unsorted_b_type))
                --temp;
            auto totalSuffixesPerThread = ((std::distance(temp, currentSuffix) + numThreads - 1) / numThreads);

            // process suffixes
            for (auto threadId = 0; threadId < numThreads; ++threadId)
            {
                numSuffixes[threadId] = 0;
                auto endForThisThread = currentSuffix - totalSuffixesPerThread;
                if (endForThisThread < temp)
                    endForThisThread = temp;
                post_task_to_thread
                (
                    threadId, 
                    [](
                        uint8_t const * inputBegin,
                        suffix_index * begin,
                        suffix_index * end,
                        entry_type * cache,
                        int32_t & numSuffixes,
                        int32_t * suffixCount
                    )
                    {
                        auto curCache = cache;
                        ++begin;
                        uint8_t currentPrecedingSymbol = 0;
                        int32_t currentPrecedingSymbolCount = 0;
                        while (--begin > end)
                        {
                            if ((*begin & preceding_suffix_is_type_a_flag) == 0)
                            {
                                int32_t precedingSuffixIndex = ((*begin & sa_index_mask) - 1);
                                auto precedingSuffix = (inputBegin + precedingSuffixIndex);
                                auto precedingSymbol = precedingSuffix[0];
                                int32_t flag = ((precedingSuffixIndex > 0) && (precedingSuffix[-1] <= precedingSymbol)) ? 0 : preceding_suffix_is_type_a_flag;
                                *curCache++ = {precedingSymbol, precedingSuffixIndex | flag};
                                if (precedingSymbol != currentPrecedingSymbol)
                                {
                                    suffixCount[currentPrecedingSymbol] += currentPrecedingSymbolCount;
                                    currentPrecedingSymbol = precedingSymbol;
                                    currentPrecedingSymbolCount = 0;
                                }
                                ++currentPrecedingSymbolCount;
                            }
                        }
                        suffixCount[currentPrecedingSymbol] += currentPrecedingSymbolCount;
                        numSuffixes = std::distance(cache, curCache);
                    }, inputBegin_, currentSuffix, endForThisThread, cache[threadId].get(), std::ref(numSuffixes[threadId]), sCount[threadId]
                );
                currentSuffix = endForThisThread;
            }
            wait_for_all_tasks_completed();

            // 
            for (auto threadId = 0, begin = 0, numSymbolsPerThread = ((0x100 + numThreads - 1) / numThreads); threadId < numThreads; ++threadId)
            {
                auto end = begin + numSymbolsPerThread;
                if (end > 0x100)
                    end = 0x100;
                post_task_to_thread
                (
                    threadId, 
                    [&dest, &backBucketOffset, &sCount, numThreads]
                    (
                        int32_t begin,
                        int32_t end
                    )
                    {
                        for (auto threadId = 0; threadId < numThreads; ++threadId)
                            for (auto symbol = begin; symbol < end; ++symbol)
                            {
                                dest[threadId][symbol] = backBucketOffset[symbol];
                                backBucketOffset[symbol] -= sCount[threadId][symbol];
                                sCount[threadId][symbol] = 0;
                            }
                    }, begin, end
                );
                begin = end;
            }
            wait_for_all_tasks_completed();

            //
            for (auto threadId = 0; threadId < numThreads; ++threadId)
                post_task_to_thread
                (
                    threadId,
                    [&](
                        suffix_index * dest[0x100],
                        entry_type const * begin,
                        entry_type const * end
                    )
                    {
                        --begin;
                        while (++begin < end)
                            *(--dest[begin->precedingSuffix_]) = begin->precedingSuffixIndex_;
                    }, 
                    dest[threadId], cache[threadId].get(), cache[threadId].get() + numSuffixes[threadId]
                );
            wait_for_all_tasks_completed();
        }
        currentSuffix -= aCount_[symbol];
    }
}


//==============================================================================
void maniscalco::msufsort::second_stage_its_right_to_left_pass_single_threaded
(
    // private:
    // induce sorted position of B suffixes from sorted B* suffixes
    // This is the first half of the second stage of the ITS ... the 'right to left' pass
)
{
    auto currentSuffix = suffixArrayBegin_ + inputSize_;
    for (auto i = 0xff; i >= 0; --i)
    {
        auto backBucketOffset = &backBucketOffset_[i << 8];
        auto prevWrite = backBucketOffset;
        int32_t previousPrecedingSymbol = 0;
        auto endSuffix = currentSuffix - bCount_[i];
        while (currentSuffix > endSuffix)
        {
            if ((*currentSuffix & preceding_suffix_is_type_a_flag) == 0)
            {
                int32_t precedingSuffixIndex = ((*currentSuffix & sa_index_mask) - 1);
                auto precedingSuffix = (inputBegin_ + precedingSuffixIndex);
                auto precedingSymbol = precedingSuffix[0];
                int32_t flag = ((precedingSuffixIndex > 0) && (precedingSuffix[-1] <= precedingSymbol)) ? 0 : preceding_suffix_is_type_a_flag;
                if (precedingSymbol != previousPrecedingSymbol)
                {
                    previousPrecedingSymbol = precedingSymbol;
                    prevWrite = backBucketOffset + previousPrecedingSymbol;
                }
                *(--*prevWrite) = (precedingSuffixIndex | flag);
            }
            --currentSuffix;
        }
        currentSuffix -= aCount_[i];
    }
}


//==============================================================================
void maniscalco::msufsort::second_stage_its_left_to_right_pass_single_threaded
(
    // private:
    // induce sorted position of A suffixes from sorted B suffixes
    // This is the second half of the second stage of the ITS ... the 'left to right' pass
)
{
    auto currentSuffix = suffixArrayBegin_ - 1;
    uint8_t previousPrecedingSymbol = 0;
    auto previousFrontBucketOffset = frontBucketOffset_;
    while (++currentSuffix < suffixArrayEnd_)
    {
        auto currentSuffixIndex = *currentSuffix;
        if (currentSuffixIndex & preceding_suffix_is_type_a_flag)
        {
            if ((currentSuffixIndex & sa_index_mask) != 0)
            {
                int32_t precedingSuffixIndex = ((currentSuffixIndex & sa_index_mask) - 1);
                auto precedingSuffix = (inputBegin_ + precedingSuffixIndex);
                auto precedingSymbol = precedingSuffix[0];
                int32_t flag = ((precedingSuffixIndex > 0) && (precedingSuffix[-1] >= precedingSymbol)) ? preceding_suffix_is_type_a_flag : 0;
                if (precedingSymbol != previousPrecedingSymbol)
                {
                    previousPrecedingSymbol = precedingSymbol;
                    previousFrontBucketOffset = frontBucketOffset_ + previousPrecedingSymbol;
                }
                *((*previousFrontBucketOffset)++) = (precedingSuffixIndex | flag);
            }
            *(currentSuffix) &= sa_index_mask;
        }
    }
}


//==============================================================================
void maniscalco::msufsort::second_stage_its_left_to_right_pass_multi_threaded
(
    // private:
    // induce sorted position of A suffixes from sorted B suffixes
    // This is the second half of the second stage of the ITS ... the 'left to right' pass
)
{
    auto numThreads = (int32_t)(numWorkerThreads_ + 1); // +1 for main thread
    auto currentSuffix = suffixArrayBegin_;
    auto max_cache_size = (1 << 12);
    struct entry_type
    {
        uint8_t precedingSuffix_;
        int32_t precedingSuffixIndex_;
    };
    std::unique_ptr<entry_type []> cache[numThreads];
    for (auto i = 0; i < numThreads; ++i)
        cache[i].reset(new entry_type[max_cache_size]);
    int32_t numSuffixes[numThreads] = {};
    int32_t sCount[numThreads][0x100] = {};
    suffix_index * dest[numThreads][0x100] = {};

    while (currentSuffix < suffixArrayEnd_)
    {
        // calculate current 'safe' suffixes to process
        while ((currentSuffix < suffixArrayEnd_) && (!(*currentSuffix & preceding_suffix_is_type_a_flag)))
            ++currentSuffix;
        if (currentSuffix >= suffixArrayEnd_)
            break;

        auto begin = currentSuffix;
        auto maxEnd = begin + (max_cache_size * numThreads);
        if (maxEnd > suffixArrayEnd_)
            maxEnd = suffixArrayEnd_;
        currentSuffix += (currentSuffix != maxEnd);
        while ((currentSuffix != maxEnd) && (*currentSuffix != (int32_t)0x80000000))
            ++currentSuffix;
        auto end = currentSuffix;
        auto totalSuffixes = std::distance(begin, end);
        auto totalSuffixesPerThread = ((totalSuffixes + numThreads - 1) / numThreads);

        // process suffixes
        for (auto threadId = 0; threadId < numThreads; ++threadId)
        {
            numSuffixes[threadId] = 0;
            auto endForThisThread = begin + totalSuffixesPerThread;
            if (endForThisThread > end)
                endForThisThread = end;
            post_task_to_thread
            (
                threadId, 
                [](
                    uint8_t const * inputBegin,
                    suffix_index * begin,
                    suffix_index * end,
                    entry_type * cache,
                    int32_t & numSuffixes,
                    int32_t * suffixCount
                )
                {
                    auto current = begin;
                    auto curCache = cache;
                    --current;
                    uint8_t currentPrecedingSymbol = 0;
                    int32_t currentPrecedingSymbolCount = 0;
                    while (++current != end)
                    {
                        auto currentSuffixIndex = *current;
                        if (currentSuffixIndex & preceding_suffix_is_type_a_flag)
                        {
                            currentSuffixIndex &= sa_index_mask;
                            if (currentSuffixIndex != 0)
                            {
                                int32_t precedingSuffixIndex = (currentSuffixIndex - 1);
                                auto precedingSuffix = (inputBegin + precedingSuffixIndex);
                                auto precedingSymbol = precedingSuffix[0];
                                int32_t flag = ((precedingSuffixIndex > 0) && (precedingSuffix[-1] >= precedingSymbol)) ? preceding_suffix_is_type_a_flag : 0;
                                *curCache++ = {precedingSymbol, precedingSuffixIndex | flag};
                                if (precedingSymbol != currentPrecedingSymbol)
                                {
                                    suffixCount[currentPrecedingSymbol] += currentPrecedingSymbolCount;
                                    currentPrecedingSymbol = precedingSymbol;
                                    currentPrecedingSymbolCount = 0;
                                }
                                ++currentPrecedingSymbolCount;
                            }
                            *current = currentSuffixIndex;
                        }
                    }
                    suffixCount[currentPrecedingSymbol] += currentPrecedingSymbolCount;
                    numSuffixes = std::distance(cache, curCache);
                }, inputBegin_, begin, endForThisThread, cache[threadId].get(), std::ref(numSuffixes[threadId]), sCount[threadId]
            );
            begin = endForThisThread;
        }
        wait_for_all_tasks_completed();

        //
        for (auto threadId = 0, begin = 0, numSymbolsPerThread = ((0x100 + numThreads - 1) / numThreads); threadId < numThreads; ++threadId)
        {
            auto end = begin + numSymbolsPerThread;
            if (end > 0x100)
                end = 0x100;
            post_task_to_thread
            (
                threadId, 
                [&dest, this, &sCount, numThreads]
                (
                    int32_t begin,
                    int32_t end
                )
                {
                    for (auto threadId = 0; threadId < numThreads; ++threadId)
                        for (auto symbol = begin; symbol < end; ++symbol)
                        {
                            dest[threadId][symbol] = frontBucketOffset_[symbol];
                            frontBucketOffset_[symbol] += sCount[threadId][symbol];
                            sCount[threadId][symbol] = 0;
                        }
                }, begin, end
            );
            begin = end;
        }
        wait_for_all_tasks_completed();

        //
        for (auto threadId = 0; threadId < numThreads; ++threadId)
            post_task_to_thread
            (
                threadId,
                [](
                    suffix_index * dest[0x100],
                    entry_type const * begin,
                    entry_type const * end
                )
                {
                    --begin;
                    while (++begin != end)
                        *(dest[begin->precedingSuffix_]++) = begin->precedingSuffixIndex_;
                }, 
                dest[threadId], cache[threadId].get(), cache[threadId].get() + numSuffixes[threadId]
            );
        wait_for_all_tasks_completed();
    }
}


//==============================================================================
void maniscalco::msufsort::second_stage_its
(
    // private:
    // performs the the second stage of the improved two stage sort.
)
{
    if (numWorkerThreads_ == 0)
    {
        auto start = std::chrono::system_clock::now();
        second_stage_its_right_to_left_pass_single_threaded();
        auto finish = std::chrono::system_clock::now();
        #ifdef VERBOSE
            std::cout << "second stage right to left pass time: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() << " ms " << std::endl;
        #endif
        start = std::chrono::system_clock::now();
        second_stage_its_left_to_right_pass_single_threaded();
        finish = std::chrono::system_clock::now();
        #ifdef VERBOSE
            std::cout << "second stage left to right pass time: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() << " ms " << std::endl;
        #endif
    }
    else
    {
        auto start = std::chrono::system_clock::now();
        second_stage_its_right_to_left_pass_multi_threaded();
        auto finish = std::chrono::system_clock::now();
        #ifdef VERBOSE
            std::cout << "second stage right to left pass time: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() << " ms " << std::endl;
        #endif
        start = std::chrono::system_clock::now();
        second_stage_its_left_to_right_pass_multi_threaded();
        finish = std::chrono::system_clock::now();
        #ifdef VERBOSE
            std::cout << "second stage left to right pass time: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() << " ms " << std::endl;
        #endif
    }
}


//==============================================================================
void maniscalco::msufsort::second_stage_its_as_burrows_wheeler_transform_right_to_left_pass_single_threaded
(
    // private:
    // induce sorted position of B suffixes from sorted B* suffixes
    // This is the first half of the second stage of the ITS ... the 'right to left' pass
)
{
    auto currentSuffix = suffixArrayBegin_ + inputSize_;
    for (auto i = 0xff; i >= 0; --i)
    {
        auto backBucketOffset = &backBucketOffset_[i << 8];
        auto prevWrite = backBucketOffset;
        int32_t previousPrecedingSymbol = 0;
        auto endSuffix = currentSuffix - bCount_[i];
        while (currentSuffix > endSuffix)
        {
            int32_t precedingSuffixIndex = ((*currentSuffix & sa_index_mask) - 1);
            auto precedingSuffix = (inputBegin_ + precedingSuffixIndex);
            auto precedingSymbol = precedingSuffix[0];
            if ((*currentSuffix & preceding_suffix_is_type_a_flag) == 0)
            {
                int32_t flag = ((precedingSuffixIndex > 0) && (precedingSuffix[-1] <= precedingSymbol)) ? 0 : preceding_suffix_is_type_a_flag;
                if (precedingSymbol != previousPrecedingSymbol)
                {
                    previousPrecedingSymbol = precedingSymbol;
                    prevWrite = backBucketOffset + previousPrecedingSymbol;
                }
                *(--*prevWrite) = (precedingSuffixIndex | flag);
                if (precedingSuffix >= inputBegin_)
                    *currentSuffix = precedingSymbol;
            }
            --currentSuffix;
        }
        currentSuffix -= aCount_[i];
    }
}


//==============================================================================
void maniscalco::msufsort::second_stage_its_as_burrows_wheeler_transform_right_to_left_pass_multi_threaded
(
    // private:
    // induce sorted position of B suffixes from sorted B* suffixes
    // This is the first half of the second stage of the ITS ... the 'right to left' pass
)
{
    auto numThreads = (int32_t)(numWorkerThreads_ + 1); // +1 for main thread
    auto max_cache_size = (1 << 12);
    struct entry_type
    {
        uint8_t precedingSuffix_;
        int32_t precedingSuffixIndex_;
    };
    std::unique_ptr<entry_type []> cache[numThreads];
    for (auto i = 0; i < numThreads; ++i)
        cache[i].reset(new entry_type[max_cache_size]);
    int32_t numSuffixes[numThreads] = {};
    int32_t sCount[numThreads][0x100] = {};
    suffix_index * dest[numThreads][0x100] = {};

    auto currentSuffix = suffixArrayBegin_ + inputSize_;
    for (auto symbol = 0xff; symbol >= 0; --symbol)
    {
        auto backBucketOffset = &backBucketOffset_[symbol << 8];
        auto endSuffix = currentSuffix - bCount_[symbol];
        
        while (currentSuffix > endSuffix)
        {
            // determine how many B/B* suffixes are safe to process during this pass
            auto maxEnd = currentSuffix - (max_cache_size * numThreads);
            if (maxEnd < suffixArrayBegin_)
                maxEnd = suffixArrayBegin_;
            if (maxEnd < endSuffix)
                maxEnd = endSuffix;
            auto temp = currentSuffix;
            while ((temp > maxEnd) && (*temp != suffix_is_unsorted_b_type))
                --temp;
            auto totalSuffixesPerThread = ((std::distance(temp, currentSuffix) + numThreads - 1) / numThreads);

            // process suffixes
            for (auto threadId = 0; threadId < numThreads; ++threadId)
            {
                numSuffixes[threadId] = 0;
                auto endForThisThread = currentSuffix - totalSuffixesPerThread;
                if (endForThisThread < temp)
                    endForThisThread = temp;
                post_task_to_thread
                (
                    threadId, 
                    [](
                        uint8_t const * inputBegin,
                        suffix_index * begin,
                        suffix_index * end,
                        entry_type * cache,
                        int32_t & numSuffixes,
                        int32_t * suffixCount
                    )
                    {
                        ++begin;
                        auto curCache = cache;
                        uint8_t currentPrecedingSymbol = 0;
                        int32_t currentPrecedingSymbolCount = 0;
                        while (--begin > end)
                        {
                            auto currentSuffixIndex = *begin;
                            if ((currentSuffixIndex & preceding_suffix_is_type_a_flag) == 0)
                            {
                                int32_t precedingSuffixIndex = ((currentSuffixIndex & sa_index_mask) - 1);
                                auto precedingSuffix = (inputBegin + precedingSuffixIndex);
                                auto precedingSymbol = precedingSuffix[0];
                                int32_t flag = ((precedingSuffixIndex > 0) && (precedingSuffix[-1] <= precedingSymbol)) ? 0 : preceding_suffix_is_type_a_flag;
                                *curCache++ = {precedingSymbol, precedingSuffixIndex | flag};
                                if (precedingSymbol != currentPrecedingSymbol)
                                {
                                    suffixCount[currentPrecedingSymbol] += currentPrecedingSymbolCount;
                                    currentPrecedingSymbol = precedingSymbol;
                                    currentPrecedingSymbolCount = 0;
                                }
                                ++currentPrecedingSymbolCount;
                                if (precedingSuffixIndex >= 0)
                                    *begin = precedingSymbol;
                            }
                        }
                        suffixCount[currentPrecedingSymbol] += currentPrecedingSymbolCount;
                        numSuffixes = std::distance(cache, curCache);
                    }, inputBegin_, currentSuffix, endForThisThread, cache[threadId].get(), std::ref(numSuffixes[threadId]), sCount[threadId]
                );
                currentSuffix = endForThisThread;
            }
            wait_for_all_tasks_completed();

            // 
            for (auto threadId = 0, begin = 0, numSymbolsPerThread = ((0x100 + numThreads - 1) / numThreads); threadId < numThreads; ++threadId)
            {
                auto end = begin + numSymbolsPerThread;
                if (end > 0x100)
                    end = 0x100;
                post_task_to_thread
                (
                    threadId, 
                    [&dest, &backBucketOffset, &sCount, numThreads]
                    (
                        int32_t begin,
                        int32_t end
                    )
                    {
                        for (auto threadId = 0; threadId < numThreads; ++threadId)
                            for (auto symbol = begin; symbol < end; ++symbol)
                            {
                                dest[threadId][symbol] = backBucketOffset[symbol];
                                backBucketOffset[symbol] -= sCount[threadId][symbol];
                                sCount[threadId][symbol] = 0;
                            }
                    }, begin, end
                );
                begin = end;
            }
            wait_for_all_tasks_completed();

            //
            for (auto threadId = 0; threadId < numThreads; ++threadId)
                post_task_to_thread
                (
                    threadId,
                    [&](
                        suffix_index * dest[0x100],
                        entry_type const * begin,
                        entry_type const * end
                    )
                    {
                        --begin;
                        while (++begin < end)
                            *(--dest[begin->precedingSuffix_]) = begin->precedingSuffixIndex_;
                    }, 
                    dest[threadId], cache[threadId].get(), cache[threadId].get() + numSuffixes[threadId]
                );
            wait_for_all_tasks_completed();
        }

        currentSuffix -= aCount_[symbol];
    }
}


//==============================================================================
int32_t maniscalco::msufsort::second_stage_its_as_burrows_wheeler_transform_left_to_right_pass_single_threaded
(
    // private:
    // induce sorted position of A suffixes from sorted B suffixes
    // This is the second half of the second stage of the ITS ... the 'left to right' pass
)
{
    auto sentinel = suffixArrayBegin_;
    auto currentSuffix = suffixArrayBegin_ - 1;
    uint8_t previousPrecedingSymbol = 0;
    auto previousFrontBucketOffset = frontBucketOffset_;
    while (++currentSuffix < suffixArrayEnd_)
    {
        auto currentSuffixIndex = *currentSuffix;
        if (currentSuffixIndex & preceding_suffix_is_type_a_flag)
        {
            int32_t precedingSuffixIndex = ((currentSuffixIndex & sa_index_mask) - 1);
            auto precedingSuffix = (inputBegin_ + precedingSuffixIndex);
            if ((currentSuffixIndex & sa_index_mask) != 0)
            {
                auto precedingSymbol = precedingSuffix[0];
                int32_t flag = ((precedingSuffixIndex > 0) && (precedingSuffix[-1] >= precedingSymbol)) ? preceding_suffix_is_type_a_flag : 0;
                if (precedingSymbol != previousPrecedingSymbol)
                {
                    previousPrecedingSymbol = precedingSymbol;
                    previousFrontBucketOffset = frontBucketOffset_ + previousPrecedingSymbol;
                }
                if (flag)
                    *((*previousFrontBucketOffset)++) = (precedingSuffixIndex | flag);
                else
                    *((*previousFrontBucketOffset)++) = ((precedingSuffixIndex > 0) ? precedingSuffix[-1] : preceding_suffix_is_type_a_flag);
            }
            if (precedingSuffixIndex >= 0)
                *currentSuffix = *precedingSuffix;
            else
                sentinel = currentSuffix;
        }
    }
    int32_t sentinelIndex = (int32_t)std::distance(suffixArrayBegin_, sentinel);
    return sentinelIndex;
}


//==============================================================================
int32_t maniscalco::msufsort::second_stage_its_as_burrows_wheeler_transform_left_to_right_pass_multi_threaded
(
    // private:
    // induce sorted position of A suffixes from sorted B suffixes
    // This is the second half of the second stage of the ITS ... the 'left to right' pass
)
{
    auto sentinel = suffixArrayBegin_;
    auto numThreads = (int32_t)(numWorkerThreads_ + 1); // +1 for main thread
    auto currentSuffix = suffixArrayBegin_;
    auto max_cache_size = (1 << 12);
    struct entry_type
    {
        uint8_t precedingSuffix_;
        int32_t precedingSuffixIndex_;
    };
    std::unique_ptr<entry_type []> cache[numThreads];
    for (auto i = 0; i < numThreads; ++i)
        cache[i].reset(new entry_type[max_cache_size]);
    int32_t numSuffixes[numThreads] = {};
    int32_t sCount[numThreads][0x100] = {};
    suffix_index * dest[numThreads][0x100] = {};

    while (currentSuffix < suffixArrayEnd_)
    {
        // calculate current 'safe' suffixes to process
        auto begin = currentSuffix;
        auto maxEnd = begin + (max_cache_size * numThreads);
        if (maxEnd > suffixArrayEnd_)
            maxEnd = suffixArrayEnd_;
        currentSuffix += (currentSuffix != maxEnd);
        while ((currentSuffix != maxEnd) && (*currentSuffix != (int32_t)0x80000000))
            ++currentSuffix;
        auto end = currentSuffix;
        auto totalSuffixes = std::distance(begin, end);
        auto totalSuffixesPerThread = ((totalSuffixes + numThreads - 1) / numThreads);

        // process suffixes
        for (auto threadId = 0; threadId < numThreads; ++threadId)
        {
            numSuffixes[threadId] = 0;
            auto endForThisThread = begin + totalSuffixesPerThread;
            if (endForThisThread > end)
                endForThisThread = end;
            post_task_to_thread
            (
                threadId, 
                [&sentinel](
                    uint8_t const * inputBegin,
                    suffix_index * begin,
                    suffix_index * end,
                    entry_type * cache,
                    int32_t & numSuffixes,
                    int32_t * suffixCount
                )
                {
                    auto current = begin;
                    auto curCache = cache;
                    --current;
                    uint8_t currentPrecedingSymbol = 0;
                    int32_t currentPrecedingSymbolCount = 0;
                    while (++current != end)
                    {
                        auto currentSuffixIndex = *current;
                        if (currentSuffixIndex & preceding_suffix_is_type_a_flag)
                        {
                            int32_t precedingSuffixIndex = ((currentSuffixIndex & sa_index_mask) - 1);
                            auto precedingSuffix = (inputBegin + precedingSuffixIndex);
                            if ((currentSuffixIndex & sa_index_mask) != 0)
                            {
                                auto precedingSymbol = precedingSuffix[0];
                                bool precedingSuffixIsTypeA = ((precedingSuffixIndex == 0) || (precedingSuffix[-1] >= precedingSymbol));
                                int32_t flag = (precedingSuffixIsTypeA) ? preceding_suffix_is_type_a_flag : 0;
                                if (flag)
                                    *curCache++ = {precedingSymbol, precedingSuffixIndex | flag};
                                else
                                    *curCache++ = {precedingSymbol, (precedingSuffixIndex > 0) ? precedingSuffix[-1] : 0};
                                if (precedingSymbol != currentPrecedingSymbol)
                                {
                                    suffixCount[currentPrecedingSymbol] += currentPrecedingSymbolCount;
                                    currentPrecedingSymbol = precedingSymbol;
                                    currentPrecedingSymbolCount = 0;
                                }
                                ++currentPrecedingSymbolCount;
                            }
                            if (precedingSuffixIndex >= 0)
                                *current = precedingSuffix[0];
                            else
                                sentinel = current;
                        }
                    }
                    suffixCount[currentPrecedingSymbol] += currentPrecedingSymbolCount;
                    numSuffixes = std::distance(cache, curCache);
                }, inputBegin_, begin, endForThisThread, cache[threadId].get(), std::ref(numSuffixes[threadId]), sCount[threadId]
            );
            begin = endForThisThread;
        }
        wait_for_all_tasks_completed();

        //
        for (auto threadId = 0, begin = 0, numSymbolsPerThread = ((0x100 + numThreads - 1) / numThreads); threadId < numThreads; ++threadId)
        {
            auto end = begin + numSymbolsPerThread;
            if (end > 0x100)
                end = 0x100;
            post_task_to_thread
            (
                threadId, 
                [&dest, this, &sCount, numThreads]
                (
                    int32_t begin,
                    int32_t end
                )
                {
                    for (auto threadId = 0; threadId < numThreads; ++threadId)
                        for (auto symbol = begin; symbol < end; ++symbol)
                        {
                            dest[threadId][symbol] = frontBucketOffset_[symbol];
                            frontBucketOffset_[symbol] += sCount[threadId][symbol];
                            sCount[threadId][symbol] = 0;
                        }
                }, begin, end
            );
            begin = end;
        }
        wait_for_all_tasks_completed();

        //
        for (auto threadId = 0; threadId < numThreads; ++threadId)
            post_task_to_thread
            (
                threadId,
                [](
                    suffix_index * dest[0x100],
                    entry_type const * begin,
                    entry_type const * end
                )
                {
                    --begin;
                    while (++begin != end)
                        *(dest[begin->precedingSuffix_]++) = begin->precedingSuffixIndex_;
                }, 
                dest[threadId], cache[threadId].get(), cache[threadId].get() + numSuffixes[threadId]
            );
        wait_for_all_tasks_completed();
    }
    int32_t sentinelIndex = (int32_t)std::distance(suffixArrayBegin_, sentinel);
    return sentinelIndex;
}


//==============================================================================
int32_t maniscalco::msufsort::second_stage_its_as_burrows_wheeler_transform
(
    // private:
    // creates the burrows wheeler transform while completing the second stage
    // of the improved two stage sort.
)
{
    if (numWorkerThreads_ == 0)
    {
        auto start = std::chrono::system_clock::now();
        second_stage_its_as_burrows_wheeler_transform_right_to_left_pass_single_threaded();
        auto finish = std::chrono::system_clock::now();
        #ifdef VERBOSE
            std::cout << "second stage right to left pass time: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() << " ms " << std::endl;
        #endif
        start = std::chrono::system_clock::now();
        auto sentinelIndex = second_stage_its_as_burrows_wheeler_transform_left_to_right_pass_single_threaded();
        finish = std::chrono::system_clock::now();
        #ifdef VERBOSE
            std::cout << "second stage left to right pass time: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() << " ms " << std::endl;
        #endif
        return sentinelIndex;
    }
    else
    {
        auto start = std::chrono::system_clock::now();
        second_stage_its_as_burrows_wheeler_transform_right_to_left_pass_multi_threaded();
        auto finish = std::chrono::system_clock::now();
        #ifdef VERBOSE
            std::cout << "second stage right to left pass time: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() << " ms " << std::endl;
        #endif
        start = std::chrono::system_clock::now();
        auto sentinelIndex = second_stage_its_as_burrows_wheeler_transform_left_to_right_pass_multi_threaded();
        finish = std::chrono::system_clock::now();
        #ifdef VERBOSE
            std::cout << "second stage left to right pass time: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() << " ms " << std::endl;
        #endif
        return sentinelIndex;
    }
}


//==============================================================================
void maniscalco::msufsort::count_suffixes
(
    uint8_t const * begin,
    uint8_t const * end,
    int32_t (&aCount)[0x10000],
    int32_t (&bCount)[0x10000],
    int32_t (&bStarCount)[0x10000]
)
{
    if (begin < end)
        return;
    auto current = begin;
    auto suffixType = get_suffix_type(current);
    if (suffixType == suffix_type::bStar)
        ++bStarCount[endian_swap<host_order_type, big_endian_type>(*(uint16_t const *)current--)];
    if ((suffixType == suffix_type::bStar) || (suffixType == suffix_type::b))
        while ((current >= end) && (current[0] <= current[1]))
            ++bCount[endian_swap<host_order_type, big_endian_type>(*(uint16_t const *)current--)];
    while (current >= end)
    {
        while ((current >= end) && (current[0] >= current[1]))
            ++aCount[endian_swap<host_order_type, big_endian_type>(*(uint16_t const *)current--)];
        if (current >= end)
            ++bStarCount[endian_swap<host_order_type, big_endian_type>(*(uint16_t const *)current--)];
        while ((current >= end) && (current[0] <= current[1]))
            ++bCount[endian_swap<host_order_type, big_endian_type>(*(uint16_t const *)current--)];
    }
}


//==============================================================================
void maniscalco::msufsort::initial_two_byte_radix_sort
(
    uint8_t const * begin,
    uint8_t const * end,
    int32_t (&bStarOffset)[0x10000]
)
{
    if (begin < end)
        return;
    auto current = begin;
    auto suffixType = get_suffix_type(current);
    if (suffixType == suffix_type::bStar)
    {
        int32_t flag = ((current > inputBegin_) && (current[-1] <= current[0])) ? 0 : preceding_suffix_is_type_a_flag;
        suffixArrayBegin_[bStarOffset[endian_swap<host_order_type, big_endian_type>(*(uint16_t const *)current)]++] = (std::distance(inputBegin_, current) | flag);
    }
    if ((suffixType == suffix_type::bStar) || (suffixType == suffix_type::b))
        while ((current >= end) && (current[0] <= current[1]))
            --current;
    while (current >= end)
    {
        while ((current >= end) && (current[0] >= current[1]))
            --current;
        if (current >= end)
        {
            int32_t flag = ((current > inputBegin_) && (current[-1] <= current[0])) ? 0 : preceding_suffix_is_type_a_flag;
            suffixArrayBegin_[bStarOffset[endian_swap<host_order_type, big_endian_type>(*(uint16_t const *)current)]++] = 
                        (std::distance(inputBegin_, current) | flag);
        }
        while ((current >= end) && (current[0] <= current[1]))
            --current;
    }
}


//==============================================================================
void maniscalco::msufsort::first_stage_its
(
    // private:
    // does the first stage of the improved two stage sort
)
{
    auto numThreads = (int32_t)(numWorkerThreads_ + 1); // +1 for main thread
    auto start = std::chrono::system_clock::now();
    std::unique_ptr<int32_t []> bCount(new int32_t[0x10000]{});
    std::unique_ptr<int32_t []> aCount(new int32_t[0x10000]{});
    std::unique_ptr<int32_t [][0x10000]> bStarCount(new int32_t[numThreads][0x10000]{});
    auto numSuffixesPerThread = ((inputSize_ + numThreads - 1) / numThreads);

    {
        std::unique_ptr<int32_t [][0x10000]> threadBCount(new int32_t[numThreads][0x10000]{});
        std::unique_ptr<int32_t [][0x10000]> threadACount(new int32_t[numThreads][0x10000]{});
        auto inputCurrent = inputBegin_;
        for (auto threadId = 0; threadId < numThreads; ++threadId)
        {
            auto inputEnd = inputCurrent + numSuffixesPerThread;
            if (inputEnd > (inputEnd_ - 1))
                inputEnd = (inputEnd_ - 1);
            post_task_to_thread(threadId, &msufsort::count_suffixes, this, inputEnd - 1, inputCurrent, 
                    std::ref(threadACount[threadId]), std::ref(threadBCount[threadId]), std::ref(bStarCount[threadId]));
            inputCurrent = inputEnd;
        }
        wait_for_all_tasks_completed();

        ++aCount[((uint16_t)inputEnd_[-1]) << 8];
        ++aCount_[inputEnd_[-1]];
        for (auto threadId = 0; threadId < numThreads; ++threadId)
            for (auto j = 0; j < 0x10000; ++j)
            {
                bCount[j] += threadBCount[threadId][j];
                bCount_[j >> 8] += threadBCount[threadId][j] + bStarCount[threadId][j];
                aCount[j] += threadACount[threadId][j];
                aCount_[j >> 8] += threadACount[threadId][j];
            }
    }

    // compute bucket offsets into suffix array
    int32_t total = 1;  // 1 for sentinel
    int32_t bStarTotal = 0;
    std::unique_ptr<int32_t []> totalBStarCount(new int32_t[0x10000]{});
    std::unique_ptr<int32_t [][0x10000]> bStarOffset(new int32_t[numThreads][0x10000]{});
    std::unique_ptr<std::pair<int32_t, int32_t> []> partitions(new std::pair<int32_t, int32_t>[0x10000]{});
    auto numPartitions = 0;
    for (int32_t i = 0; i < 0x100; ++i)
    {
        int32_t s = (i << 8);
        frontBucketOffset_[i] = (suffixArrayBegin_ + total);
        for (int32_t j = 0; j < 0x100; ++j, ++s)
        {
            auto partitionStartIndex = bStarTotal;
            for (int32_t threadId = 0; threadId < numThreads; ++threadId)
            {
                bStarOffset[threadId][s] = bStarTotal;
                totalBStarCount[s] += bStarCount[threadId][s];
                bStarTotal += bStarCount[threadId][s];
                bCount[s] += bStarCount[threadId][s];
            }
            total += (bCount[s] + aCount[s]);
            backBucketOffset_[(j << 8) | i] = suffixArrayBegin_ + total;
            if (totalBStarCount[s] > 0)
                partitions[numPartitions++] = std::make_pair(partitionStartIndex, totalBStarCount[s]);
        }
    }

    // multi threaded two byte radix sort forms initial partitions which
    // will be fully sorted by multikey quicksort
    auto inputCurrent = inputBegin_;
    for (auto threadId = 0; threadId < numThreads; ++threadId)
    {
        auto inputEnd = inputCurrent + numSuffixesPerThread;
        if (inputEnd > (inputEnd_ - 1))
            inputEnd = (inputEnd_ - 1);
        post_task_to_thread(threadId, &msufsort::initial_two_byte_radix_sort, this, inputEnd - 1, inputCurrent, std::ref(bStarOffset[threadId]));
        inputCurrent = inputEnd;
    }
    wait_for_all_tasks_completed();

    auto finish = std::chrono::system_clock::now();
    #ifdef VERBOSE
        std::cout << "direct sort initial 16 bit sort time: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() << " ms " << std::endl;
    #endif
    start = std::chrono::system_clock::now();
    
    // multikey quicksort on B* parititions
    std::atomic<int32_t> partitionCount(numPartitions);
    for (auto threadId = 0; threadId < numThreads; ++threadId)
    {
        post_task_to_thread
        (
            threadId, 
            [&]()
            {
                while (true)
                {
                    int32_t partitionIndex = --partitionCount;
                    if (partitionIndex < 0)
                        break;
                    auto const & partition = partitions[partitionIndex];
                    multikey_quicksort(suffixArrayBegin_ + partition.first, suffixArrayBegin_ + partition.first + partition.second, 2, 0);
                }
            }
        );
    }
    wait_for_all_tasks_completed();

    // spread b* to their final locations in suffix array
    auto destination = suffixArrayBegin_ + total;
    auto source = suffixArrayBegin_ + bStarTotal;
    for (int32_t i = 0xffff; i >= 0; --i)
    {
        if (bCount[i] || aCount[i])
        {
            destination -= bCount[i];
            source -= totalBStarCount[i];
            for (auto j = totalBStarCount[i] - 1; j >= 0; --j)
                destination[j] = source[j];
            for (auto j = totalBStarCount[i]; j < bCount[i]; ++j)
                destination[j] = suffix_is_unsorted_b_type;
            destination -= aCount[i];
            for (auto j = 0; j < aCount[i]; ++j)
                destination[j] = preceding_suffix_is_type_a_flag;
        }
    }
    suffixArrayBegin_[0] = (inputSize_ | preceding_suffix_is_type_a_flag); // sa[0] = sentinel

    finish = std::chrono::system_clock::now();
    #ifdef VERBOSE
        std::cout << "direct sort time: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() << " ms " << std::endl;
    #endif
}


//==============================================================================
auto maniscalco::msufsort::make_suffix_array
(
    // public:
    // computes the suffix array for the input data
    uint8_t const * inputBegin,
    uint8_t const * inputEnd
) -> suffix_array
{
    inputBegin_ = inputBegin;
    inputEnd_ = inputEnd;
    inputSize_ = std::distance(inputBegin_, inputEnd_);
    getValueEnd_ = (inputEnd_ - sizeof(uint64_t));
    getValueMaxIndex_ = (inputSize_ - sizeof(uint64_t));
    for (auto & e : copyEnd_)
        e = 0x00;
    auto source = inputEnd_ - sizeof(uint64_t);
    auto dest = copyEnd_;
    if (source < inputBegin_)
    {
        auto n = std::distance(source, inputBegin);
        source += n;
        dest += n;
    }
    std::copy(source, inputEnd_, dest);
    suffix_array suffixArray;
    auto suffixArraySize = (inputSize_ + 1);
    suffixArray.resize(suffixArraySize);
    for (auto & e : suffixArray)
        e = 0;
    suffixArrayBegin_ = suffixArray.data();
    suffixArrayEnd_ = suffixArrayBegin_ + suffixArraySize;
    inverseSuffixArrayBegin_ = (suffixArrayBegin_ + ((inputSize_ + 1) >> 1));
    inverseSuffixArrayEnd_ = suffixArrayEnd_;

    first_stage_its();
    second_stage_its();
    return suffixArray;
}


//==============================================================================
int32_t maniscalco::msufsort::forward_burrows_wheeler_transform
(
    // public:
    // computes the burrows wheeler transform for the input data and
    // replaces the input data with that transformed result.
    // returns the index of the sentinel character (which is removed from the
    // transformed data).
    uint8_t * inputBegin,
    uint8_t * inputEnd
)
{
    inputBegin_ = inputBegin;
    inputEnd_ = inputEnd;
    inputSize_ = std::distance(inputBegin_, inputEnd_);

    getValueEnd_ = (inputEnd_ - sizeof(uint64_t));
    getValueMaxIndex_ = (inputSize_ - sizeof(uint64_t));
    for (auto & e : copyEnd_)
        e = 0x00;
    auto source = inputEnd_ - sizeof(uint64_t);
    auto dest = copyEnd_;
    if (source < inputBegin_)
    {
        auto n = std::distance((uint8_t const *)source, (uint8_t const *)inputBegin);
        source += n;
        dest += n;
    }
    std::copy(source, inputEnd_, dest);
    suffix_array suffixArray;
    auto suffixArraySize = (inputSize_ + 1);
    suffixArray.resize(suffixArraySize);
    for (auto & e : suffixArray)
        e = 0;
    suffixArrayBegin_ = suffixArray.data();
    suffixArrayEnd_ = suffixArrayBegin_ + suffixArraySize;
    inverseSuffixArrayBegin_ = (suffixArrayBegin_ + ((inputSize_ + 1) >> 1));
    inverseSuffixArrayEnd_ = suffixArrayEnd_;

    first_stage_its();
    int32_t sentinelIndex = second_stage_its_as_burrows_wheeler_transform();
    for (int32_t i = 0; i < (inputSize_ + 1); ++i)
    {
        if (i != sentinelIndex)
            *inputBegin++ = (uint8_t)suffixArray[i];
    }
    return sentinelIndex;
}


#ifdef USE_EXPERIMENTAL_INVERSE_BWT
    //==============================================================================
    void maniscalco::msufsort::reverse_burrows_wheeler_transform
    (
        uint8_t * inputBegin,
        uint8_t * inputEnd,
        int32_t sentinelIndex,
        int32_t numThreads
    )
    {
        #pragma pack(push, 1)
        struct index_type
        {
            suffix_index value_;
            std::uint8_t symbol_;
        };
        #pragma pack(pop)

        auto inputSize = std::distance(inputBegin, inputEnd);
        std::vector<index_type> index;
        index.resize(inputSize + 1);
	    int32_t symbolRange[0x101];
	    for (auto & e : symbolRange)
		    e = 0;
	    symbolRange[0] = 1;
	    for (auto inputCurent = inputBegin; inputCurent < inputEnd; ++inputCurent)
		    symbolRange[((uint32_t)*inputCurent) + 1]++;
	    int32_t n = 0;
	    for (auto & e : symbolRange)
	    {
		    auto temp = e;
		    e = n;
		    n += temp;
	    }
	    n = 0;
	    index[0] = {sentinelIndex, inputBegin[0]};
	    for (int32_t i = 0; i < inputSize; i++, n++)
	    {
		    n += (i == sentinelIndex);
            auto k = symbolRange[(uint32_t)inputBegin[i] + 1]++;
		    index[k] = {n, inputBegin[k - (k >= sentinelIndex)]};
	    }

        std::size_t maxPartitionsPerThread = 256;
        std::vector<ibwt_partition_info> ibwtPartitionInfo;
        std::size_t partitionCount = (numThreads * maxPartitionsPerThread);
        if (partitionCount > index.size())
            partitionCount = index.size();
        ibwtPartitionInfo.reserve(partitionCount + 8192);
        std::size_t maxBytesPerPartition = (((index.size() << 1) - 1) / partitionCount);

        auto firstDecodeIndex = index[0].value_;
        auto outputCurrent = inputBegin;
        auto currentIndex = 0;
        while (currentIndex < (std::int32_t)index.size())
        {
            auto partitionSize = maxBytesPerPartition;
            if ((currentIndex + partitionSize) > index.size())
                partitionSize = (index.size() - currentIndex);
            ibwtPartitionInfo.push_back({index[currentIndex].value_, index[currentIndex].value_, outputCurrent, outputCurrent,
                    ((outputCurrent + partitionSize) <= inputEnd) ? (outputCurrent + partitionSize) : inputEnd});
            index[currentIndex].value_ |= 0x80000000;
            currentIndex += partitionSize;
            outputCurrent += partitionSize;
        }
        partitionCount = ibwtPartitionInfo.size();

        std::vector<std::thread> threads;
        threads.resize(numThreads);
        struct decoded_info
        {
            std::uint8_t const * begin_;
            std::uint8_t const * end_;
            suffix_index startIndex_;
            suffix_index endIndex_;
        };

        std::vector<decoded_info> decodedInfo;
        decodedInfo.reserve(8192);

        std::vector<std::pair<std::uint8_t *, std::uint8_t *>> availableDecodeSpace;
        availableDecodeSpace.reserve(8192);
        while (!ibwtPartitionInfo.empty())
        {
            auto partitionsRemaining = ibwtPartitionInfo.size();
            maxPartitionsPerThread = (((partitionsRemaining << 1) - 1) / numThreads);
            if (maxPartitionsPerThread < 1)
                maxPartitionsPerThread = 1;
            for (auto threadId = 0; threadId < numThreads; ++threadId)
            {
                auto numPartitions = maxPartitionsPerThread;
                if (numPartitions > partitionsRemaining)
                    numPartitions = partitionsRemaining;
                partitionsRemaining -= numPartitions;
                threads[threadId] = std::move(std::thread(
                        [](
                            index_type * indexBegin,
                            suffix_index sentinelIndex,
                            ibwt_partition_info * partitionBegin, 
                            ibwt_partition_info * partitionEnd
                        )
                        {
                            bool done = false;
                            while (!done)
                            {
                                done = true;
                                for (auto partitionCurrent = partitionBegin; partitionCurrent < partitionEnd; ++partitionCurrent)
                                {
                                    auto & e = *partitionCurrent;
                                    if (((e.currentIndex_ & (suffix_index)0x80000000) == 0) && (e.currentOutput_ < e.endOutput_))
                                    {
                                        done = false;
                                        auto i = e.currentIndex_;
                                        *e.currentOutput_ = indexBegin[i].symbol_;
                                        e.currentOutput_ += (i != sentinelIndex);
                                        e.currentIndex_ = indexBegin[i].value_;
                                    }
                                }
                            }
                        },
                        index.data(), sentinelIndex, ibwtPartitionInfo.data() + partitionsRemaining, 
                        ibwtPartitionInfo.data() + partitionsRemaining + numPartitions));
            }
            for (auto & e : threads)
                e.join();

            for (auto iter = ibwtPartitionInfo.begin(); iter != ibwtPartitionInfo.end(); )
            {
                if (iter->currentOutput_ != nullptr)
                {
                    auto startIndex = iter->startIndex_;
                    auto endIndex = (iter->currentIndex_ & 0x7fffffff);
                    if ((iter->currentIndex_ & 0x80000000) || (iter->beginOutput_ != iter->currentOutput_))
                    {
                        decodedInfo.push_back({iter->beginOutput_, iter->currentOutput_, startIndex, endIndex});
                        iter->startIndex_ = endIndex;
                    }
                }
                if (iter->currentIndex_ & 0x80000000)
                {
                    if (iter->currentOutput_ < iter->endOutput_)
                        availableDecodeSpace.push_back(std::make_pair(iter->currentOutput_, iter->endOutput_));
                    iter = ibwtPartitionInfo.erase(iter);
                }
                else
                {
                    ++iter;
                }
            }

            if (!ibwtPartitionInfo.empty())
            {
                for (auto & e : ibwtPartitionInfo)
                {
                    if (!availableDecodeSpace.empty())
                    {
                        auto a = availableDecodeSpace.back();
                        availableDecodeSpace.pop_back();
                        e.beginOutput_ = a.first;
                        e.currentOutput_ = a.first;
                        e.endOutput_ = a.second;
                    }
                    else
                    {
                        e.currentOutput_ = nullptr;
                        e.endOutput_ = nullptr;
                    }
                }
            }
        }

        auto currentDecodedIndex = 0;
        std::uint8_t const * curDec = nullptr;
        std::uint8_t const * curDecEnd = nullptr;
        suffix_index curEndIndex = 0;
        for (std::size_t i = 0; i < decodedInfo.size(); ++i)
            if (decodedInfo[i].startIndex_ == firstDecodeIndex)
            {
                curDec = decodedInfo[currentDecodedIndex].begin_;
                curDecEnd = decodedInfo[currentDecodedIndex].end_;
                curEndIndex = decodedInfo[currentDecodedIndex].endIndex_;
            }

        auto beginWrite = (std::uint8_t *)index.data();
        auto currentWrite = beginWrite;
        auto endWrite = (currentWrite + inputSize);
        while (currentWrite < endWrite)
        {
            while ((currentWrite < endWrite) && (curDec < curDecEnd))
                *currentWrite++ = *curDec++;
            for (std::size_t j = 0; j < decodedInfo.size(); ++j)
                if (decodedInfo[j].startIndex_ == curEndIndex)
                {
                    curDec = decodedInfo[j].begin_;
                    curDecEnd = decodedInfo[j].end_;
                    curEndIndex = decodedInfo[j].endIndex_;
                    break;
                }
        }
        std::copy(beginWrite, endWrite, inputBegin);
    }


#else // !USE_EXPERIMENTAL_INVERSE_BWT


    //==============================================================================
    void maniscalco::msufsort::reverse_burrows_wheeler_transform
    (
        // public:
        // reverse the input (which is a BWT with sentinel symbol removed at index provided).
        // overwrites the input data with the reverse transformed data.
        uint8_t * inputBegin,
        uint8_t * inputEnd,
        int32_t sentinelIndex
    )
    {
        auto inputSize = std::distance(inputBegin, inputEnd);
        std::vector<suffix_index> index;
        index.resize(inputSize + 1);
	    int32_t symbolRange[0x102];
	    for (auto & e : symbolRange)
		    e = 0;

	    symbolRange[0] = 1;
	    for (auto inputCurent = inputBegin; inputCurent < inputEnd; ++inputCurent)
		    symbolRange[((uint32_t)*inputCurent) + 1]++;

	    int32_t n = 0;
	    for (auto & e : symbolRange)
	    {
		    auto temp = e;
		    e = n;
		    n += temp;
	    }

	    n = 0;
	    index[0] = sentinelIndex;
	    for (int32_t i = 0; i < inputSize; i++, n++)
	    {
		    n += (i == sentinelIndex);
		    index[symbolRange[(uint32_t)inputBegin[i] + 1]++] = n;
	    }
	    n = sentinelIndex;
	    std::vector<uint8_t> reversedBuffer;
        reversedBuffer.resize(inputSize);
	    for (auto & e : reversedBuffer)
	    {
		    n = index[n];
		    e = inputBegin[n - (n >= sentinelIndex)];
	    }
        for (auto e : reversedBuffer)
            *(inputBegin++) = e;
    }
#endif // !USE_EXPERIMENTAL_INVERSE_BWT

