#pragma once

#include "CoreMinimal.h"
#include "QuizTypes.h"

class UQuizPopSettings;

class FQuizPopManager
{
public:
    void Initialize();
    void Shutdown();

    // Focus Mode ����
    void StartFocusMode();  // ScheduleNext() ����
    void StopFocusMode();   // ���� �ߴ�
    bool IsFocusModeActive() const { return bFocusModeActive; }

    // (���ϸ� �ܺο����� �� �� �ֵ��� ����)
    void ScheduleNext(float InSeconds);

    int32 IntervalSeconds = 10;

private:
    bool PickRandomQuiz(FName& OutRowName, FQuizRow& OutRow);
    void ShowQuiz();
    void SaveState();
    void LoadState();

    void OnUserActivity();
    bool IsQuizWindowOpen() const;

private:
    TSet<FName> UsedRows;
    double NextTimeUtcSeconds = 0.0;
    TWeakObjectPtr<UDataTable> CachedTable;
    
    bool bShowImmediatelyOnStart = false;
    bool bFocusModeActive = false;
    FName LastUsedName;

    FString ConfigSection = TEXT("/Script/QuizPopEditor.QuizState");

private:
    TSharedPtr<class FGlobalInputWatcher> GlobalInputWatcher; // Windows ����

    // �Է� �������μ���
    TSharedPtr<class FQuizInputPreprocessor> InputPreprocessor;

    bool bIsShuttingDown = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ComPort = TEXT("COM6");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BaudRate = 115200;

    void OpenSerial(const FString& InComPort, int32 InBaud);
    void CloseSerial();
    void SetupDCB(int32 Baud, struct _DCB& Out);
    void SendLineAsync(const FString& Line);
    void* SerialHandle = nullptr;

    void TriggerReverseRotation();




};