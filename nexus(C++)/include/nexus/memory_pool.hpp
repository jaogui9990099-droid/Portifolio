#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>

namespace nexus {

// fixed-size slab allocator.
//
// pre-allocates 'capacity' objects of type T in one contiguous block.
// alloc() and free() are both O(1), zero external fragmentation.
// not thread-safe -- wrap with a mutex if shared across threads.
template<typename T>
class MemoryPool {
public:
    // T must be at least pointer-sized so we can store the free-list link in the slot
    static_assert(sizeof(T) >= sizeof(void*),
                  "T must be at least pointer-sized for the free-list to work");

    explicit MemoryPool(std::size_t capacity)
        : capacity_(capacity)
        , storage_(new std::byte[sizeof(T) * capacity])
    {
        // build the intrusive free-list
        for (std::size_t i = 0; i < capacity - 1; ++i)
            *reinterpret_cast<void**>(slot(i)) = slot(i + 1);
        *reinterpret_cast<void**>(slot(capacity - 1)) = nullptr;
        head_ = slot(0);
    }

    MemoryPool(const MemoryPool&)            = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;
    MemoryPool(MemoryPool&&)                 = delete;
    MemoryPool& operator=(MemoryPool&&)      = delete;

    ~MemoryPool() = default;

    [[nodiscard]] T* allocate() {
        if (!head_) [[unlikely]]
            throw std::bad_alloc{};
        void* ptr = head_;
        head_     = *reinterpret_cast<void**>(head_);
        ++in_use_;
        return static_cast<T*>(ptr);
    }

    void deallocate(T* ptr) noexcept {
        if (!ptr) return;
        *reinterpret_cast<void**>(ptr) = head_;
        head_    = ptr;
        --in_use_;
    }

    std::size_t capacity()  const noexcept { return capacity_; }
    std::size_t in_use()    const noexcept { return in_use_; }
    std::size_t available() const noexcept { return capacity_ - in_use_; }
    bool        empty()     const noexcept { return in_use_ == 0; }
    bool        full()      const noexcept { return in_use_ == capacity_; }

private:
    void* slot(std::size_t i) noexcept {
        return storage_.get() + (i * sizeof(T));
    }

    const std::size_t            capacity_;
    std::unique_ptr<std::byte[]> storage_;
    void*                        head_{nullptr};
    std::size_t                  in_use_{0};
};

} // namespace nexus
