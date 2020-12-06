# Melon

A cross-platform data-driven game engine written in C++.

**Data Driven**. Melon implement an Entity Component System (ECS) framework, which is massively multi-threaded and cache-friendly.

**Modular Design**. Melon consists of multiple optional modules. Each module can be compiled and linked independently, allowing convenient frontend backend separation.

## Features

* Simple to use

* Data stored in contiguous memory

* Task-based asynchronous pattern

* Can detect entity creation and destruction in a ECS way

* Can detect component addition and removement in a ECS way

* Component sharing supported

* Singleton supported

* Simple entity filter supported

* Based on Vulkan

## Concepts

Melon implement an rigorous ECS framework with multiple convenient mechanisms.

**Entity**. As defined in ECS architecture, an Entity is just a id to bind different type of components, it does not have behaviour and data.

**Component**. A Component is a struct that contains only data. Each Component will occupy a block of memory, and Components of the same type are stored contiguously in a way.

> Notice that pointers are not recommanded to be stored in it, especially smart pointer, because the memory management uses memset.

**System**. A System uses EntityFilters to get a set of Entities, and then conducts behaviours on the data binded to them.

**EntityFilter**. An EntityFilter specify conditions to filter Entities, such as with or without certain types of Components.

**SharedComponent**. A SharedComponent can be shared among Entities. Entities with the same SharedComponent are organized contiguously, which is helpful to implement render batch, etc.

**SingletonComponent**. A SingletonComponent is independent and not binded to an Entity. It is used for global data and singleton pattern.

**ManualComponent**. A ManualComponent will not be removed when destroying the binding Entity. An well-designed EntityFilter can be used to detect the Entity destruction, and then the ManualComponent should be mananually removed.

> Entity creation can be detected by filtering Entities without certain type of ManualComponent, and then manually add a ManualComponent of this type to avoid duplicate detection.
> Component additon and removement can be detected in a similar way.

**ManualSharedComponent**. A ManualSharedComponent is similar to a ManualComponent

## License

Melon is licensed under the BSD 3-Clause License.

## Acknowledgements

[GLFW](https://github.com/glfw/glfw)

[GLM](https://github.com/g-truc/glm)

[glslang](https://github.com/KhronosGroup/glslang)

[volk](https://github.com/zeux/volk)

[Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers)

[Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
