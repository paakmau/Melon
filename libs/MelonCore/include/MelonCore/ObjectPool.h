#pragma once

#include <cstdlib>
#include <new>
#include <type_traits>
#include <vector>

namespace MelonCore {

template <typename T>
class ObjectPool {
  public:
    static constexpr unsigned int kCountPerBuffer = 128;

    ObjectPool();
    ObjectPool(ObjectPool const&) = delete;
    ObjectPool(ObjectPool&& other);
    // ObejctPool can't be destroy util all objects are recycled
    ~ObjectPool();

    template <typename... Args>
    T* request(Args&&... args);
    void recycle(T* object);

  private:
    void createBuffer();
    std::vector<T*> _buffer;
    std::vector<T*> _pool;
};

template <typename T>
ObjectPool<T>::ObjectPool() {
    createBuffer();
}

template <typename T>
ObjectPool<T>::ObjectPool(ObjectPool&& other) : _buffer(std::move(other._buffer)), _pool(std::move(other._pool)) {}

template <typename T>
ObjectPool<T>::~ObjectPool() {
    for (T const* buffer : _buffer)
        delete[] buffer;
    _buffer.clear();
    _pool.clear();
}

template <typename T>
template <typename... Args>
T* ObjectPool<T>::request(Args&&... args) {
    if (_pool.empty())
        createBuffer();
    T* object = _pool.back();
    new (object) T(std::forward<Args>(args)...);
    _pool.pop_back();
    return object;
}

template <typename T>
void ObjectPool<T>::recycle(T* object) {
    if constexpr (std::is_trivially_destructible<T>())
        object->~T();
    _pool.push_back(object);
}

template <typename T>
void ObjectPool<T>::createBuffer() {
    T* p = _buffer.emplace_back(reinterpret_cast<T*>(new std::aligned_storage_t<sizeof(T), alignof(T)>[kCountPerBuffer]));
    for (unsigned int i = 0; i < kCountPerBuffer; i++, p++)
        _pool.push_back(p);
}

}  // namespace MelonCore