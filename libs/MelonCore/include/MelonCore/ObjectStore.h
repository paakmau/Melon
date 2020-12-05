#pragma once

#include <array>
#include <climits>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace MelonCore {

template <std::size_t Count>
class ObjectStore {
  public:
    static constexpr unsigned int kInvalidIndex = std::numeric_limits<unsigned int>::max();

    template <typename Type>
    unsigned int push(unsigned int const& typeId, Type const& object);
    void pop(unsigned int const& typeId, unsigned int const& index);

    template <typename Type>
    Type const* object(unsigned int const& index) const;

    unsigned int objectIndex(unsigned int const& typeId, void const* object) const;

    template <typename Type>
    unsigned int objectIndex(unsigned int const& typeId, Type const& object) const;

  private:
    struct ObjectWrapper {
        unsigned int typeId;
        void const* object;
        std::size_t (*hash)(void const* const&);
        bool (*equalTo)(void const* const&, void const* const&);
    };

    struct ObjectWrapperHash {
        std::size_t operator()(ObjectWrapper const& objectWrapper) const {
            return objectWrapper.typeId ^ objectWrapper.hash(objectWrapper.object);
        }
    };

    struct ObjectWrapperEqualTo {
        bool operator()(ObjectWrapper const& lhs, ObjectWrapper const& rhs) const {
            return lhs.typeId == rhs.typeId && lhs.equalTo(lhs.object, rhs.object);
        }
    };

    template <typename Type>
    static std::size_t objectWrapperHash(void const* const& object);

    template <typename Type>
    static bool objectWrapperEqualTo(void const* const& lhs, void const* const& rhs);

    template <typename Type>
    static void objectDeleter(void* const& object) {
        delete static_cast<Type*>(object);
    }

    std::array<std::size_t (*)(void const* const&), Count> _typeHashes;
    std::array<bool (*)(void const* const&, void const* const&), Count> _typeEqualTos;
    std::array<void (*)(void* const&), Count> _typeDeleters;

    unsigned int _indexCount{};
    std::vector<unsigned int> _freeIndices;

    std::vector<void*> _store;
    std::vector<unsigned int> _referenceCounts;

    std::unordered_map<ObjectWrapper, unsigned int, ObjectWrapperHash, ObjectWrapperEqualTo> _objectIndexMap;
};

template <std::size_t Count>
template <typename Type>
inline unsigned int ObjectStore<Count>::push(unsigned int const& typeId, Type const& object) {
    _typeHashes[typeId] = objectWrapperHash<Type>;
    _typeEqualTos[typeId] = objectWrapperEqualTo<Type>;
    _typeDeleters[typeId] = objectDeleter<Type>;
    ObjectWrapper objectWrapper{
        typeId, static_cast<void const*>(&object),
        _typeHashes[typeId],
        _typeEqualTos[typeId]};
    unsigned int index;
    if (_objectIndexMap.contains(objectWrapper))
        index = _objectIndexMap[objectWrapper];
    else {
        if (_freeIndices.empty()) {
            index = _indexCount++;
            _store.emplace_back(nullptr);
            _referenceCounts.emplace_back(0U);
        } else
            index = _freeIndices.back(), _freeIndices.pop_back();
        Type* objectToSave = new Type(object);
        objectWrapper.object = objectToSave;
        _objectIndexMap.emplace(objectWrapper, index);
        _store[index] = objectToSave;
    }
    _referenceCounts[index]++;
    return index;
}

template <std::size_t Count>
inline void ObjectStore<Count>::pop(unsigned int const& typeId, unsigned int const& index) {
    if (index == kInvalidIndex) return;
    void* object = _store[index];
    _referenceCounts[index]--;
    bool removed = _referenceCounts[index] == 0;
    if (removed) {
        _freeIndices.push_back(index);

        ObjectWrapper objectWrapper{
            typeId, object,
            _typeHashes[typeId],
            _typeEqualTos[typeId]};
        _objectIndexMap.erase(objectWrapper);

        _typeDeleters[typeId](object);
    }
}

template <std::size_t Count>
template <typename Type>
Type const* ObjectStore<Count>::object(unsigned int const& index) const {
    if (index == kInvalidIndex) return nullptr;
    return static_cast<Type*>(_store[index]);
}

template <std::size_t Count>
unsigned int ObjectStore<Count>::objectIndex(unsigned int const& typeId, void const* object) const {
    if (_typeHashes[typeId] == nullptr) return kInvalidIndex;
    ObjectWrapper objectWrapper{
        typeId, object,
        _typeHashes[typeId],
        _typeEqualTos[typeId]};
    unsigned int index;
    if (_objectIndexMap.contains(objectWrapper))
        return _objectIndexMap[objectWrapper];
    else
        return kInvalidIndex;
}

template <std::size_t Count>
template <typename Type>
unsigned int ObjectStore<Count>::objectIndex(unsigned int const& typeId, Type const& object) const {
    ObjectWrapper objectWrapper{
        typeId, static_cast<void const*>(&object),
        objectWrapperHash<Type>,
        objectWrapperEqualTo<Type>};
    unsigned int index;
    if (_objectIndexMap.contains(objectWrapper))
        return _objectIndexMap[objectWrapper];
    else
        return kInvalidIndex;
}

template <std::size_t Count>
template <typename Type>
std::size_t ObjectStore<Count>::objectWrapperHash(void const* const& object) {
    return std::hash<Type>()(*static_cast<Type const*>(object));
}

template <std::size_t Count>
template <typename Type>
bool ObjectStore<Count>::objectWrapperEqualTo(void const* const& lhs, void const* const& rhs) {
    return std::equal_to<Type>()(*static_cast<Type const*>(lhs), *static_cast<Type const*>(rhs));
}

}  // namespace MelonCore
