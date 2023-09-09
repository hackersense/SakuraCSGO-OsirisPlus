#include "WindowUtils.h"

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) noexcept
{
    DWORD dwCurProcessId = *reinterpret_cast<DWORD*>(lParam);
    DWORD dwProcessId = 0;

    GetWindowThreadProcessId(hwnd, &dwProcessId);
    if (dwProcessId == dwCurProcessId && GetParent(hwnd) == nullptr)
    {
        *reinterpret_cast<HWND*>(lParam) = hwnd;
        return false;
    }
    return true;
}

HWND WindowUtils::GetMainWindow() noexcept
{
    DWORD dwCurrentProcessId = GetCurrentProcessId();
    if (!EnumWindows(EnumWindowsProc, LPARAM(&dwCurrentProcessId)))
        return HWND(dwCurrentProcessId);
    return nullptr;
}
