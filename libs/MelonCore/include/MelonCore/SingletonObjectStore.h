#pragma once

#include <array>

namespace MelonCore {

template <std::size_t Count>
class SingletonObjectStore {
  public:
    template <typename Type>
    void push(unsigned int const& typeId, Type const& object) {
        _store[typeId] = new Type(object);
    }

    template <typename Type>
    void pop(unsigned int const& typeId) {
        delete _store[typeId];
        _store[typeId] = nullptr;
    }

    template <typename Type>
    Type* object(unsigned int const& typeId) const {
        return static_cast<Type*>(_store[typeId]);
    }

  private:
    std::array<void*, Count> _store;
};

}  // namespace MelonCore
