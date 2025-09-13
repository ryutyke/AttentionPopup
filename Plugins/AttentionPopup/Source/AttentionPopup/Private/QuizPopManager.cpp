#include "QuizPopManager.h"
#include "QuizPopSettings.h"
#include "QuizInputPreprocessor.h"

#include "Engine/DataTable.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/PlatformTime.h"
#include "Containers/Ticker.h"
#include "Framework/Application/SlateApplication.h"
#include "Async/Async.h"
#include "GlobalInputWatcher.h"
#include "QuizDataTable.h"
#include "SQuizWindow.h"

#include <Windows.h>
#include <string>

static TSharedPtr<FTSTicker::FDelegateHandle> GActiveTickerHandle;

void FQuizPopManager::Initialize()
{
    const UQuizPopSettings* Settings = GetDefault<UQuizPopSettings>();
    IntervalSeconds = Settings->IntervalSeconds;
    bShowImmediatelyOnStart = Settings->bShowImmediatelyOnStart;

    UDataTable* Table = FQuizDataTable::LoadQuizDataTable(Settings->QuizMode);
    CachedTable = TWeakObjectPtr<UDataTable>(Table);

    /*if (Settings->QuizDataTable.IsValid())
    {
        CachedTable = Settings->QuizDataTable.Get();
    }
    else if (Settings->QuizDataTable.ToSoftObjectPath().IsValid())
    {
        CachedTable = Cast<UDataTable>(Settings->QuizDataTable.LoadSynchronous());
    }*/

    LoadState();

    // 집중 모드 자동 시작 여부는 정책에 맞춰 선택.
    // 여기서는 시작 시 자동으로 켜지지 않도록 false 유지.
    bFocusModeActive = false;

    // bShowImmediatelyOnStart가 true여도, Focus Mode가 꺼져 있으면 아무것도 하지 않음.
    // 툴바에서 StartFocusMode() 호출 시 첫 스케줄이 잡힙니다.

    OpenSerial(ComPort, BaudRate);
}

void FQuizPopManager::Shutdown()
{
    bIsShuttingDown = true;

    if (GActiveTickerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(*GActiveTickerHandle);
        GActiveTickerHandle.Reset();
    }

#if PLATFORM_WINDOWS
    GlobalInputWatcher.Reset();
#endif

    SaveState();
    CloseSerial();
}

void FQuizPopManager::StartFocusMode()
{
    if (bFocusModeActive) return;
    bFocusModeActive = true;

    const double Now = FPlatformTime::Seconds();
    NextTimeUtcSeconds = bShowImmediatelyOnStart ? Now : Now + IntervalSeconds;

    const float Delay = (float)FMath::Max(0.0, NextTimeUtcSeconds - Now);

    const UQuizPopSettings* Settings = GetDefault<UQuizPopSettings>();

    // 입력 프리프로세서 등록 (에디터 전역 입력 감지)
    {
        InputPreprocessor = MakeShared<FQuizInputPreprocessor>([this]()
            {
                OnUserActivity();
            });
        FSlateApplication::Get().RegisterInputPreProcessor(InputPreprocessor, 0);
    }

#if PLATFORM_WINDOWS
    {
        TArray<FString> Lower;
        for (const FString& N : Settings->ActivityProcessWhitelist)
        {
            Lower.Add(N.ToLower());
        }

        GlobalInputWatcher = FGlobalInputWatcher::Create(
            Lower,
            FOnExternalActivity::CreateLambda([this]()
                {
                    // 퀴즈 창이 열려 있으면 무시(선택)
                    if (IsQuizWindowOpen()) return;
                    const double Now = FPlatformTime::Seconds();
                    NextTimeUtcSeconds = Now + IntervalSeconds;
                    ScheduleNext(IntervalSeconds);
                    SaveState();
                })
        );
    }
#endif

    ScheduleNext(Delay);
    SaveState();
}

void FQuizPopManager::StopFocusMode()
{
    if (!bFocusModeActive) return;
    bFocusModeActive = false;

    if (GActiveTickerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(*GActiveTickerHandle);
        GActiveTickerHandle.Reset();
    }

    // 입력 프리프로세서 해제
    if (InputPreprocessor.IsValid())
    {
        FSlateApplication::Get().UnregisterInputPreProcessor(InputPreprocessor);
        InputPreprocessor.Reset();
    }

    GlobalInputWatcher.Reset();

    SaveState();
}

void FQuizPopManager::ScheduleNext(float InSeconds)
{
    const UQuizPopSettings* Settings = GetDefault<UQuizPopSettings>();
    IntervalSeconds = FMath::RandRange(Settings->MinQuestionIntervalSec, Settings->MaxQuestionIntervalSec);

    if (!bFocusModeActive) return; // Focus Mode OFF면 스케줄 안 함

    if (GActiveTickerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(*GActiveTickerHandle);
        GActiveTickerHandle.Reset();
    }

    GActiveTickerHandle = MakeShared<FTSTicker::FDelegateHandle>(
        FTSTicker::GetCoreTicker().AddTicker(
            FTickerDelegate::CreateLambda([this, Start = FPlatformTime::Seconds(), Delay = InSeconds](float) mutable
                {
                    if (!bFocusModeActive) return false;

                    const double Now = FPlatformTime::Seconds();
                    if (Now - Start >= Delay)
                    {
                        SQuizWindow::CloseResultWindowIfOpen();
                        ShowQuiz();
                        NextTimeUtcSeconds = FPlatformTime::Seconds() + IntervalSeconds;

                        // 다음 스케줄
                        ScheduleNext(IntervalSeconds);
                        SaveState();
                        return false;
                    }
                    return true;
                }),
            0.25f
        )
    );
}

bool FQuizPopManager::PickRandomQuiz(FName& OutRowName, FQuizRow& OutRow)
{
    //if (!CachedTable.IsValid()) return false;

    const UQuizPopSettings* Settings = GetDefault<UQuizPopSettings>();
    const UDataTable* Table = FQuizDataTable::LoadQuizDataTable(Settings->QuizMode);

    TArray<FName> Names;
    //Names = CachedTable->GetRowNames();
    Names = Table->GetRowNames();
    Names.RemoveAll([this](const FName& N) { return UsedRows.Contains(N); });

    if (Names.Num() == 0)
    {
        UsedRows.Empty();
        Names = Table->GetRowNames();
        //Names = CachedTable->GetRowNames();
    }
    if (Names.Num() == 0) return false;

    const int32 Index = FMath::RandRange(0, Names.Num() - 1);
    OutRowName = Names[Index];

    static const FString Ctx = TEXT("QuizPick");
    //if (FQuizRow* Row = CachedTable->FindRow<FQuizRow>(OutRowName, Ctx))
    if (FQuizRow* Row = Table->FindRow<FQuizRow>(OutRowName, Ctx))
    {
        OutRow = *Row;
        return true;
    }
    return false;
}

void FQuizPopManager::ShowQuiz()
{
    FName RowName;
    FQuizRow Row;
    if (!PickRandomQuiz(RowName, Row))
    {
        return;
    }

    UsedRows.Add(RowName);

    int ChoicesCnt = Row.Choices.Num();

    TArray<FText> OptionsArr;
    OptionsArr.Reserve(ChoicesCnt);
    for (int i = 0; i < ChoicesCnt; i++) OptionsArr.Add(Row.Choices[i]);

    AsyncTask(ENamedThreads::GameThread, [Row, OptionsArr]()
        {
            // Todo
            SQuizWindow::ShowQuizModal(Row.QuizType, Row.Question, Row.Image, OptionsArr, Row.Answer, Row.TimeLimit, [](bool) {});
        });
}

void FQuizPopManager::SaveState()
{
    // Focus Mode 상태와 NextTime 저장
    GConfig->SetBool(*ConfigSection, TEXT("FocusModeActive"), bFocusModeActive, GEditorPerProjectIni);
    GConfig->SetDouble(*ConfigSection, TEXT("NextTime"), NextTimeUtcSeconds, GEditorPerProjectIni);

    // UsedRows 저장
    FString UsedStr;
    for (const FName& N : UsedRows)
    {
        if (!UsedStr.IsEmpty()) UsedStr += TEXT(",");
        UsedStr += N.ToString();
    }
    GConfig->SetString(*ConfigSection, TEXT("UsedRows"), *UsedStr, GEditorPerProjectIni);
    GConfig->Flush(false, GEditorPerProjectIni);
}

void FQuizPopManager::LoadState()
{
    GConfig->GetBool(*ConfigSection, TEXT("FocusModeActive"), bFocusModeActive, GEditorPerProjectIni);
    GConfig->GetDouble(*ConfigSection, TEXT("NextTime"), NextTimeUtcSeconds, GEditorPerProjectIni);

    FString UsedStr;
    if (GConfig->GetString(*ConfigSection, TEXT("UsedRows"), UsedStr, GEditorPerProjectIni))
    {
        TArray<FString> Tokens;
        UsedStr.ParseIntoArray(Tokens, TEXT(","), true);
        for (const FString& T : Tokens)
        {
            if (!T.IsEmpty())
            {
                UsedRows.Add(FName(*T));
            }
        }
    }

    // 복구 시점에 FocusMode가 켜져 있었다면 다음 스케줄을 복원할 수도 있음.
    // 편의상, 여기서는 모듈 초기화 직후에는 '정지' 상태로 두고,
    // 툴바에서 다시 Start를 누르게 하는 정책을 추천.
}

void FQuizPopManager::OnUserActivity()
{
    // 퀴즈 창이 떠 있는 동안엔 리셋하지 않음(선택적)
    if (IsQuizWindowOpen())
    {
        return;
    }

    const double Now = FPlatformTime::Seconds();
    NextTimeUtcSeconds = Now + IntervalSeconds;

    // 즉시 재스케줄
    ScheduleNext(IntervalSeconds);
    SaveState();
}

bool FQuizPopManager::IsQuizWindowOpen() const
{
    if (bIsShuttingDown)
    {
        return false; // 종료 중이면 그냥 false 리턴
    }

    if (!FSlateApplication::IsInitialized())
    {
        return false;
    }

    // 타 플러그인/창과 충돌 방지를 위해 타이틀 비교를 간단하게 사용
    // (SQuizWindow에서 Title을 "깜짝 퀴즈"로 설정했었죠)
    const TArray<TSharedRef<SWindow>> Windows = FSlateApplication::Get().GetInteractiveTopLevelWindows();
    for (const TSharedRef<SWindow>& W : Windows)
    {
        if (W->GetTitle().ToString().Contains(TEXT("깜짝 퀴즈")))
        {
            return true;
        }
    }
    return false;
}


void FQuizPopManager::OpenSerial(const FString& InComPort, int32 InBaud)
{
    UE_LOG(LogTemp, Warning, TEXT("Opening serial: %s @ %d"), *InComPort, InBaud);

    // 방법 1: 기본 접근
    FString WinPath = FString::Printf(TEXT("\\\\.\\%s"), *InComPort);
    HANDLE H = CreateFileW(*WinPath, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

    if (H == INVALID_HANDLE_VALUE)
    {
        UE_LOG(LogTemp, Error, TEXT("Method 1 failed. Trying alternative approaches..."));

        // 방법 2: FILE_FLAG_OVERLAPPED 사용
        H = CreateFileW(*WinPath, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
            OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);

        if (H == INVALID_HANDLE_VALUE)
        {
            // 방법 3: 공유 모드로 시도
            H = CreateFileW(*WinPath, GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

            if (H == INVALID_HANDLE_VALUE)
            {
                const DWORD Err = GetLastError();
                UE_LOG(LogTemp, Error, TEXT("All methods failed for %s (Final Error %lu): "),
                    *InComPort, Err);

                // 구체적인 해결 방법 제시
                UE_LOG(LogTemp, Error, TEXT("=== SOLUTIONS TO TRY ==="));
                UE_LOG(LogTemp, Error, TEXT("1. Close ALL programs that might use COM6"));
                UE_LOG(LogTemp, Error, TEXT("2. Unplug Arduino, wait 5 seconds, plug back in"));
                UE_LOG(LogTemp, Error, TEXT("3. Run Unreal Editor as Administrator"));
                UE_LOG(LogTemp, Error, TEXT("4. Try different USB port"));
                UE_LOG(LogTemp, Error, TEXT("5. Restart Windows (driver reset)"));

                SerialHandle = nullptr;
                return;
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("✓ CreateFileW SUCCESS! Handle: %p"), H);

    // 나머지 설정은 동일...
    DCB Dcb{};
    Dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(H, &Dcb))
    {
        UE_LOG(LogTemp, Error, TEXT("GetCommState failed"));
        CloseHandle(H);
        SerialHandle = nullptr;
        return;
    }

    SetupDCB(InBaud, Dcb);
    if (!SetCommState(H, &Dcb))
    {
        UE_LOG(LogTemp, Error, TEXT("SetCommState failed"));
        CloseHandle(H);
        SerialHandle = nullptr;
        return;
    }

    COMMTIMEOUTS T{ 0,0,0,0,0 };
    SetCommTimeouts(H, &T);

    SerialHandle = H;
    UE_LOG(LogTemp, Log, TEXT("✓ Serial connection established: %s @ %d"), *InComPort, InBaud);

    // Arduino 초기화 대기
    FPlatformProcess::Sleep(2.0f);
}

void FQuizPopManager::CloseSerial()
{
    if (SerialHandle)
    {
        CloseHandle((HANDLE)SerialHandle);
        SerialHandle = nullptr;
        UE_LOG(LogTemp, Log, TEXT("Serial closed"));
    }
}

#define TRUE 1
#define TRUE true

void FQuizPopManager::SetupDCB(int32 Baud, DCB& Out)
{
    Out.BaudRate = Baud;
    Out.ByteSize = 8;
    Out.Parity = NOPARITY;
    Out.StopBits = ONESTOPBIT;
    Out.fBinary = TRUE;
    Out.fDtrControl = DTR_CONTROL_ENABLE;
    Out.fRtsControl = RTS_CONTROL_ENABLE;
}

void FQuizPopManager::SendLineAsync(const FString& Line)
{
    if (!SerialHandle) return;
    FString Copy = Line; // 캡쳐 안전
    HANDLE H = (HANDLE)SerialHandle;

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [H, Copy]()
        {
            // Copy가 FString* 라고 가정 (아니면 타입에 맞게 수정)
            FTCHARToUTF8 Conv(*Copy);      // 변환 객체는 스코프 끝까지 살아 있어야 함
            const ANSICHAR* Data = Conv.Get();
            const int32 NumBytes = Conv.Length();  // 바이트 수(널 문자 제외)

            DWORD Written = 0;
            WriteFile(H, Data, static_cast<DWORD>(NumBytes), &Written, nullptr);
            FlushFileBuffers(H);
        });
}
