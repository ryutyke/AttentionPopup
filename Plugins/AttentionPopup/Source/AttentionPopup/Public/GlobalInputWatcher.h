#pragma once

#include "CoreMinimal.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include "Windows/HideWindowsPlatformTypes.h"

DECLARE_DELEGATE(FOnExternalActivity);

class FGlobalInputWatcher
{
public:
    // Whitelist: 소문자 exe 이름 배열 (예: "devenv.exe")
    static TSharedPtr<FGlobalInputWatcher> Create(const TArray<FString>& InProcessWhitelist, FOnExternalActivity InOnActivity);

    ~FGlobalInputWatcher();

private:
    FGlobalInputWatcher() = default;
    bool Init(const TArray<FString>& InProcessWhitelist, FOnExternalActivity InOnActivity);
    void Shutdown();

#if PLATFORM_WINDOWS
private:
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
    static bool ShouldFireForCurrentForeground();
    static FString GetForegroundProcessNameLower();

private:
    static inline HHOOK KeyboardHook = nullptr;
    static inline HHOOK MouseHook = nullptr;
    static inline FOnExternalActivity ActivityDelegate;
    static inline TSet<FString> Whitelist; // lower-case
    static inline double LastFireTime = 0.0;
#endif
};