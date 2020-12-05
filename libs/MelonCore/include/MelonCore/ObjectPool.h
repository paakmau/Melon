#pragma once

#include <cstdlib>
#include <new>
#include <type_traits>
#include <vector>

namespace MelonCore {

template <typename Type>
class ObjectPool {
  public:
    static constexpr unsigned int kCountPerBuffer = 128;

    ObjectPool();
    ObjectPool(ObjectPool const&) = delete;
    ObjectPool(ObjectPool&& other);
    // ObejctPool can't be destroy util all objects are recycled
    ~ObjectPool();

    template <typename... Args>
    Type* request(Args&&... args);
    void recycle(Type* object);

  private:
    void createBuffer();
    std::vector<Type*> _buffer;
    std::vector<Type*> _pool;
};

template <typename Type>
ObjectPool<Type>::ObjectPool() {
    createBuffer();
}

template <typename Type>
ObjectPool<Type>::ObjectPool(ObjectPool&& other) : _buffer(std::move(other._buffer)), _pool(std::move(other._pool)) {}

template <typename Type>
ObjectPool<Type>::~ObjectPool() {
    for (Type const* buffer : _buffer)
        delete[] buffer;
    _buffer.clear();
    _pool.clear();
}

template <typename Type>
template <typename... Args>
Type* ObjectPool<Type>::request(Args&&... args) {
    if (_pool.empty())
        createBuffer();
    Type* object = _pool.back();
    new (object) Type(std::forward<Args>(args)...);
    _pool.pop_back();
    return object;
}

template <typename Type>
void ObjectPool<Type>::recycle(Type* object) {
    if constexpr (std::is_trivially_destructible<Type>())
        object->~Type();
    _pool.push_back(object);
}

template <typename Type>
void ObjectPool<Type>::createBuffer() {
    Type* p = _buffer.emplace_back(reinterpret_cast<Type*>(new std::aligned_storage_t<sizeof(Type), alignof(Type)>[kCountPerBuffer]));
    for (unsigned int i = 0; i < kCountPerBuffer; i++, p++)
        _pool.push_back(p);
}

}  // namespace MelonCore