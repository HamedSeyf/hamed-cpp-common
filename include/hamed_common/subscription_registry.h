#pragma once

import <cassert>;
import <concepts>;
import <cstddef>;
import <cstdint>;
import <functional>;
import <mutex>;
import <shared_mutex>;
import <stdexcept>;
import <string>;
import <type_traits>;
import <unordered_map>;
import <utility>;
import <vector>;


template<typename Type>
concept Hashable =
	requires(const Type& value)
	{
		{
			std::hash<Type>{}(value)
		} -> std::convertible_to<std::size_t>;
	};

template<typename Type>
concept IncrementableHandle =
	requires(Type value)
{
	{
		++value
	} -> std::same_as<Type&>;
};

// Optimized for read-heavy workloads: subscribe and unsubscribe take an
// exclusive lock, while concurrent readers take shared locks only long enough
// to create detached snapshots. A snapshot can therefore remain in use after
// one of its objects is unsubscribed from the registry.
// T being the subscriber type; K being subscriptions' key type; H being subscribers' handle type
template<typename T, typename K = std::string, typename H = std::uint64_t>
    requires
        std::is_copy_constructible_v<T> &&
        std::is_nothrow_swappable_v<T> &&
        std::is_copy_constructible_v<K> &&
        Hashable<K> &&
		std::equality_comparable<K> &&
        std::default_initializable<H> &&
		std::is_copy_constructible_v<H> &&
		std::is_nothrow_swappable_v<H> &&
		Hashable<H> &&
		std::equality_comparable<H> &&
		IncrementableHandle<H>
class TSubscriptionRegistry
{
public:

	H subscribe(T object, const K& key)
    {
        std::unique_lock lock{ _subscriptionsMutex };

        ++_latestHandle;

		if (handleToLocationMap.contains(_latestHandle))
        {
            assert(false && "Generated a duplicate subscription handle.");
            throw std::overflow_error{ "Subscription handle space exhausted." };
        }

        const auto [bucketIter, bucketInserted] =
            keyToBucketMap.try_emplace(key);

        auto& bucket = bucketIter->second;
        const std::size_t objectIndex = bucket.objects.size();

        try
        {
            bucket.objects.push_back(std::move(object));

            try
            {
                bucket.handles.push_back(_latestHandle);

                try
                {
                    const bool locationInserted =
                        handleToLocationMap.try_emplace(
                            _latestHandle,
                            SubscriptionLocation{ key, objectIndex }).second;

                    if (!locationInserted)
                    {
                        assert(false && "Generated a duplicate subscription handle.");
                        throw std::overflow_error{ "Subscription handle space exhausted." };
                    }
                }
                catch (...)
                {
                    bucket.handles.pop_back();
                    throw;
                }
            }
            catch (...)
            {
                bucket.objects.pop_back();
                throw;
            }
        }
        catch (...)
        {
            if (bucketInserted)
            {
                keyToBucketMap.erase(bucketIter);
            }

            throw;
        }

        return _latestHandle;
	}

	bool unsubscribe(const H& handle)
    {
        std::unique_lock lock{ _subscriptionsMutex };

		if (const auto foundLocation = handleToLocationMap.find(handle); foundLocation != handleToLocationMap.end())
        {
            const auto foundBucket = keyToBucketMap.find(foundLocation->second.key);

            if (foundBucket == keyToBucketMap.end())
            {
                assert(false && "Subscription location refers to a missing bucket.");
                throw std::logic_error{ "Subscription indexes are inconsistent." };
            }

            auto& bucket = foundBucket->second;
            const std::size_t objectIndex = foundLocation->second.index;

            if (bucket.objects.size() != bucket.handles.size() ||
                objectIndex >= bucket.objects.size() ||
                bucket.handles[objectIndex] != handle)
            {
                assert(false && "Subscription location refers to an invalid bucket index.");
                throw std::logic_error{ "Subscription indexes are inconsistent." };
            }

            // Safe to use bucket.objects.size() - 1 due to size check earlier
            const std::size_t lastIndex = bucket.objects.size() - 1;

            if (objectIndex != lastIndex)
            {
                const auto movedLocation =
                    handleToLocationMap.find(bucket.handles[lastIndex]);

                if (movedLocation == handleToLocationMap.end())
                {
                    assert(false && "Subscription bucket contains an unindexed handle.");
                    throw std::logic_error{ "Subscription indexes are inconsistent." };
                }

                using std::swap;
                swap(bucket.objects[objectIndex], bucket.objects[lastIndex]);
                swap(bucket.handles[objectIndex], bucket.handles[lastIndex]);
                movedLocation->second.index = objectIndex;
            }

            bucket.objects.pop_back();
            bucket.handles.pop_back();
			handleToLocationMap.erase(foundLocation);

            if (bucket.objects.empty())
            {
                keyToBucketMap.erase(foundBucket);
            }

			return true;
		}

		return false;
	}

    [[nodiscard]] std::vector<T> getSubscribedObjects(const K& key) const
    {
        std::shared_lock lock{ _subscriptionsMutex };
        if (auto foundIter = keyToBucketMap.find(key); foundIter != keyToBucketMap.end())
        {
            return foundIter->second.objects;
        }
        return {};
    }

private:

    struct SubscriptionBucket
    {
        std::vector<T> objects;
        std::vector<H> handles;
    };

    struct SubscriptionLocation
    {
        K key;
        std::size_t index;
    };

    H _latestHandle{ };
    // Using a shared_mutex since subscriptions snapshots happen concurrently and subscription changes are relatively rare
    mutable std::shared_mutex _subscriptionsMutex;
    std::unordered_map<K, SubscriptionBucket> keyToBucketMap;
	std::unordered_map<H, SubscriptionLocation> handleToLocationMap;

};
