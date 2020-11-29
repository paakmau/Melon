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

    template <typename T>
    unsigned int push(const unsigned int& typeId, const T& object);
    void pop(const unsigned int& typeId, const unsigned int& index);

    template <typename T>
    const T* object(const unsigned int& index) const;

    unsigned int objectIndex(const unsigned int& typeId, const void* object) const;

    template <typename T>
    unsigned int objectIndex(const unsigned int& typeId, const T& object) const;

   private:
    struct ObjectWrapper {
        unsigned int typeId;
        const void* object;
        std::size_t (*hash)(const void* const&);
        bool (*equalTo)(const void* const&, const void* const&);
    };

    struct ObjectWrapperHash {
        std::size_t operator()(const ObjectWrapper& objectWrapper) const {
            return objectWrapper.typeId ^ objectWrapper.hash(objectWrapper.object);
        }
    };

    struct ObjectWrapperEqualTo {
        bool operator()(const ObjectWrapper& lhs, const ObjectWrapper& rhs) const {
            return lhs.typeId == rhs.typeId && lhs.equalTo(lhs.object, rhs.object);
        }
    };

    template <typename T>
    static std::size_t objectWrapperHash(const void* const& object);

    template <typename T>
    static bool objectWrapperEqualTo(const void* const& lhs, const void* const& rhs);

    template <typename T>
    static void objectDeleter(void* const& object) {
        delete static_cast<T*>(object);
    }

    std::array<std::size_t (*)(const void* const&), Count> _typeHashes;
    std::array<bool (*)(const void* const&, const void* const&), Count> _typeEqualTos;
    std::array<void (*)(void* const&), Count> _typeDeleters;

    unsigned int _indexCount{};
    std::vector<unsigned int> _freeIndices;

    std::vector<void*> _store;
    std::vector<unsigned int> _referenceCounts;

    std::unordered_map<ObjectWrapper, unsigned int, ObjectWrapperHash, ObjectWrapperEqualTo> _objectIndexMap;
};

template <std::size_t Count>
template <typename T>
inline unsigned int ObjectStore<Count>::push(const unsigned int& typeId, const T& object) {
    _typeHashes[typeId] = objectWrapperHash<T>;
    _typeEqualTos[typeId] = objectWrapperEqualTo<T>;
    _typeDeleters[typeId] = objectDeleter<T>;
    ObjectWrapper objectWrapper{
        typeId, static_cast<const void*>(&object),
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
        T* objectToSave = new T(object);
        objectWrapper.object = objectToSave;
        _objectIndexMap.emplace(objectWrapper, index);
        _store[index] = objectToSave;
    }
    _referenceCounts[index]++;
    return index;
}

template <std::size_t Count>
inline void ObjectStore<Count>::pop(const unsigned int& typeId, const unsigned int& index) {
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
template <typename T>
const T* ObjectStore<Count>::object(const unsigned int& index) const {
    if (index == kInvalidIndex) return nullptr;
    return static_cast<T*>(_store[index]);
}

template <std::size_t Count>
unsigned int ObjectStore<Count>::objectIndex(const unsigned int& typeId, const void* object) const {
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
template <typename T>
unsigned int ObjectStore<Count>::objectIndex(const unsigned int& typeId, const T& object) const {
    ObjectWrapper objectWrapper{
        typeId, static_cast<const void*>(&object),
        objectWrapperHash<T>,
        objectWrapperEqualTo<T>};
    unsigned int index;
    if (_objectIndexMap.contains(objectWrapper))
        return _objectIndexMap[objectWrapper];
    else
        return kInvalidIndex;
}

template <std::size_t Count>
template <typename T>
std::size_t ObjectStore<Count>::objectWrapperHash(const void* const& object) {
    return std::hash<T>()(*static_cast<const T*>(object));
}

template <std::size_t Count>
template <typename T>
bool ObjectStore<Count>::objectWrapperEqualTo(const void* const& lhs, const void* const& rhs) {
    return std::equal_to<T>()(*static_cast<const T*>(lhs), *static_cast<const T*>(rhs));
}

}  // namespace MelonCore
