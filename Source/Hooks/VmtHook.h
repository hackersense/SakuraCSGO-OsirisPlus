#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "../SDK/Platform.h"
#ifdef _WIN32
#include <x86RetSpoof.h>
#include "../Memory.h"
#endif

class VmtHook {
public:
    void init(void* base) noexcept;
    void restore() const noexcept;
    void hookAt(std::size_t index, void* fun) const noexcept;

    template<typename T, std::size_t Idx, typename ...Args>
    constexpr auto getOriginal(Args...) const noexcept
    {
        return reinterpret_cast<T(THISCALL_CONV*)(void*, Args...)>(oldVmt[Idx]);
    }

    template<typename T, std::size_t Idx, typename ...Args>
    constexpr auto callOriginal(Args... args) const noexcept
    {
#ifdef _WIN32
        return x86RetSpoof::invokeThiscall<T, Args...>(std::uintptr_t(base), oldVmt[Idx], memory->jmpEbxGadgetInClient, args...);
#else
        return getOriginal<T, Idx>(args...)(base, args...);
#endif
    }

    template<typename T, typename Base, std::size_t Idx, typename ...Args>
    constexpr auto callOriginal(Base base, Args... args) const noexcept
    {
#ifdef _WIN32
        return x86RetSpoof::invokeThiscall<T, Args...>(std::uintptr_t(base), oldVmt[Idx], memory->jmpEbxGadgetInClient, args...);
#else
        return getOriginal<T, Idx>(args...)(base, args...);
#endif
    }

private:
    void* base;
    std::size_t length;
    std::unique_ptr<std::uintptr_t[]> oldVmt;
};
