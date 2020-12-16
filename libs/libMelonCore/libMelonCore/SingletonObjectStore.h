#pragma once

#include <array>

namespace Melon {

template <std::size_t Count>
class SingletonObjectStore {
  public:
    template <typename Type>
    void push(unsigned int const& typeId, Type const& object) {
        m_Store[typeId] = new Type(object);
    }

    template <typename Type>
    void pop(unsigned int const& typeId) {
        delete m_Store[typeId];
        m_Store[typeId] = nullptr;
    }

    template <typename Type>
    Type* object(unsigned int const& typeId) const {
        return static_cast<Type*>(m_Store[typeId]);
    }

  private:
    std::array<void*, Count> m_Store;
};

}  // namespace Melon
