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
    static constexpr unsigned int k_InvalidIndex = std::numeric_limits<unsigned int>::max();

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

    std::array<std::size_t (*)(void const* const&), Count> m_TypeHashes;
    std::array<bool (*)(void const* const&, void const* const&), Count> m_TypeEqualTos;
    std::array<void (*)(void* const&), Count> m_TypeDeleters;

    unsigned int m_IndexCount{};
    std::vector<unsigned int> m_FreeIndices;

    std::vector<void*> m_Store;
    std::vector<unsigned int> m_ReferenceCounts;

    std::unordered_map<ObjectWrapper, unsigned int, ObjectWrapperHash, ObjectWrapperEqualTo> m_ObjectIndexMap;
};

template <std::size_t Count>
template <typename Type>
inline unsigned int ObjectStore<Count>::push(unsigned int const& typeId, Type const& object) {
    m_TypeHashes[typeId] = objectWrapperHash<Type>;
    m_TypeEqualTos[typeId] = objectWrapperEqualTo<Type>;
    m_TypeDeleters[typeId] = objectDeleter<Type>;
    ObjectWrapper objectWrapper{
        typeId, static_cast<void const*>(&object),
        m_TypeHashes[typeId],
        m_TypeEqualTos[typeId]};
    unsigned int index;
    if (m_ObjectIndexMap.contains(objectWrapper))
        index = m_ObjectIndexMap[objectWrapper];
    else {
        if (m_FreeIndices.empty()) {
            index = m_IndexCount++;
            m_Store.emplace_back(nullptr);
            m_ReferenceCounts.emplace_back(0U);
        } else
            index = m_FreeIndices.back(), m_FreeIndices.pop_back();
        Type* objectToSave = new Type(object);
        objectWrapper.object = objectToSave;
        m_ObjectIndexMap.emplace(objectWrapper, index);
        m_Store[index] = objectToSave;
    }
    m_ReferenceCounts[index]++;
    return index;
}

template <std::size_t Count>
inline void ObjectStore<Count>::pop(unsigned int const& typeId, unsigned int const& index) {
    if (index == k_InvalidIndex) return;
    void* object = m_Store[index];
    m_ReferenceCounts[index]--;
    bool removed = m_ReferenceCounts[index] == 0;
    if (removed) {
        m_FreeIndices.push_back(index);

        ObjectWrapper objectWrapper{
            typeId, object,
            m_TypeHashes[typeId],
            m_TypeEqualTos[typeId]};
        m_ObjectIndexMap.erase(objectWrapper);

        m_TypeDeleters[typeId](object);
    }
}

template <std::size_t Count>
template <typename Type>
Type const* ObjectStore<Count>::object(unsigned int const& index) const {
    if (index == k_InvalidIndex) return nullptr;
    return static_cast<Type*>(m_Store[index]);
}

template <std::size_t Count>
unsigned int ObjectStore<Count>::objectIndex(unsigned int const& typeId, void const* object) const {
    if (m_TypeHashes[typeId] == nullptr) return k_InvalidIndex;
    ObjectWrapper objectWrapper{
        typeId, object,
        m_TypeHashes[typeId],
        m_TypeEqualTos[typeId]};
    unsigned int index;
    if (m_ObjectIndexMap.contains(objectWrapper))
        return m_ObjectIndexMap[objectWrapper];
    else
        return k_InvalidIndex;
}

template <std::size_t Count>
template <typename Type>
unsigned int ObjectStore<Count>::objectIndex(unsigned int const& typeId, Type const& object) const {
    ObjectWrapper objectWrapper{
        typeId, static_cast<void const*>(&object),
        objectWrapperHash<Type>,
        objectWrapperEqualTo<Type>};
    unsigned int index;
    if (m_ObjectIndexMap.contains(objectWrapper))
        return m_ObjectIndexMap[objectWrapper];
    else
        return k_InvalidIndex;
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
