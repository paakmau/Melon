#pragma once

#include <array>

namespace MelonCore {

template <std::size_t Count>
class SingletonObjectStore {
  public:
    template <typename T>
    void push(unsigned int const& typeId, T const& object) {
        _store[typeId] = new T(object);
    }

    template <typename T>
    void pop(unsigned int const& typeId) {
        delete _store[typeId];
        _store[typeId] = nullptr;
    }

    template <typename T>
    T* object(unsigned int const& typeId) const {
        return static_cast<T*>(_store[typeId]);
    }

  private:
    std::array<void*, Count> _store;
};

}  // namespace MelonCore
