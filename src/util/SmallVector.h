// (c) Sam Donow 2018
#pragma once
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <functional>
#include <type_traits>

namespace detail {
    static constexpr size_t svDefaultSizeTag = -1;
}
template<typename T, size_t SizeParam = detail::svDefaultSizeTag>
class SmallVector {
    static constexpr size_t DefaultSize = sizeof(void *) * 2 - sizeof(uint8_t);
    static constexpr size_t N = std::max(1UL,
        SizeParam == detail::svDefaultSizeTag ?
            DefaultSize / sizeof(T)  : SizeParam);
    static_assert(N <= 255, "SmallVector size too large");
    // TODO: as a space optimization, we could stick the SSO size
    // into the bufStart ptr
    T* bufStart;
    struct SSO {
        T buf[N];
        uint8_t size;
    };

    struct NonSSO {
        // bufEnd points to the last element, not 1 past the end.
        // The cleverness allowed by keeping these as ptrs instead of size is mostly
        // lost by having to do the SSO, so I may change this
        T* bufEnd;
        T* capEnd;
    };

    union {
        SSO sso;
        NonSSO nonSSO;
    };

    bool isSSO() const noexcept {
        return bufStart == sso.buf;
    }

  public:

    size_t capacity() const noexcept { return isSSO() ? N : (nonSSO.capEnd - bufStart); }

    size_t size() const noexcept {
        return isSSO() ? sso.size : static_cast<size_t>(nonSSO.bufEnd - bufStart) + 1;
    }

  private:

    // TODO: use allocators or otherwise handle things better here?
    void reserveImpl(size_t cap) {
        T* newBuf = static_cast<T*>(operator new[](cap * sizeof(T)));
        const size_t startSize = size();
        for (size_t i = 0; i < startSize; ++i) {
            new (static_cast<void*>(newBuf + i)) T(std::move(bufStart[i]));
        }
        for (size_t i = 0; i < startSize; ++i) {
            bufStart[i].~T();
        }
        if (!isSSO()) {
            operator delete[](bufStart);
        }
        // The fact that we are reserving means we are no longer SSO
        bufStart        = newBuf;
        nonSSO.bufEnd   = newBuf + startSize - 1;
        nonSSO.capEnd   = newBuf + cap;
    }

    void ensureCapacity(size_t newSize) {
        if (capacity() < newSize) {
            reserveImpl(2 * newSize);
        }
    }

    void moveUp(size_t pos, size_t count) {
        for (size_t i = size() - 1; i >= pos + count; --i) {
            bufStart[i] = std::move(bufStart[i - count]);
        }
    }

    void moveDown(size_t pos, size_t count) {
        for (size_t i = pos; pos < size() - count; ++i) {
            bufStart[i] = std::move(bufStart[i + count]);
        }
    }

    void setSize(size_t size) noexcept {
        if (isSSO()) {
            sso.size = size;
        } else {
            nonSSO.bufEnd = bufStart + size - 1;
        }
    }

    template<bool allowShrink, typename Functor>
    void resizeImpl(size_t count, Functor construct) {
        static_assert(std::is_invocable_r_v<void, Functor, void*>);
        const size_t currSize = size();
        if (allowShrink && currSize > count) {
            for (T* ePtr = bufStart + currSize; ePtr >= bufStart + count; --ePtr) {
                ePtr->~T();
            }
            setSize(count);
        } else if (currSize < count) {
            ensureCapacity(count);
            T* ePtr = bufStart + currSize;
            const T* endPoint = bufStart + count;
            while (ePtr < endPoint) {
                construct(ePtr++);
            }
            setSize(count);
        }
    }

    void resizeNoShrink(size_t count) {
        resizeImpl<false>(count, [](void* ptr) { new (ptr) T(); });
    }

  public:

    ~SmallVector() {
        if (!isSSO()) {
            for (size_t i = 0; i < size(); ++i) {
                bufStart[i].~T();
            }
            operator delete[](bufStart);
        }
    }

    T& operator[](size_t pos) noexcept { return bufStart[pos]; }
    const T& operator[](size_t pos) const noexcept { return bufStart[pos]; }

    T& at(size_t pos) { return const_cast<T&>(static_cast<const SmallVector *>(this)->at(pos)); }
    const T& at(size_t pos) const  {
        if (pos < size()) {
            return bufStart[pos];
        } else {
            throw std::out_of_range("Out of range");
        }
    }

    T& front() noexcept { return *bufStart; }
    const T& front() const noexcept { return *bufStart; }

    // We implement back in terms of the start, not the end, because of SSO
    T& back() noexcept { return bufStart[size() - 1]; }
    const T& back() const noexcept { return bufStart[size() - 1]; }

    T* data() noexcept { return bufStart; }
    // Iterators are just ptrs
    T* begin() noexcept { return bufStart; }
    const T* begin() const noexcept { return bufStart; }
    const T* cbegin() const noexcept { return begin(); }

    T* end() noexcept { return bufStart + size(); }
    const T* end() const noexcept { return bufStart + size(); }
    const T* cend() const noexcept { return end(); }


    bool empty() const noexcept { return size() == 0; }

    void reserve(size_t cap) {
        if (cap > capacity()) {
            reserveImpl(cap);
        }
    }

    // In the standard this has the strong exception guarantee for std vector. Let's not bother
    void shrink_to_fit() noexcept {
        const size_t ourSize = size();
        if (ourSize < capacity() && ourSize > N) {
            reserveImpl(ourSize);
        }
    }

    void clear() noexcept {
        for (size_t i = 0; i < size(); ++i) {
            bufStart[i].~T();
        }
        setSize(0);
    }

    void resize(size_t count) {
        resizeImpl<true>(count, [](void *ptr) { new (ptr) T(); });
    }

    void resize(size_t count, const T& value) {
        resizeImpl<true>(count, [&value](void* ptr) { new (ptr) T(value); });
    }

    T* insert(const T* pos, size_t count, const T& value) {
        const size_t posVal = pos - bufStart;
        resizeNoShrink(size() + count);
        moveUp(posVal, count);
        T* const retPos = bufStart + posVal;
        for (size_t i = 0; i < count; ++i) {
            retPos[i] = value;
        }
        return retPos;
    }

    T* insert(const T* pos, const T& value) { return insert(pos, 1, value); }

    T* insert(const T* pos, T&& value) {
        const size_t posVal = pos - bufStart;
        resizeNoShrink(size() + 1);
        moveUp(posVal, 1);
        T* const retPos = bufStart + posVal;
        *retPos = std::move(value);
        return retPos;
    }

    template<class InputIt>
    T* insert(const T* pos, InputIt first, InputIt last) {
        const size_t count = static_cast<size_t>(std::distance(first, last));
        const size_t posVal = pos - bufStart;
        resizeNoShrink(size() + count);
        moveUp(posVal, count);
        T* const retPos = bufStart + posVal;
        for (size_t i = 0; i < count; ++i) {
            retPos[i] = *first++;
        }
        return retPos;
    }

    T* insert(const T* pos, std::initializer_list<T> ilist) {
        return insert(pos, ilist.begin(), ilist.end());
    }

    template<typename... Args>
    T* emplace(const T* pos, Args&&... args) {
        const size_t posVal = pos - bufStart;
        resizeNoShrink(size() + 1);
        moveUp(posVal, 1);
        T* const retPos = bufStart + posVal;
        retPos->~T();
        new (static_cast<void *>(retPos)) T(args...);
        return retPos;
    }

    T* erase(const T* pos) {
        return erase(pos, pos + 1);
    }

    T* erase(const T* pos, const T* last) {
        const size_t posVal = pos - bufStart - 1;
        const size_t count = static_cast<size_t>(last - pos);
        moveDown(posVal, count);
        setSize(size() - count);
        return bufStart + posVal;
    }

    void push_back(const T& value) {
        resizeImpl<false>(size() + 1, [&value](void *ptr) { new (ptr) T(value); });
    }

    void push_back(T&& value) {
        resizeImpl<false>(size() + 1, [&value](void *ptr) { new (ptr) T(std::move(value)); });
    }

    template<typename... Args>
    T& emplace_back(Args&&... args) {
        resizeImpl<false>(size() + 1, [&args...](void *ptr) { new (ptr) T(args...); });
        return back();
    }

    void pop_back() {
        const size_t currSize = size();
        bufStart[currSize].~T();
        setSize(currSize - 1);
    }

    SmallVector() : bufStart(sso.buf) {
        sso.size = 0;
    }

    SmallVector(const SmallVector& other) : bufStart(sso.buf) {
        sso.size = 0;
        resize(other.size());
        insert(begin(), other.begin(), other.end());
    }

    SmallVector& operator=(SmallVector& other) {
        clear();
        insert(begin(), other.begin(), other.end());
        return *this;
    }

    // Move assignment and construction are a bit tricky with SSO. Will do soon
};
