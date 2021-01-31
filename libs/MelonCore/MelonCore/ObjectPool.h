#pragma once

#include <array>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <new>
#include <type_traits>
#include <vector>

namespace Melon {

// ObejctPool can't be destroy util all objects are recycled
template <typename Type>
class ObjectPool {
  public:
    static constexpr unsigned int k_CountPerBuffer = 128;

    ObjectPool();
    ObjectPool(const ObjectPool&) = delete;

    template <typename... Args>
    Type* request(Args&&... args);
    void recycle(Type* object);

  private:
    struct Buffer {
        alignas(Type) std::array<std::byte, sizeof(Type) * k_CountPerBuffer> buffer;
    };

    void createBuffer();
    std::vector<std::unique_ptr<Buffer>> m_Buffers;
    std::vector<Type*> m_FreeObjects;
};

template <typename Type>
ObjectPool<Type>::ObjectPool() {
    createBuffer();
}

template <typename Type>
template <typename... Args>
Type* ObjectPool<Type>::request(Args&&... args) {
    if (m_FreeObjects.empty())
        createBuffer();
    Type* object = m_FreeObjects.back();
    new (object) Type(std::forward<Args>(args)...);
    m_FreeObjects.pop_back();
    return object;
}

template <typename Type>
void ObjectPool<Type>::recycle(Type* object) {
    if constexpr (std::is_trivially_destructible<Type>())
        object->~Type();
    m_FreeObjects.push_back(object);
}

template <typename Type>
void ObjectPool<Type>::createBuffer() {
    Type* p = reinterpret_cast<Type*>(m_Buffers.emplace_back(std::make_unique<Buffer>()).get());
    for (unsigned int i = 0; i < k_CountPerBuffer; i++, p++)
        m_FreeObjects.push_back(p);
}

}  // namespace Melon