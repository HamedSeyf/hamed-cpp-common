#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>
#include <queue>
#include <vector>
#include <array>
#include <set>
#include <optional>
#include <random>
#include <string>
#include <cstdio>
#include <ranges>
#include <iterator>
#include <functional>

#include <spdlog/spdlog.h>


// Sorts any mutable random-access range in place (std::vector, std::span, std::array,
// a custom container's own iterators, ...) rather than being tied to std::vector.
template<std::ranges::random_access_range R, typename TComparator = std::ranges::less>
    requires
        std::ranges::sized_range<R> &&
        std::sortable<std::ranges::iterator_t<R>, TComparator>
void bubbleSort(R&& input, TComparator comparator = {})
{
	const std::size_t size = std::ranges::size(input);

	if (size < 2)
	{
		return;
	}

	const auto begin = std::ranges::begin(input);

    // Safe to use size - 1 due to size check earlier
    for (std::size_t outerElementIndexToFix = size - 1; outerElementIndexToFix > 0; --outerElementIndexToFix)
	{
		bool hasSwappedAnyElement = false;

		for (std::size_t innerElementIndex = 0; innerElementIndex < outerElementIndexToFix; ++innerElementIndex)
		{
			if (comparator(begin[innerElementIndex + 1], begin[innerElementIndex]))
			{
				std::ranges::iter_swap(begin + innerElementIndex, begin + innerElementIndex + 1);
				hasSwappedAnyElement = true;
			}
		}

		if (!hasSwappedAnyElement)
		{
			break;
		}
	}
}


template<typename T, typename TComparator>
[[nodiscard]] std::vector<T> mergeSortedVectors(const std::vector<T>& input1, const std::vector<T>& input2, const TComparator& comparator)
{
	std::vector<T> retVal;
	retVal.reserve(input1.size() + input2.size());

	auto iter1 = input1.begin();
	auto iter2 = input2.begin();

	while (iter1 != input1.end() || iter2 != input2.end())
	{
		if (iter1 == input1.end())
		{
			retVal.push_back(*iter2++);
		}
		else if (iter2 == input2.end())
		{
			retVal.push_back(*iter1++);
		}
		else if (comparator(*iter2, *iter1))
		{
			retVal.push_back(*iter2++);
		}
		else
		{
			retVal.push_back(*iter1++);
		}
	}

	return retVal;
}


// Reads from any random-access range (std::vector, std::span, std::array, a custom
// container's own iterators, ...); it never mutates input, only copies out of it,
// so unlike bubbleSort it doesn't need input to be sortable/mutable.
template<std::ranges::random_access_range R, typename TComparator = std::ranges::less>
    requires
        std::ranges::sized_range<R> &&
        std::indirect_strict_weak_order<TComparator, std::ranges::iterator_t<R>>
[[nodiscard]] std::vector<std::ranges::range_value_t<R>> mergeSort(R&& input, std::size_t startIndex, std::size_t endIndex, TComparator comparator = {})
{
	using T = std::ranges::range_value_t<R>;

	assert(startIndex <= endIndex);
	assert(std::ranges::empty(input) || endIndex < std::ranges::size(input));

	if (std::ranges::empty(input))
	{
		return {};
	}

	const auto begin = std::ranges::begin(input);

	if (startIndex == endIndex)
	{
		return std::vector<T>(begin + startIndex, begin + endIndex + 1);
	}

	// Avoiding (startIndex + endIndex) / 2 to avoid potential overflow of sum operation
	const std::size_t midIndex = startIndex + ((endIndex - startIndex) / 2);

	const std::vector<T> leftSortedVector = mergeSort(input, startIndex, midIndex, comparator);
	const std::vector<T> rightSortedVector = mergeSort(input, midIndex + 1, endIndex, comparator);

	return mergeSortedVectors(leftSortedVector, rightSortedVector, comparator);
}


// Simple overload which sorts the whole range
template<std::ranges::random_access_range R, typename TComparator = std::ranges::less>
    requires
        std::ranges::sized_range<R> &&
        std::indirect_strict_weak_order<TComparator, std::ranges::iterator_t<R>>
[[nodiscard]] std::vector<std::ranges::range_value_t<R>> mergeSort(R&& input, TComparator comparator = {})
{
	const std::size_t size = std::ranges::size(input);

	return size < 2
		? std::vector<std::ranges::range_value_t<R>>(std::ranges::begin(input), std::ranges::end(input))
		: mergeSort(input, std::size_t{ 0 }, size - 1, comparator);
}
