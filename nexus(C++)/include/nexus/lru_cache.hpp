#pragma once

#include <chrono>
#include <list>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <unordered_map>

namespace nexus {

using Clock = std::chrono::steady_clock;

// thread-safe LRU cache with optional per-entry TTL.
// get, put, evict are all O(1).
// TTL of zero disables expiry.
template<typename K, typename V>
class LruCache {
public:
    using Duration = std::chrono::duration<double>;

    explicit LruCache(std::size_t capacity, Duration ttl = Duration::zero())
        : capacity_(capacity), ttl_(ttl)
    {
        if (capacity == 0) throw std::invalid_argument("LruCache capacity must be > 0");
        index_.reserve(capacity);
    }

    LruCache(const LruCache&)            = delete;
    LruCache& operator=(const LruCache&) = delete;

    [[nodiscard]] std::optional<V> get(const K& key) {
        std::unique_lock lock(mu_);

        auto it = index_.find(key);
        if (it == index_.end()) {
            ++stats_.misses;
            return std::nullopt;
        }

        if (is_expired(it->second->expires_at)) {
            evict(it);
            ++stats_.misses;
            ++stats_.expirations;
            return std::nullopt;
        }

        // move to front (most recently used)
        lru_.splice(lru_.begin(), lru_, it->second);
        ++stats_.hits;
        return it->second->value;
    }

    void put(K key, V value) {
        std::unique_lock lock(mu_);

        auto it = index_.find(key);
        if (it != index_.end()) {
            // update in place and bump to front
            it->second->value      = std::move(value);
            it->second->expires_at = next_expiry();
            lru_.splice(lru_.begin(), lru_, it->second);
            return;
        }

        if (lru_.size() >= capacity_)
            evict_lru();

        lru_.emplace_front(Entry{key, std::move(value), next_expiry()});
        index_[key] = lru_.begin();
        ++stats_.puts;
    }

    bool erase(const K& key) {
        std::unique_lock lock(mu_);
        auto it = index_.find(key);
        if (it == index_.end()) return false;
        evict(it);
        return true;
    }

    void clear() {
        std::unique_lock lock(mu_);
        lru_.clear();
        index_.clear();
    }

    std::size_t size()     const noexcept { std::unique_lock l(mu_); return lru_.size(); }
    std::size_t capacity() const noexcept { return capacity_; }
    bool        empty()    const noexcept { return size() == 0; }

    struct Stats {
        uint64_t hits{0};
        uint64_t misses{0};
        uint64_t puts{0};
        uint64_t evictions{0};
        uint64_t expirations{0};

        double hit_rate() const noexcept {
            uint64_t total = hits + misses;
            return total > 0 ? static_cast<double>(hits) / total : 0.0;
        }
    };

    Stats stats() const {
        std::unique_lock lock(mu_);
        return stats_;
    }

private:
    struct Entry {
        K                 key;
        V                 value;
        Clock::time_point expires_at;
    };

    using List     = std::list<Entry>;
    using Iterator = typename List::iterator;
    using Index    = std::unordered_map<K, Iterator>;

    const std::size_t capacity_;
    const Duration    ttl_;

    mutable std::mutex mu_;
    List               lru_;
    Index              index_;
    Stats              stats_;

    Clock::time_point next_expiry() const noexcept {
        if (ttl_ == Duration::zero()) return Clock::time_point::max();
        return Clock::now() + std::chrono::duration_cast<Clock::duration>(ttl_);
    }

    bool is_expired(Clock::time_point tp) const noexcept {
        return tp != Clock::time_point::max() && Clock::now() > tp;
    }

    void evict(typename Index::iterator it) {
        lru_.erase(it->second);
        index_.erase(it);
        ++stats_.evictions;
    }

    void evict_lru() {
        if (lru_.empty()) return;
        auto tail = std::prev(lru_.end());
        index_.erase(tail->key);
        lru_.erase(tail);
        ++stats_.evictions;
    }
};

} // namespace nexus
