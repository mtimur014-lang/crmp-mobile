#pragma once

#include <cstdint>
#include <cstddef>

namespace crmp {

class MemoryHook {
public:
    static uintptr_t FindLibraryBase(const char* libName);
    static bool WriteMemory(uintptr_t address, const void* data, size_t size);
    static bool MakeNop(uintptr_t address, size_t size);
};

} // namespace crmp
