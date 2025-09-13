#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UObject/SoftObjectPtr.h"
#include "QuizPopSettings.generated.h"

UENUM(BlueprintType)
enum class EQuizMode : uint8
{
    Simple      UMETA(DisplayName = "Simple Mode"),
    Developer    UMETA(DisplayName = "Developer Mode"),
    Expert      UMETA(DisplayName = "Expert Mode"),
    Custom      UMETA(DisplayName = "Custom Mode")
};

UCLASS(Config=EditorPerProjectUserSettings, DefaultConfig, meta=(DisplayName="Quiz Pop"))
class UQuizPopSettings : public UDeveloperSettings
{
    GENERATED_BODY()
public:
    // 퀴즈 데이터 테이블
    UPROPERTY(EditAnywhere, Config, Category="Data")
    TSoftObjectPtr<UDataTable> QuizDataTable;

    // Todo : 최소 최대 시간 사이 랜덤
    // 출제 주기 (초)
    UPROPERTY(EditAnywhere, Config, Category="Timing", meta=(ClampMin="5", UIMin="60"))
    int32 IntervalSeconds = 10;

    UPROPERTY(EditAnywhere, Config, Category="Timing", meta=(ClampMin="1", UIMin="3600"))
    int32 MinQuestionIntervalSec = 8;

    UPROPERTY(EditAnywhere, Config, Category="Timing", meta=(ClampMin="1", UIMin="3600"))
    int32 MaxQuestionIntervalSec = 20;

    // Todo : 빼도 될 듯.
    // 에디터 시작 시 즉시 한 번 표시
    UPROPERTY(EditAnywhere, Config, Category="Timing")
    bool bShowImmediatelyOnStart = false;

    // 1문제 제한 시간(초). 0 이하이면 제한 없음
    UPROPERTY(EditAnywhere, Config, Category="Quiz", meta=(ClampMin="0", UIMin="0"))
    int32 QuizTimeLimitSeconds = 5;

    // 외부 앱 입력에 반응할 프로세스 화이트리스트 (Windows 전용)
    // 예: devenv.exe(Visual Studio), rider64.exe, code.exe(VS Code)
    UPROPERTY(EditAnywhere, Config, Category="External Activity (Windows)")
    TArray<FString> ActivityProcessWhitelist = { TEXT("devenv.exe"), TEXT("rider64.exe"), TEXT("code.exe") };

    static const UQuizPopSettings* GetSettings();

    UPROPERTY(EditAnywhere, Config, Category = "Quiz Mode")
    EQuizMode QuizMode = EQuizMode::Simple;

};