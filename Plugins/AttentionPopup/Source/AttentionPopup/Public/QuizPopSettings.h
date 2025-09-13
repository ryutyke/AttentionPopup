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
    // ���� ������ ���̺�
    UPROPERTY(EditAnywhere, Config, Category="Data")
    TSoftObjectPtr<UDataTable> QuizDataTable;

    // Todo : �ּ� �ִ� �ð� ���� ����
    // ���� �ֱ� (��)
    UPROPERTY(EditAnywhere, Config, Category="Timing", meta=(ClampMin="5", UIMin="60"))
    int32 IntervalSeconds = 10;

    UPROPERTY(EditAnywhere, Config, Category="Timing", meta=(ClampMin="1", UIMin="3600"))
    int32 MinQuestionIntervalSec = 8;

    UPROPERTY(EditAnywhere, Config, Category="Timing", meta=(ClampMin="1", UIMin="3600"))
    int32 MaxQuestionIntervalSec = 20;

    // Todo : ���� �� ��.
    // ������ ���� �� ��� �� �� ǥ��
    UPROPERTY(EditAnywhere, Config, Category="Timing")
    bool bShowImmediatelyOnStart = false;

    // 1���� ���� �ð�(��). 0 �����̸� ���� ����
    UPROPERTY(EditAnywhere, Config, Category="Quiz", meta=(ClampMin="0", UIMin="0"))
    int32 QuizTimeLimitSeconds = 5;

    // �ܺ� �� �Է¿� ������ ���μ��� ȭ��Ʈ����Ʈ (Windows ����)
    // ��: devenv.exe(Visual Studio), rider64.exe, code.exe(VS Code)
    UPROPERTY(EditAnywhere, Config, Category="External Activity (Windows)")
    TArray<FString> ActivityProcessWhitelist = { TEXT("devenv.exe"), TEXT("rider64.exe"), TEXT("code.exe") };

    static const UQuizPopSettings* GetSettings();

    UPROPERTY(EditAnywhere, Config, Category = "Quiz Mode")
    EQuizMode QuizMode = EQuizMode::Simple;

};