#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "../SDK/Platform.h"
#ifdef _WIN32
#include <x86RetSpoof.h>
#include "../Memory.h"
#endif

class VmtSwap {
public:
    void init(void* base) noexcept;
    void restore() noexcept
    {
        if (newVmt)
            *reinterpret_cast<std::uintptr_t**>(base) = oldVmt;
    }

    template<typename T>
    void hookAt(std::size_t index, T fun) const noexcept
    {
        newVmt[index + dynamicCastInfoLength] = reinterpret_cast<std::uintptr_t>(fun);
    }

    template<typename T, std::size_t Idx, typename ...Args>
    constexpr auto getOriginal(Args... args) const noexcept
    {
        return reinterpret_cast<T(__thiscall*)(void*, Args...)>(oldVmt[Idx]);
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
    static constexpr auto dynamicCastInfoLength = 1;

    void* base = nullptr;
    std::uintptr_t* oldVmt = nullptr;
    std::unique_ptr<std::uintptr_t[]> newVmt;
    std::size_t length = 0;
};