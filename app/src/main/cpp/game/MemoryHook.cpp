#include "MemoryHook.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/mman.h>

namespace crmp {

uintptr_t MemoryHook::FindLibraryBase(const char* libName) {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return 0;

    char line[512];
    uintptr_t baseAddr = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, libName)) {
            baseAddr = static_cast<uintptr_t>(strtoull(line, nullptr, 16));
            break;
        }
    }
    fclose(fp);
    return baseAddr;
}

bool MemoryHook::WriteMemory(uintptr_t address, const void* data, size_t size) {
    if (address == 0 || data == nullptr || size == 0) return false;
    long pageSize = sysconf(_SC_PAGESIZE);
    uintptr_t pageStart = address & ~(pageSize - 1);
    size_t pageLen = ((address + size) - pageStart + pageSize - 1) & ~(pageSize - 1);

    if (mprotect(reinterpret_cast<void*>(pageStart), pageLen, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        return false;
    }
    std::memcpy(reinterpret_cast<void*>(address), data, size);
    __builtin___clear_cache(reinterpret_cast<char*>(address), reinterpret_cast<char*>(address + size));
    return true;
}

bool MemoryHook::MakeNop(uintptr_t address, size_t size) {
#if defined(__aarch64__)
    if (size % 4 != 0) return false;
    const uint32_t nopInstr = 0xD503201F;
    for (size_t i = 0; i < size; i += 4) {
        if (!WriteMemory(address + i, &nopInstr, 4)) return false;
    }
    return true;
#else
    if (size % 2 != 0) return false;
    const uint16_t nopInstr = 0xBF00;
    for (size_t i = 0; i < size; i += 2) {
        if (!WriteMemory(address + i, &nopInstr, 2)) return false;
    }
    return true;
#endif
}

} // namespace crmp
