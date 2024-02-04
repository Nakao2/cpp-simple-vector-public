#pragma once

#include <algorithm>

using namespace std;


template <typename Type>
class ArrayPtr {
public:

    ArrayPtr() = default;

    explicit ArrayPtr(size_t size) {
        if (size != 0) {
            raw_ptr_ = new Type[size + 1];  // Last element exists for an "end" pointer
        }
    }

    explicit ArrayPtr(Type* raw_ptr) noexcept
        : raw_ptr_(raw_ptr) {
    }

    ArrayPtr(ArrayPtr&& source) {
        raw_ptr_ = source.Release();
    }

    ArrayPtr(const ArrayPtr&) = delete;

    ~ArrayPtr() {
        delete[] raw_ptr_;
    }

    ArrayPtr& operator=(const ArrayPtr&) = delete;

    ArrayPtr& operator=(ArrayPtr&& source) {
        if (this != &source) {
            this->swap(source);
        }
        return *this;
    }

    [[nodiscard]] Type* Release() noexcept {
        Type* output = raw_ptr_;
        raw_ptr_ = nullptr;
        return output;
    }

    Type& operator[](size_t index) noexcept {
        return raw_ptr_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return raw_ptr_[index];
    }

    explicit operator bool() const {
        return static_cast<bool>(raw_ptr_);
    }

    Type* Get() const noexcept {
        return raw_ptr_;
    }

    void swap(ArrayPtr& other) noexcept {
        std::swap(raw_ptr_, other.raw_ptr_);
    }

private:
    Type* raw_ptr_ = nullptr;
};