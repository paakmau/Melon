#pragma once

#include <MelonCore/Event.h>

namespace Melon {

struct KeyDownEvent : public Event {
    int key;
};

struct KeyUpEvent : public Event {
    int key;
};

struct MouseButtonDownEvent : public Event {
    int button;
};

struct MouseButtonUpEvent : public Event {
    int button;
};

struct MouseScrollEvent : public Event {
    float xOffset;
    float yOffset;
};

}  // namespace Melon
