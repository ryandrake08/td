#pragma once
#include <algorithm>
#include <memory>
#include <mutex>
#include <unordered_map>

// shared instance
template<typename T>
std::shared_ptr<T> get_shared_instance()
{
    static std::mutex m;
    static std::weak_ptr<T> cache;

    std::lock_guard<std::mutex> lock(m);

    auto shared(cache.lock());
    if(cache.expired())
    {
        shared.reset(new T);
        cache = shared;
    }
    return shared;
}

// shared value given key
template<typename K, typename V>
std::shared_ptr<V> get_shared_instance(const K& key)
{
    static std::mutex m;
    static std::unordered_map<K,std::weak_ptr<V>> cache_map;

    std::lock_guard<std::mutex> lock(m);

    auto cache(cache_map[key]);
    auto shared(cache.lock());
    if(cache.expired())
    {
        shared.reset(new V(key));
        cache_map[key] = shared;
    }
    return shared;
}

// shared value given two parameters that form a key
template<typename K0, typename K1, typename V>
std::shared_ptr<V> get_shared_instance(const K0& key0, const K1& key1)
{
    // hash function (for unordered_map)
    struct pair_hash
    {
        std::size_t operator()(const std::pair<K0, K1> &x) const
        {
            return std::hash<K0>()(x.first) ^ std::hash<K1>()(x.second);
        }
    };

    static std::mutex m;
    static std::unordered_map<std::pair<K0,K1>,std::weak_ptr<V>, pair_hash> cache_map;

    std::lock_guard<std::mutex> lock(m);

    auto key(std::make_pair(key0, key1));
    auto cache(cache_map[key]);
    auto shared(cache.lock());
    if(cache.expired())
    {
        shared.reset(new V(key0, key1));
        cache_map[key] = shared;
    }
    return shared;
}
