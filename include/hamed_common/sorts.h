#pragma once

import <atomic>;
import <chrono>;
import <memory>;
import <thread>;
import <condition_variable>;
import <mutex>;
import <shared_mutex>;
import <queue>;
import <vector>;
import <array>;
import <set>;
import <optional>;
import <random>;
import <string>;
import <cstdio>;
import <ranges>;
import <iterator>;
import <functional>;

#include <spdlog/spdlog.h>


// Sorts any mutable random-access range in place (std::vector, std::span, std::array,
// a custom container's own iterators, ...) rather than being tied to std::vector.
template<std::ranges::random_access_range R, typename TComparator = std::ranges::less>
    requires
        std::ranges::sized_range<R> &&
        std::sortable<std::ranges::iterator_t<R>, TComparator>
void BubbleSort(R&& Input, TComparator Comparator = {})
{
	const std::size_t Size = std::ranges::size(Input);

	if (Size < 2)
	{
		return;
	}

	const auto Begin = std::ranges::begin(Input);

    // Safe to use Size - 1 due to size check earlier
    for (std::size_t OuterElementIndexToFix = Size - 1; OuterElementIndexToFix > 0; --OuterElementIndexToFix)
	{
		bool bHasSwappedAnyElement = false;

		for (std::size_t InnerElementIndex = 0; InnerElementIndex < OuterElementIndexToFix; ++InnerElementIndex)
		{
			if (Comparator(Begin[InnerElementIndex + 1], Begin[InnerElementIndex]))
			{
				std::ranges::iter_swap(Begin + InnerElementIndex, Begin + InnerElementIndex + 1);
				bHasSwappedAnyElement = true;
			}
		}

		if (!bHasSwappedAnyElement)
		{
			break;
		}
	}
}


template<typename T, typename TComparator>
[[nodiscard]] std::vector<T> MergeSortedVectors(const std::vector<T>& Input1, const std::vector<T>& Input2, const TComparator& Comparator)
{
	std::vector<T> RetVal;
	RetVal.reserve(Input1.size() + Input2.size());

	auto Iter1 = Input1.begin();
	auto Iter2 = Input2.begin();

	while (Iter1 != Input1.end() || Iter2 != Input2.end())
	{
		if (Iter1 == Input1.end())
		{
			RetVal.push_back(*Iter2++);
		}
		else if (Iter2 == Input2.end())
		{
			RetVal.push_back(*Iter1++);
		}
		else if (Comparator(*Iter2, *Iter1))
		{
			RetVal.push_back(*Iter2++);
		}
		else
		{
			RetVal.push_back(*Iter1++);
		}
	}

	return RetVal;
}


// Reads from any random-access range (std::vector, std::span, std::array, a custom
// container's own iterators, ...); it never mutates Input, only copies out of it,
// so unlike BubbleSort it doesn't need Input to be sortable/mutable.
template<std::ranges::random_access_range R, typename TComparator = std::ranges::less>
    requires
        std::ranges::sized_range<R> &&
        std::indirect_strict_weak_order<TComparator, std::ranges::iterator_t<R>>
[[nodiscard]] std::vector<std::ranges::range_value_t<R>> MergeSort(R&& Input, std::size_t StartIndex, std::size_t EndIndex, TComparator Comparator = {})
{
	using T = std::ranges::range_value_t<R>;

	assert(StartIndex <= EndIndex);
	assert(std::ranges::empty(Input) || EndIndex < std::ranges::size(Input));

	if (std::ranges::empty(Input))
	{
		return {};
	}

	const auto Begin = std::ranges::begin(Input);

	if (StartIndex == EndIndex)
	{
		return std::vector<T>(Begin + StartIndex, Begin + EndIndex + 1);
	}

	// Avoiding (StartIndex + EndIndex) / 2 to avoid potential overflow of sum operation
	const std::size_t MidIndex = StartIndex + ((EndIndex - StartIndex) / 2);

	const std::vector<T> LeftSortedVector = MergeSort(Input, StartIndex, MidIndex, Comparator);
	const std::vector<T> RightSortedVector = MergeSort(Input, MidIndex + 1, EndIndex, Comparator);

	return MergeSortedVectors(LeftSortedVector, RightSortedVector, Comparator);
}


// Simple overload which sorts the whole range
template<std::ranges::random_access_range R, typename TComparator = std::ranges::less>
    requires
        std::ranges::sized_range<R> &&
        std::indirect_strict_weak_order<TComparator, std::ranges::iterator_t<R>>
[[nodiscard]] std::vector<std::ranges::range_value_t<R>> MergeSort(R&& Input, TComparator Comparator = {})
{
	const std::size_t Size = std::ranges::size(Input);

	return Size < 2
		? std::vector<std::ranges::range_value_t<R>>(std::ranges::begin(Input), std::ranges::end(Input))
		: MergeSort(Input, std::size_t{ 0 }, Size - 1, Comparator);
}
