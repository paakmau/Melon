#pragma once

#include <cstdlib>
#include <new>
#include <type_traits>
#include <vector>

namespace MelonCore {

template <typename Type>
class ObjectPool {
  public:
    static constexpr unsigned int k_CountPerBuffer = 128;

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
    std::vector<Type*> m_Buffer;
    std::vector<Type*> m_Pool;
};

template <typename Type>
ObjectPool<Type>::ObjectPool() {
    createBuffer();
}

template <typename Type>
ObjectPool<Type>::ObjectPool(ObjectPool&& other) : m_Buffer(std::move(other.m_Buffer)), m_Pool(std::move(other.m_Pool)) {}

template <typename Type>
ObjectPool<Type>::~ObjectPool() {
    for (Type const* buffer : m_Buffer)
        delete[] buffer;
    m_Buffer.clear();
    m_Pool.clear();
}

template <typename Type>
template <typename... Args>
Type* ObjectPool<Type>::request(Args&&... args) {
    if (m_Pool.empty())
        createBuffer();
    Type* object = m_Pool.back();
    new (object) Type(std::forward<Args>(args)...);
    m_Pool.pop_back();
    return object;
}

template <typename Type>
void ObjectPool<Type>::recycle(Type* object) {
    if constexpr (std::is_trivially_destructible<Type>())
        object->~Type();
    m_Pool.push_back(object);
}

template <typename Type>
void ObjectPool<Type>::createBuffer() {
    Type* p = m_Buffer.emplace_back(reinterpret_cast<Type*>(new std::aligned_storage_t<sizeof(Type), alignof(Type)>[k_CountPerBuffer]));
    for (unsigned int i = 0; i < k_CountPerBuffer; i++, p++)
        m_Pool.push_back(p);
}

}  // namespace MelonCore