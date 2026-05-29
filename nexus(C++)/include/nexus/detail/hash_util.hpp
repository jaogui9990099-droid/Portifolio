#pragma once

#include <cstddef>
#include <cstdint>
#include <bit>

namespace nexus::detail {

// wyhash finalizer -- fast, good distribution, no deps
// see: https://github.com/wangyi-fudan/wyhash
inline constexpr uint64_t wyhash64(uint64_t key, uint64_t seed = 0xa0761d6478bd642full) noexcept {
    key ^= seed;
    key ^= key >> 33;
    key *= 0xff51afd7ed558ccdull;
    key ^= key >> 33;
    key *= 0xc4ceb9fe1a85ec53ull;
    key ^= key >> 33;
    return key;
}

// knuth multiplicative hash for 32-bit keys
inline constexpr uint32_t knuth32(uint32_t key) noexcept {
    return key * 2654435761u;
}

// smallest prime >= n. used to pick table sizes that reduce clustering
inline constexpr std::size_t next_prime(std::size_t n) noexcept {
    if (n < 2) return 2;
    if (n % 2 == 0) ++n;
    for (;;) {
        bool prime = true;
        for (std::size_t d = 3; d * d <= n; d += 2) {
            if (n % d == 0) { prime = false; break; }
        }
        if (prime) return n;
        n += 2;
    }
}

// round up to the next power of two
inline constexpr std::size_t ceil_pow2(std::size_t n) noexcept {
    if (n <= 1) return 1;
    return std::size_t{1} << (std::bit_width(n - 1));
}

// hash adapter -- specialize for custom types by providing:
//   template<> struct Hasher<T> { std::size_t operator()(const T&) const noexcept; };
template<typename T>
struct Hasher {
    std::size_t operator()(const T& v) const noexcept {
        return static_cast<std::size_t>(wyhash64(std::hash<T>{}(v)));
    }
};

} // namespace nexus::detail
