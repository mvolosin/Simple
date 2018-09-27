//#include "stdafx.h"
#ifndef SIMPLE_STATIC_MAP_H
#define SIMPLE_STATIC_MAP_H

#include <cstdint>
#include <cstdlib>
#include <tuple>

namespace Simple {
namespace detail {

template <class T, std::size_t Size>
class storage {
    static const size_t TypeSize = sizeof(T);
    static const size_t TypeAlign = alignof(T);
    static const size_t StorageSize = Size * TypeSize;
    static const size_t LastTypeBegin = StorageSize - TypeSize;

    using StorageType = typename std::aligned_storage<TypeSize, TypeAlign>::type;
    StorageType storage_[Size];

public:
    void* data()
    {
        return static_cast<void*>(&storage_);
    }

    T& operator[](size_t index)
    {
        return reinterpret_cast<T&>(storage_[index * TypeSize]);
    }

    T& at(size_t index)
    {
        const size_t start = index * TypeSize;
        if (start > LastTypeBegin) {
            throw std::out_of_range("Cannot access type on given index.");
        }
        return reinterpret_cast<T&>(storage_[index * TypeSize]);
    }
};
} // namespace detail

template <class Key, class T, std::size_t Capacity>
class StaticMap {
public:
    using key_type = Key;
    using mapped_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using value_type = std::pair<const Key, T&>;
    using size_type = size_t;

    virtual ~StaticMap() noexcept
    {
        clear();
    }

    reference operator[](const Key& key)
    {
        auto o = findKey(key);
        if (o.second) {
            return values_[o.first];
        }
        new (&values_[size_]) mapped_type(); 
        new (&keys_[size_]) key_type(key); 
        return values_[size_++];
    }

    reference at(const Key& key)
    {
        auto o = findKey(key);
        if (o.second) {
            return values_[o.first];
        } else 
            throw std::out_of_range("Specified key not found.");
    }

    void clear()
    {
        for (size_t i = 0; i < size_; ++i) {
            keys_[i].~key_type();
            values_[i].~mapped_type();
        }
        size_ = 0;
    }

    size_type size() const
    {
        return size_;
    }

    bool empty() const
    {
        return size_ == 0;
    }

private:
    std::pair<size_t, bool> findKey(const Key& key)
    {
        for (size_t i = 0; i < size_; ++i) {
            if (key == keys_[i]) {
                return std::pair<size_t, bool>(i, true);
            }
        }
        return std::pair<size_t, bool>(0, false);
    }

    std::size_t size_{0};
    detail::storage<mapped_type, Capacity> values_;
    detail::storage<key_type, Capacity> keys_;
};
} // namespace Simple

#endif /* ifndef SIMPLE_STATIC_MAP_H */
