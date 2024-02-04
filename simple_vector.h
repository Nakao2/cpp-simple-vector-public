#pragma once
#include "array_ptr.h"

#include <cassert>
#include <initializer_list>
#include <new>
#include <stdexcept>
#include <algorithm>

using namespace std;

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t new_capacity) {
        capacity_ = new_capacity;
    }
    size_t GetCapacity() const {
        return capacity_;
    }
private:
    size_t capacity_ = 0;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size)
        : SimpleVector(size, Type{}) {
    }

    SimpleVector(size_t size, const Type& value) {
        ArrayPtr<Type>* new_array = new ArrayPtr<Type>(size);
        base_array_.swap(*new_array);
        std::fill(base_array_.Get(), base_array_.Get() + size, value);
        size_ = size;
        capacity_ = size;
    }

    SimpleVector(std::initializer_list<Type> init) {
        size_t in_size = init.size();
        ArrayPtr<Type>* new_array = new ArrayPtr<Type>(in_size);
        base_array_.swap(*new_array);
        std::copy(init.begin(), init.end(), base_array_.Get());
        size_ = in_size;
        capacity_ = in_size;
    }

    SimpleVector(const ReserveProxyObj& obj) {
        this->IncreaseCapacity(obj.GetCapacity());
    }

    SimpleVector(SimpleVector&& source)
        : base_array_(move(source.base_array_)) {
        std::swap(size_, source.size_);
        std::swap(capacity_, source.capacity_);
    }

    ~SimpleVector() = default;

    SimpleVector(const SimpleVector& other) {
        ArrayPtr<Type>* copy = new ArrayPtr<Type>(other.size_);
        std::copy(other.begin(), other.end(), copy->Get());
        base_array_.swap(*copy);
        size_ = other.size_;
        capacity_ = other.size_;
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector<Type>* copy = new SimpleVector<Type>(rhs);
            this->swap(*copy);
            delete copy;
        }
        return *this;
    }

    void PushBack(Type item) {
        if (size_ == capacity_) {
            this->IncreaseCapacity(1);
        }
        base_array_[size_] = std::move(item);
        ++size_;
    }

    Iterator Insert(ConstIterator pos, Type value) {
        assert(pos >= this->begin() && pos <= this->end());
        auto pos_num = pos - this->begin();
        if (size_ == capacity_) {
            this->IncreaseCapacity(1);
        }
        Iterator it = this->begin() + pos_num;
        std::move_backward(it, this->end(), this->end() + 1);
        *it = std::move(value);
        ++size_;
        return it;
    }

    void PopBack() noexcept {
        assert(size_ != 0);
        --size_;
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= this->begin() && pos < this->end());
        Iterator it = this->begin();
        while (it != pos) {
            ++it;
        }
        std::move(it + 1, this->end(), it);
        --size_;
        return it;
    }

    void swap(SimpleVector& other) noexcept {
        base_array_.swap(other.base_array_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            capacity_ = 0;                       // So the increase happens exactly to "new_capacity"
            this->IncreaseCapacity(new_capacity);
        }
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return (size_ == 0);
    }

    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return base_array_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return base_array_[index];
    }

    Type& At(size_t index) {
        if (index < size_) {
            return base_array_[index];
        }
        else {
            throw std::out_of_range("");
        }
    }

    const Type& At(size_t index) const {
        if (index < size_) {
            return base_array_[index];
        }
        else {
            throw std::out_of_range("");
        }
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        }
        else {
            if (new_size > capacity_) {
                this->IncreaseCapacity(new_size);
            }
            for (auto it = base_array_.Get() + size_; it != base_array_.Get() + new_size; ++it) {
                Type value{};
                (*it) = std::move(value);
            }
            size_ = new_size;
        }
    }

    Iterator begin() noexcept {
        return base_array_.Get();
    }

    Iterator end() noexcept {
        return base_array_.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        return base_array_.Get();
    }

    ConstIterator end() const noexcept {
        return base_array_.Get() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return base_array_.Get();
    }

    ConstIterator cend() const noexcept {
        return base_array_.Get() + size_;
    }

private:
    ArrayPtr<Type> base_array_;
    size_t size_ = 0;
    size_t capacity_ = 0;

    void IncreaseCapacity(size_t new_size) {
        size_t new_array_capacity = std::max(new_size, 2 * capacity_);
        ArrayPtr<Type>* new_array = new ArrayPtr<Type>(new_array_capacity);
        std::move(base_array_.Get(), base_array_.Get() + size_, new_array->Get());
        base_array_.swap(*new_array);
        delete new_array;
        capacity_ = new_array_capacity;
    }
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() == rhs.GetSize()) {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }
    return false;
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (rhs < lhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}