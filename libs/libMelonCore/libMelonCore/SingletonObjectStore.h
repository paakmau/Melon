#pragma once

#include <array>

namespace Melon {

template <std::size_t Count>
class SingletonObjectStore {
  public:
    template <typename Type>
    void push(const unsigned int& typeId, const Type& object) {
        m_Store[typeId] = new Type(object);
    }

    template <typename Type>
    void pop(const unsigned int& typeId) {
        delete m_Store[typeId];
        m_Store[typeId] = nullptr;
    }

    template <typename Type>
    Type* object(const unsigned int& typeId) const {
        return static_cast<Type*>(m_Store[typeId]);
    }

  private:
    std::array<void*, Count> m_Store;
};

}  // namespace Melon
