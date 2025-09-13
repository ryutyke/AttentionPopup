#include "GlobalInputWatcher.h"

#if PLATFORM_WINDOWS
// Windows include hygiene
THIRD_PARTY_INCLUDES_START
#define NOMINMAX
#include "Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include "Windows/HideWindowsPlatformTypes.h"
#include <Psapi.h>
THIRD_PARTY_INCLUDES_END

#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "Misc/ScopeLock.h"
#include "Misc/Paths.h"

TSharedPtr<FGlobalInputWatcher> FGlobalInputWatcher::Create(const TArray<FString>& InProcessWhitelist, FOnExternalActivity InOnActivity)
{
    //TSharedPtr<FGlobalInputWatcher> Ptr = MakeShared<FGlobalInputWatcher>();
    TSharedPtr<FGlobalInputWatcher> Ptr = MakeShareable(new FGlobalInputWatcher());
    if (!Ptr->Init(InProcessWhitelist, MoveTemp(InOnActivity)))
    {
        Ptr.Reset();
    }
    return Ptr;
}

bool FGlobalInputWatcher::Init(const TArray<FString>& InProcessWhitelist, FOnExternalActivity InOnActivity)
{
    Whitelist.Empty();
    for (const FString& Name : InProcessWhitelist)
    {
        Whitelist.Add(Name.ToLower());
    }
    ActivityDelegate = InOnActivity;
    LastFireTime = 0.0;

    KeyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, &LowLevelKeyboardProc, GetModuleHandleW(nullptr), 0);
    MouseHook = SetWindowsHookExW(WH_MOUSE_LL, &LowLevelMouseProc, GetModuleHandleW(nullptr), 0);

    if (!KeyboardHook || !MouseHook)
    {
        Shutdown();
        return false;
    }
    return true;
}

void FGlobalInputWatcher::Shutdown()
{
    if (KeyboardHook)
    {
        UnhookWindowsHookEx(KeyboardHook);
        KeyboardHook = nullptr;
    }
    if (MouseHook)
    {
        UnhookWindowsHookEx(MouseHook);
        MouseHook = nullptr;
    }
    ActivityDelegate.Unbind();
    Whitelist.Empty();
}

FGlobalInputWatcher::~FGlobalInputWatcher()
{
    Shutdown();
}

LRESULT CALLBACK FGlobalInputWatcher::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (!FSlateApplication::IsInitialized())
    {
        return CallNextHookEx(MouseHook, nCode, wParam, lParam);
    }

    if (nCode == HC_ACTION)
    {
        // 키 다운/업 모두 활동으로 간주
        if (ShouldFireForCurrentForeground())
        {
            const double Now = FPlatformTime::Seconds();
            if (Now - LastFireTime >= 0.5) // 디바운스 0.5s
            {
                LastFireTime = Now;
                if (ActivityDelegate.IsBound())
                {
                    ActivityDelegate.Execute();
                }
            }
        }
    }
    return CallNextHookEx(KeyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK FGlobalInputWatcher::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        // 이동/클릭/휠 전부 활동으로 간주
        if (ShouldFireForCurrentForeground())
        {
            const double Now = FPlatformTime::Seconds();
            if (Now - LastFireTime >= 0.5)
            {
                LastFireTime = Now;
                if (ActivityDelegate.IsBound())
                {
                    ActivityDelegate.Execute();
                }
            }
        }
    }
    return CallNextHookEx(MouseHook, nCode, wParam, lParam);
}

bool FGlobalInputWatcher::ShouldFireForCurrentForeground()
{
    const FString Proc = GetForegroundProcessNameLower();
    if (Proc.IsEmpty()) return false;
    return Whitelist.Contains(Proc);
}

FString FGlobalInputWatcher::GetForegroundProcessNameLower()
{
    HWND Hwnd = GetForegroundWindow();
    if (!Hwnd) return FString();

    DWORD Pid = 0;
    GetWindowThreadProcessId(Hwnd, &Pid);
    if (!Pid) return FString();

    HANDLE HProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, Pid);
    if (!HProc) return FString();

    wchar_t Buffer[MAX_PATH] = { 0 };
    DWORD Len = GetModuleBaseNameW(HProc, nullptr, Buffer, MAX_PATH);
    CloseHandle(HProc);

    if (Len == 0) return FString();

    FString Name = FString(Buffer);
    return Name.ToLower();
}
#else

TSharedPtr<FGlobalInputWatcher> FGlobalInputWatcher::Create(const TArray<FString>&, FOnExternalActivity)
{
    // Non-Windows: 미구현
    return nullptr;
}
FGlobalInputWatcher::~FGlobalInputWatcher() {}
bool FGlobalInputWatcher::Init(const TArray<FString>&, FOnExternalActivity) { return false; }
void FGlobalInputWatcher::Shutdown() {}

#endif