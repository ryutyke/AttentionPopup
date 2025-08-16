#pragma once

#include "CoreMinimal.h"

#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "HAL/PlatformTime.h"
#include "Engine/Texture2D.h"
#include "QuizTypes.h"
#include "Brushes/SlateImageBrush.h"

// ---- 퀴즈 창 위젯 -----------------------------------------------------------

class SQuizWindow : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SQuizWindow) {}
        SLATE_ARGUMENT(EQuizType, QuizType)
        SLATE_ARGUMENT(FText, Question)
        SLATE_ARGUMENT(UTexture2D*, QuestionImage)
        SLATE_ARGUMENT(TArray<FText>, Options)
        SLATE_ARGUMENT(FText, Answer)
        SLATE_ARGUMENT(float, TimeLimitSeconds)   // <= 0 이면 무제한
            SLATE_ARGUMENT(TFunction<void(bool)>, OnClosed) // 정답여부 콜백
    SLATE_END_ARGS()

        void Construct(const FArguments& InArgs);

        virtual ~SQuizWindow();

    static void ShowQuizModal(EQuizType InQuizType, const FText& QuestionText, UTexture2D* QuestionImage, const TArray<FText>& Options, FText Answer, float TimeLimitSeconds, TFunction<void(bool)> OnClosed);
    static void CloseResultWindowIfOpen();
private:
    TSharedRef<SWidget> BuildOptions();

    void HandleAnswer(int32 SelectedIndex); // 4지선다
    void HandleAnswerString(FText InputString); // 단답

    EActiveTimerReturnType OnActiveTick(double, float);

    inline EVisibility GetTimerVisibility() const
    {
        return (TimeLimitSec > 0.f) ? EVisibility::Visible : EVisibility::Collapsed;
    }

    FText GetCountdownText() const;

    TOptional<float> GetRemainingPercent() const;

    const FSlateBrush* GetImageBrush() const;

    EVisibility GetImageVisibility() const
    {
        return (QuestionImage != nullptr) ? EVisibility::Visible : EVisibility::Collapsed;
    }

private:
    EQuizType QuizType;
    FText Question;
    UTexture2D* QuestionImage = nullptr;
    TArray<FText> Options;
    //int32 CorrectIndex = 0;

    FText Answer;
    float TimeLimitSec = 0.f;
    double EndTime = 0.0;
    bool bAnswered = false;

    TSharedPtr<STextBlock>   CountdownText;
    TSharedPtr<SProgressBar> CountdownBar;

    mutable TSharedPtr<FSlateImageBrush> ImageBrush;
    TSharedPtr<FActiveTimerHandle> ActiveTimerHandle;
    TFunction<void(bool)> OnClosed;

    FReply OnSubmitClicked();

    TSharedPtr<SEditableTextBox> AnswerTextBox;

    static TWeakPtr<SWindow> ResultWindowWeak;
};

