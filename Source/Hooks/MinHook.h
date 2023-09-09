#pragma once

#include <cstddef>
#include <memory>

#include "../SDK/Platform.h"
#ifdef _WIN32
#include "../x86RetSpoof.h"
#include "../Memory.h"
#endif

class MinHook {
public:
    void init(void* base) noexcept;
    void detour(uintptr_t base, void* fun) noexcept;
    void restore() noexcept {}
    void hookAt(std::size_t index, void* fun) noexcept;
    void enable(std::size_t index) noexcept;

    template<typename T, std::size_t Idx = 0, typename ...Args>
    constexpr auto getOriginal(Args... args) const noexcept
    {
        if (!Idx)
            return reinterpret_cast<T(__thiscall*)(void*, Args...)>(original);
        return reinterpret_cast<T(__thiscall*)(void*, Args...)>(originals[Idx]);
    }

    template<typename T, std::size_t Idx = 0, typename ...Args>
    constexpr auto callOriginal(Args... args) const noexcept
    {
#ifdef _WIN32
        if (!Idx)
            return x86RetSpoof::invokeThiscall<T, Args...>(std::uintptr_t(base), original, memory->jmpEbxGadgetInClient, args...);
        return x86RetSpoof::invokeThiscall<T, Args...>(std::uintptr_t(base), originals[Idx], memory->jmpEbxGadgetInClient, args...);
#else
        return getOriginal<T, Idx>(args...)(base, args...);
#endif
    }

    template<typename T, typename Base, std::size_t Idx = 0, typename ...Args>
    constexpr auto callOriginal(Base base, Args... args) const noexcept
    {
#ifdef _WIN32
        if (!Idx)
            return x86RetSpoof::invokeThiscall<T, Args...>(std::uintptr_t(base), original, memory->jmpEbxGadgetInClient, args...);
        return x86RetSpoof::invokeThiscall<T, Args...>(std::uintptr_t(base), originals[Idx], memory->jmpEbxGadgetInClient, args...);
#else
        return getOriginal<T, Idx>(args...)(base, args...);
#endif
    }

    auto getDetour() noexcept
    {
        return original;
    }

private:
    void* base;
    std::unique_ptr<uintptr_t[]> originals;
    uintptr_t original;
};