#pragma once

#include "detail/hash_util.hpp"

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace nexus {

template<
    typename K,
    typename V,
    typename Hash  = detail::Hasher<K>,
    typename Equal = std::equal_to<K>
>
class ConcurrentMap {
public:
    explicit ConcurrentMap(std::size_t initial_capacity = 64)
        : table_(detail::ceil_pow2(std::max(initial_capacity, std::size_t{8})))
        , mask_(table_.size() - 1)
        , size_(0)
    {}

    ConcurrentMap(const ConcurrentMap&)            = delete;
    ConcurrentMap& operator=(const ConcurrentMap&) = delete;
    ConcurrentMap(ConcurrentMap&&)                 = default;

    [[nodiscard]] std::optional<V> get(const K& key) const {
        const std::size_t h = hash_(key);

        for (std::size_t probe = 0; probe <= mask_; ++probe) {
            const std::size_t idx  = (h + probe) & mask_;
            const Slot&       slot = table_[idx];

            uint32_t ver;
            do { ver = slot.version.load(std::memory_order_acquire); }
            while (ver & 1u);

            if (!slot.occupied) {
                if (slot.version.load(std::memory_order_acquire) == ver) return std::nullopt;
                probe = 0; continue;
            }

            if (slot.displacement < static_cast<int32_t>(probe)) {
                if (slot.version.load(std::memory_order_acquire) == ver) return std::nullopt;
                probe = 0; continue;
            }

            if (equal_(slot.key, key)) {
                V value = slot.value;
                if (slot.version.load(std::memory_order_acquire) == ver) return value;
                probe = 0; continue;
            }
        }
        return std::nullopt;
    }

    void put(K key, V value) {
        std::unique_lock lock(resize_mu_);
        if (load_factor_exceeded()) rehash(table_.size() * 2);
        insert_into(table_, mask_, std::move(key), std::move(value));
        size_.fetch_add(1, std::memory_order_relaxed);
    }

    bool remove(const K& key) {
        std::unique_lock lock(resize_mu_);
        const std::size_t h = hash_(key);

        for (std::size_t probe = 0; probe <= mask_; ++probe) {
            std::size_t idx = (h + probe) & mask_;
            std::unique_lock slock(table_[idx].mu);

            if (!table_[idx].occupied) return false;
            if (table_[idx].displacement < static_cast<int32_t>(probe)) return false;
            if (!equal_(table_[idx].key, key)) continue;

            table_[idx].version.fetch_add(1, std::memory_order_release);
            table_[idx].occupied = false;

            std::size_t cur = idx;
            for (;;) {
                std::size_t ni   = (cur + 1) & mask_;
                Slot&       next = table_[ni];
                std::unique_lock nlock(next.mu);

                if (!next.occupied || next.displacement == 0) break;

                next.version.fetch_add(1, std::memory_order_release);
                table_[cur].key          = std::move(next.key);
                table_[cur].value        = std::move(next.value);
                table_[cur].displacement = next.displacement - 1;
                table_[cur].occupied     = true;
                table_[cur].version.fetch_add(1, std::memory_order_release);

                next.occupied = false;
                next.version.fetch_add(1, std::memory_order_release);
                cur = ni;
            }

            table_[idx].version.fetch_add(1, std::memory_order_release);
            size_.fetch_sub(1, std::memory_order_relaxed);
            return true;
        }
        return false;
    }

    std::size_t size()        const noexcept { return size_.load(std::memory_order_relaxed); }
    bool        empty()       const noexcept { return size() == 0; }
    std::size_t capacity()    const noexcept { return table_.size(); }
    double      load_factor() const noexcept {
        return static_cast<double>(size()) / static_cast<double>(capacity());
    }

    template<typename F>
    void for_each(F&& f) const {
        for (const auto& slot : table_)
            if (slot.occupied) f(slot.key, slot.value);
    }

private:
    struct Slot {
        std::atomic<uint32_t> version{0};
        mutable std::mutex    mu;
        bool                  occupied{false};
        int32_t               displacement{0};
        K                     key{};
        V                     value{};
    };

    Hash  hash_{};
    Equal equal_{};

    std::vector<Slot>        table_;
    std::size_t              mask_;
    std::atomic<std::size_t> size_;
    mutable std::mutex       resize_mu_;

    bool load_factor_exceeded() const noexcept {
        return (size_.load(std::memory_order_relaxed) + 1) * 4 > table_.size() * 3;
    }

    static void insert_into(std::vector<Slot>& table, std::size_t mask, K key, V value) {
        const std::size_t h = detail::Hasher<K>{}(key);
        int32_t disp = 0;

        for (std::size_t probe = 0; probe <= mask; ++probe, ++disp) {
            std::size_t idx  = (h + probe) & mask;
            Slot&       slot = table[idx];
            std::unique_lock slock(slot.mu);

            if (!slot.occupied) {
                slot.version.fetch_add(1, std::memory_order_release);
                slot.key          = std::move(key);
                slot.value        = std::move(value);
                slot.displacement = disp;
                slot.occupied     = true;
                slot.version.fetch_add(1, std::memory_order_release);
                return;
            }

            if (slot.displacement < disp) {
                slot.version.fetch_add(1, std::memory_order_release);
                std::swap(key,   slot.key);
                std::swap(value, slot.value);
                std::swap(disp,  slot.displacement);
                slot.version.fetch_add(1, std::memory_order_release);
            }
        }
        throw std::runtime_error("ConcurrentMap: table is full");
    }

    void rehash(std::size_t new_cap) {
        new_cap = detail::ceil_pow2(new_cap);
        std::vector<Slot> new_table(new_cap);
        std::size_t new_mask = new_cap - 1;

        for (auto& slot : table_)
            if (slot.occupied)
                insert_into(new_table, new_mask, std::move(slot.key), std::move(slot.value));

        table_ = std::move(new_table);
        mask_  = new_mask;
    }
};

} // namespace nexus
