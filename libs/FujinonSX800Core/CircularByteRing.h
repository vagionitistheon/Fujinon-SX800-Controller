/**
 * @file CircularByteRing.h
 * @brief SPSC Zero-Copy Lock-Free Circular Byte Stream Ring Buffer.
 */

#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>

namespace FujinonSX800 {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4324) // structure was padded due to alignment specifier
#endif

/**
 * @brief Represents a contiguous memory byte region.
 */
struct ByteRegion {
    std::uint8_t* data {nullptr};
    std::size_t size {0};
};

/**
 * @brief Represents a contiguous const memory byte region.
 */
struct ConstByteRegion {
    const std::uint8_t* data {nullptr};
    std::size_t size {0};
};

/**
 * @brief Zero-copy write view containing up to 2 contiguous memory blocks.
 */
struct ZeroCopyWriteView {
    ByteRegion first;
    ByteRegion second;

    std::size_t totalSize() const noexcept
    {
        return first.size + second.size;
    }
};

/**
 * @brief Zero-copy read view containing up to 2 contiguous const memory blocks.
 */
struct ZeroCopyReadView {
    ConstByteRegion first;
    ConstByteRegion second;

    std::size_t totalSize() const noexcept
    {
        return first.size + second.size;
    }
};

/**
 * @brief Single-Producer Single-Consumer (SPSC) Lock-Free Circular Byte Ring.
 * @tparam Capacity Total capacity in bytes (must be a power of 2 and >= 2).
 */
template <std::size_t Capacity = 65536>
class CircularByteRing {
    static_assert(Capacity >= 2 && (Capacity & (Capacity - 1)) == 0,
                  "Capacity must be a power of 2");

public:
    static constexpr std::size_t Mask = Capacity - 1;
    static constexpr std::size_t npos = static_cast<std::size_t>(-1);

    CircularByteRing()
        : m_head(0)
        , m_tail(0)
    {
    }

    ~CircularByteRing() = default;

    CircularByteRing(const CircularByteRing&) = delete;
    CircularByteRing& operator=(const CircularByteRing&) = delete;
    CircularByteRing(CircularByteRing&&) = delete;
    CircularByteRing& operator=(CircularByteRing&&) = delete;

    /**
     * @brief Gets zero-copy write view for direct OS socket or serial receive (Producer only).
     */
    ZeroCopyWriteView getWriteView() noexcept
    {
        const auto currentTail = m_tail.load(std::memory_order_relaxed);
        const auto currentHead = m_head.load(std::memory_order_acquire);
        const std::size_t used = currentTail - currentHead;
        const std::size_t freeBytes = Capacity - used;

        ZeroCopyWriteView view {};
        if (freeBytes == 0) {
            return view;
        }

        const std::size_t tailIdx = currentTail & Mask;
        const std::size_t spaceToEnd = Capacity - tailIdx;
        const std::size_t firstSize = (freeBytes < spaceToEnd) ? freeBytes : spaceToEnd;
        const std::size_t secondSize = freeBytes - firstSize;

        view.first = ByteRegion {&m_buffer[tailIdx], firstSize};
        if (secondSize > 0) {
            view.second = ByteRegion {&m_buffer[0], secondSize};
        }
        return view;
    }

    /**
     * @brief Advances write cursor by specified bytes (Producer only).
     */
    void advanceWrite(std::size_t bytes) noexcept
    {
        const auto currentTail = m_tail.load(std::memory_order_relaxed);
        m_tail.store(currentTail + bytes, std::memory_order_release);
    }

    /**
     * @brief Gets zero-copy read view for direct parsing (Consumer only).
     */
    ZeroCopyReadView getReadView() const noexcept
    {
        const auto currentHead = m_head.load(std::memory_order_relaxed);
        const auto currentTail = m_tail.load(std::memory_order_acquire);
        const std::size_t available = currentTail - currentHead;

        ZeroCopyReadView view {};
        if (available == 0) {
            return view;
        }

        const std::size_t headIdx = currentHead & Mask;
        const std::size_t spaceToEnd = Capacity - headIdx;
        const std::size_t firstSize = (available < spaceToEnd) ? available : spaceToEnd;
        const std::size_t secondSize = available - firstSize;

        view.first = ConstByteRegion {&m_buffer[headIdx], firstSize};
        if (secondSize > 0) {
            view.second = ConstByteRegion {&m_buffer[0], secondSize};
        }
        return view;
    }

    /**
     * @brief Advances read cursor by specified bytes (Consumer only).
     */
    void advanceRead(std::size_t bytes) noexcept
    {
        const auto currentHead = m_head.load(std::memory_order_relaxed);
        m_head.store(currentHead + bytes, std::memory_order_release);
    }

    /**
     * @brief Searches for target byte delimiter in readable data (Consumer only).
     */
    std::size_t findByte(std::uint8_t delimiter) const noexcept
    {
        const auto view = getReadView();
        if (view.totalSize() == 0) {
            return npos;
        }

        if (view.first.size > 0) {
            const void* p = std::memchr(view.first.data, delimiter, view.first.size);
            if (p) {
                const auto* ptr = static_cast<const std::uint8_t*>(p);
                return static_cast<std::size_t>(ptr - view.first.data);
            }
        }

        if (view.second.size > 0) {
            const void* p = std::memchr(view.second.data, delimiter, view.second.size);
            if (p) {
                const auto* ptr = static_cast<const std::uint8_t*>(p);
                return view.first.size + static_cast<std::size_t>(ptr - view.second.data);
            }
        }

        return npos;
    }

    /**
     * @brief Copies up to count bytes without advancing read cursor (Consumer only).
     */
    bool peekBytes(std::uint8_t* dest, std::size_t count) const noexcept
    {
        if (!dest || count == 0) {
            return false;
        }

        const auto view = getReadView();
        if (view.totalSize() < count) {
            return false;
        }

        const std::size_t copyFirst = (count < view.first.size) ? count : view.first.size;
        std::memcpy(dest, view.first.data, copyFirst);

        if (count > copyFirst) {
            const std::size_t copySecond = count - copyFirst;
            std::memcpy(dest + copyFirst, view.second.data, copySecond);
        }

        return true;
    }

    /**
     * @brief Copies exact bytes into buffer (Producer only).
     */
    bool writeExact(const std::uint8_t* src, std::size_t count) noexcept
    {
        if (!src || count == 0) {
            return false;
        }

        const auto view = getWriteView();
        if (view.totalSize() < count) {
            return false;
        }

        const std::size_t copyFirst = (count < view.first.size) ? count : view.first.size;
        std::memcpy(view.first.data, src, copyFirst);

        if (count > copyFirst) {
            const std::size_t copySecond = count - copyFirst;
            std::memcpy(view.second.data, src + copyFirst, copySecond);
        }

        advanceWrite(count);
        return true;
    }

    /**
     * @brief Copies exact bytes out and advances read cursor (Consumer only).
     */
    bool readExact(std::uint8_t* dest, std::size_t count) noexcept
    {
        if (!peekBytes(dest, count)) {
            return false;
        }
        advanceRead(count);
        return true;
    }

    /**
     * @brief Returns total readable bytes available in buffer.
     */
    std::size_t availableRead() const noexcept
    {
        const auto currentHead = m_head.load(std::memory_order_relaxed);
        const auto currentTail = m_tail.load(std::memory_order_acquire);
        return currentTail - currentHead;
    }

    /**
     * @brief Returns total writable byte capacity remaining.
     */
    std::size_t availableWrite() const noexcept
    {
        return Capacity - availableRead();
    }

    /**
     * @brief Returns total fixed capacity.
     */
    constexpr std::size_t capacity() const noexcept
    {
        return Capacity;
    }

    /**
     * @brief Clears the ring buffer by resetting head and tail indices.
     */
    void clear() noexcept
    {
        m_head.store(0, std::memory_order_relaxed);
        m_tail.store(0, std::memory_order_relaxed);
    }

private:
#ifdef __cpp_lib_hardware_interference_size
    static constexpr std::size_t CacheLineSize = std::hardware_destructive_interference_size;
#else
    static constexpr std::size_t CacheLineSize = 64;
#endif

    alignas(CacheLineSize) std::atomic<std::size_t> m_head;
    alignas(CacheLineSize) std::atomic<std::size_t> m_tail;
    alignas(CacheLineSize) std::array<std::uint8_t, Capacity> m_buffer;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace FujinonSX800
