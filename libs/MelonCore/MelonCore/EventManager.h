#pragma once

#include <MelonCore/Event.h>

#include <array>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace Melon {

class EventBufferBase {
  public:
    virtual ~EventBufferBase(){};
    virtual void flush() = 0;
};

template <typename Type>
class EventBuffer : public EventBufferBase {
  public:
    void push(const Type& event) { nextEvents().push_back(event); }
    typename std::vector<Type>::const_iterator begin() const { return currentEvents().begin(); }
    typename std::vector<Type>::const_iterator end() const { return currentEvents().end(); }
    virtual void flush() final {
        currentEvents().clear();
        m_CurrentEventArrayIndex ^= 1;
    }

  private:
    const std::vector<Type>& currentEvents() const { return m_EventArrays[m_CurrentEventArrayIndex]; }
    std::vector<Type>& currentEvents() { return m_EventArrays[m_CurrentEventArrayIndex]; }
    std::vector<Type>& nextEvents() { return m_EventArrays[m_CurrentEventArrayIndex ^ 1]; }

    std::array<std::vector<Type>, 2> m_EventArrays;
    unsigned int m_CurrentEventArrayIndex{};
};

class EventManager {
  public:
    template <typename Type>
    unsigned int eventId() { return eventId<Type>(typeid(Type)); }
    template <typename Type>
    unsigned int eventId(const std::type_index& typeIndex) {
        auto res = m_EventBufferIdMap.try_emplace(typeIndex, m_EventBufferIdMap.size());
        if (res.second)
            m_EventBuffers.emplace_back(std::make_unique<EventBuffer<Type>>());
        return res.first->second;
    }
    template <typename Type>
    void send(const Type& event) { send<Type>(eventId<Type>(), event); }
    template <typename Type>
    void send(const unsigned int& eventId, const Type& event) { static_cast<EventBuffer<Type>*>(m_EventBuffers[eventId].get())->push(event); }
    template <typename Type>
    void send(const std::vector<Type>& events) { send<Type>(eventId<Type>(), events); }
    template <typename Type>
    void send(const unsigned int& eventId, const std::vector<Type>& events) {
        auto* eventBuffer = static_cast<EventBuffer<Type>*>(m_EventBuffers[eventId].get());
        for (const auto& event : events)
            eventBuffer->push(event);
    }
    template <typename Type>
    typename std::vector<Type>::const_iterator begin() { return begin<Type>(eventId<Type>()); }
    template <typename Type>
    typename std::vector<Type>::const_iterator begin(const unsigned int& eventId) { return static_cast<EventBuffer<Type>*>(m_EventBuffers[eventId].get())->begin(); }
    template <typename Type>
    typename std::vector<Type>::const_iterator end() { return end<Type>(eventId<Type>()); }
    template <typename Type>
    typename std::vector<Type>::const_iterator end(const unsigned int& eventId) { return static_cast<EventBuffer<Type>*>(m_EventBuffers[eventId].get())->end(); }

    void update() {
        for (const auto& eventBuffer : m_EventBuffers)
            eventBuffer->flush();
    }

  private:
    std::unordered_map<std::type_index, unsigned int> m_EventBufferIdMap;
    std::vector<std::unique_ptr<EventBufferBase>> m_EventBuffers;
};

}  // namespace Melon
