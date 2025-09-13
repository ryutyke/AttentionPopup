#include "SQuizWindow.h"
#include "QuizTypes.h"
#include "AttentionPopup.h"
#include "QuizPopManager.h"

TWeakPtr<SWindow> SQuizWindow::ResultWindowWeak;

void SQuizWindow::Construct(const FArguments& InArgs)
{
    QuizType = InArgs._QuizType;
    Question = InArgs._Question;
    QuestionImage = InArgs._QuestionImage;
    Options = InArgs._Options;
    Answer = InArgs._Answer;
    TimeLimitSec = InArgs._TimeLimitSeconds;
    OnClosed = InArgs._OnClosed;

    if (TimeLimitSec > 0.f)
    {
        EndTime = FPlatformTime::Seconds() + TimeLimitSec;
        ActiveTimerHandle = RegisterActiveTimer(0.05f, FWidgetActiveTimerDelegate::CreateSP(this, &SQuizWindow::OnActiveTick));
    }

    
    switch (QuizType)
    {
    case EQuizType::MultipleChoice :
        ChildSlot
        [
            SNew(SBorder)
				.Padding(12)
                [
                    SNew(SVerticalBox)

                        //// 질문 (이미지 우선, 없으면 텍스트)
                        //+ SVerticalBox::Slot()
                        //.AutoHeight()
                        //.Padding(0, 0, 0, 10)
                        //[
                        //    QuestionImage
                        //        ? StaticCastSharedRef<SWidget>(
                        //            SNew(SBox)
                        //            .WidthOverride(400)
                        //            .HeightOverride(200)
                        //            [
                        //                SNew(SImage)
                        //                    .Image(this, &SQuizWindow::GetImageBrush)
                        //            ])

                        //        : StaticCastSharedRef<SWidget>(
                        //            SNew(STextBlock)
                        //            .Text(Question)
                        //            .AutoWrapText(true)
                        //        )
                        //]

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0, 0, 0, 10)
                        [
                            SNew(SBox)
                                .WidthOverride(400)
                                .HeightOverride(200)
                                .Visibility(this, &SQuizWindow::GetImageVisibility) // 사진 없으면 감춤
                                [
                                    SNew(SImage)
                                        .Image(this, &SQuizWindow::GetImageBrush)        // QuestionImage 없으면 nullptr 반환 OK
                                ]
                        ]

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0, 0, 0, 10)
                        [
                            StaticCastSharedRef<SWidget>(
                                SNew(STextBlock)
                                .Text(Question)
                                .AutoWrapText(true)
                                )
                        ]

                        // 제한 시간 표시
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0, 0, 0, 10)
                        [
                            SNew(SVerticalBox)
                                .Visibility(this, &SQuizWindow::GetTimerVisibility)
                                + SVerticalBox::Slot().AutoHeight()
                                [
                                    SAssignNew(CountdownText, STextBlock)
                                        .Text(this, &SQuizWindow::GetCountdownText)
                                ]
                                + SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 0)
                                [
                                    SAssignNew(CountdownBar, SProgressBar)
                                        .Percent(this, &SQuizWindow::GetRemainingPercent)
                                ]
                        ]

                        // 보기 버튼들
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            BuildOptions()
                        ]
                ]
        ];
        break;

    case EQuizType::FillInBlank :
        ChildSlot
            [
                SNew(SBorder)
                    .BorderImage(FAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
                    .Padding(10)
                    [
                        SNew(SVerticalBox)
                            
                            // 제한 시간 표시
                            + SVerticalBox::Slot()
                            .AutoHeight()
                            .Padding(0, 0, 0, 10)
                            [
                                SNew(SVerticalBox)
                                    .Visibility(this, &SQuizWindow::GetTimerVisibility)
                                    + SVerticalBox::Slot().AutoHeight()
                                    [
                                        SAssignNew(CountdownText, STextBlock)
                                            .Text(this, &SQuizWindow::GetCountdownText)
                                    ]
                                    + SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 0)
                                    [
                                        SAssignNew(CountdownBar, SProgressBar)
                                            .Percent(this, &SQuizWindow::GetRemainingPercent)
                                    ]
                            ]

                            + SVerticalBox::Slot()
                            .AutoHeight()
                            .Padding(0, 0, 0, 10)
                            [
                                StaticCastSharedRef<SWidget>(
                                    SNew(STextBlock)
                                    .Text(Question)
                                    .AutoWrapText(true)
                                )
                            ]

                            + SVerticalBox::Slot()
                            .AutoHeight()
                            .Padding(0, 0, 0, 10)
                            [
                                SNew(SBox)
                                    .WidthOverride(400)
                                    .HeightOverride(200)
                                    .Visibility(this, &SQuizWindow::GetImageVisibility) // 사진 없으면 감춤
                                    [
                                        SNew(SImage)
                                            .Image(this, &SQuizWindow::GetImageBrush)        // QuestionImage 없으면 nullptr 반환 OK
                                    ]
                            ]

                            // 답변 입력창
                            + SVerticalBox::Slot()
                            .AutoHeight()
                            .Padding(5)
                            [
                                SAssignNew(AnswerTextBox, SEditableTextBox)
                                    .HintText(NSLOCTEXT("ShortAnswer", "Hint", "여기에 답을 입력하세요"))
                            ]

                            // 제출 버튼
                            + SVerticalBox::Slot()
                            .AutoHeight()
                            .Padding(5)
                            .HAlign(HAlign_Right)
                            [
                                SNew(SButton)
                                    .Text(NSLOCTEXT("ShortAnswer", "Submit", "제출"))
                                    .OnClicked(this, &SQuizWindow::OnSubmitClicked)
                            ]
                    ]
            ];
        break;

    default:
        break;
    }


    
}

SQuizWindow::~SQuizWindow()
{
    if (ActiveTimerHandle.IsValid())
    {
        UnRegisterActiveTimer(ActiveTimerHandle.ToSharedRef());
    }
    ImageBrush.Reset();
}

TSharedRef<SWidget> SQuizWindow::BuildOptions()
{
    TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
    for (int32 i = 0; i < Options.Num(); ++i)
    {
        Box->AddSlot().AutoHeight().Padding(0, 4)
            [
                SNew(SButton)
                    .Text(Options[i])
                    .OnClicked_Lambda([this, i]()
                        {
                            HandleAnswer(i);
                            return FReply::Handled();
                        })
            ];
    }
    return Box;
}

void SQuizWindow::HandleAnswer(int32 SelectedIndex)
{
    FQuizPopManager* Manager = FAttentionPopupModule::Get().GetManager();
    Manager->ScheduleNext(Manager->IntervalSeconds);

    if (bAnswered) return;
    bAnswered = true;

    if (ActiveTimerHandle.IsValid())
    {
        UnRegisterActiveTimer(ActiveTimerHandle.ToSharedRef());
    }

    FString AnswerString = Answer.ToString();              // FText → FString
    int32 AnswerInt = FCString::Atoi(*AnswerString);       // FString → int32
    int32 CorrectIndex = AnswerInt;

    const bool bCorrect = (SelectedIndex == CorrectIndex);
    const FText Feedback = bCorrect
        ? FText::FromString(FString::Printf(TEXT("정답입니다!")))
        : FText::FromString(FString::Printf(TEXT("아쉬워요! 정답은 %d번"), CorrectIndex + 1));

    if (bCorrect == false)
    {
        USoundBase* QuizSound = LoadObject<USoundBase>(GEditor,
            TEXT("/AttentionPopup/QuizNotification2.QuizNotification2"));

        if (QuizSound)
        {
            GEditor->PlayEditorSound(QuizSound);
        }
        else
        {
            // fallback - 기본 시스템 사운드
            FSlateApplication::Get().PlaySound(FAppStyle::GetSound("Notification"));
        }
    }


    TSharedRef<SWindow> WindowRef = FSlateApplication::Get().AddWindow(
        SNew(SWindow)
        .ClientSize(FVector2D(300, 120))
        .Title(FText::FromString(TEXT("퀴즈 결과")))
        [
            SNew(SBorder).Padding(12)
                [
                    SNew(STextBlock)
                        .Text(Feedback)
                        .Justification(ETextJustify::Center)
                        .AutoWrapText(true)
                ]
        ]
    );

    ResultWindowWeak = WindowRef;

    if (OnClosed) OnClosed(bCorrect);

    if (TSharedPtr<SWindow> Parent = FSlateApplication::Get().FindWidgetWindow(AsShared()))
    {
        Parent->RequestDestroyWindow();
    }
}

void SQuizWindow::HandleAnswerString(FText InputString)
{
    FQuizPopManager* Manager = FAttentionPopupModule::Get().GetManager();
    Manager->ScheduleNext(Manager->IntervalSeconds);

    if (bAnswered) return;
    bAnswered = true;

    if (ActiveTimerHandle.IsValid())
    {
        UnRegisterActiveTimer(ActiveTimerHandle.ToSharedRef());
    }

    FString AnswerString = Answer.ToString();              // FText → FString

    const bool bCorrect = (InputString.ToString().Equals(AnswerString, ESearchCase::IgnoreCase));
    FText Feedback = bCorrect
        ? FText::FromString(FString::Printf(TEXT("정답입니다!")))
        : FText::FromString(FString::Printf(TEXT("아쉬워요! 정답은 %s"), *AnswerString));

    if (InputString.IsEmpty())
    {
        Feedback = FText::FromString(FString::Printf(TEXT("아쉬워요!")));
    }

    if (bCorrect == false)
    {
        USoundBase* QuizSound = LoadObject<USoundBase>(GEditor,
            TEXT("/AttentionPopup/QuizNotification2.QuizNotification2"));
        
        if (QuizSound)
        {
            GEditor->PlayEditorSound(QuizSound);
        }
        else
        {
            // fallback - 기본 시스템 사운드
            FSlateApplication::Get().PlaySound(FAppStyle::GetSound("Notification"));
        }
    }


    TSharedRef<SWindow> WindowRef = FSlateApplication::Get().AddWindow(
        SNew(SWindow)
        .ClientSize(FVector2D(300, 120))
        .Title(FText::FromString(TEXT("퀴즈 결과")))
        [
            SNew(SBorder).Padding(12)
                [
                    SNew(STextBlock)
                        .Text(Feedback)
                        .Justification(ETextJustify::Center)
                        .AutoWrapText(true)
                ]
        ]
    );

    ResultWindowWeak = WindowRef;

    if (OnClosed) OnClosed(bCorrect);

    if (TSharedPtr<SWindow> Parent = FSlateApplication::Get().FindWidgetWindow(AsShared()))
    {
        Parent->RequestDestroyWindow();
    }
}

EActiveTimerReturnType SQuizWindow::OnActiveTick(double, float)
{
    if (TimeLimitSec <= 0.f) return EActiveTimerReturnType::Stop;

    const double Now = FPlatformTime::Seconds();
    if (CountdownText.IsValid())
    {
        CountdownText->SetText(GetCountdownText());
    }
    if (CountdownBar.IsValid())
    {
        CountdownBar->SetPercent(GetRemainingPercent());
    }

    if (Now >= EndTime)
    {
        USoundBase* QuizSound = LoadObject<USoundBase>(GEditor,
            TEXT("/AttentionPopup/QuizNotification2.QuizNotification2"));

        if (QuizSound)
        {
            GEditor->PlayEditorSound(QuizSound);
        }
        else
        {
            // fallback - 기본 시스템 사운드
            FSlateApplication::Get().PlaySound(FAppStyle::GetSound("Notification"));
        }

        HandleAnswerString(FText::GetEmpty());
        return EActiveTimerReturnType::Stop;
    }
    return EActiveTimerReturnType::Continue;
}

FText SQuizWindow::GetCountdownText() const
{
    if (TimeLimitSec <= 0.f) return FText::GetEmpty();
    const double Now = FPlatformTime::Seconds();
    const double Remain = FMath::Max(0.0, EndTime - Now);
    const int32 RemainInt = (int32)FMath::CeilToDouble(Remain);
    return FText::FromString(FString::Printf(TEXT("남은 시간: %d초"), RemainInt));
}

TOptional<float> SQuizWindow::GetRemainingPercent() const
{
    if (TimeLimitSec <= 0.f) return TOptional<float>();
    const double Now = FPlatformTime::Seconds();
    const double Remain = FMath::Clamp(EndTime - Now, 0.0, (double)TimeLimitSec);
    const float Percent = (float)(Remain / TimeLimitSec);
    return Percent;
}

const FSlateBrush* SQuizWindow::GetImageBrush() const
{
    if (!QuestionImage) return nullptr;
    if (!ImageBrush.IsValid())
    {
        // const_cast: SlateImageBrush 생성 위해 캐시
        TSharedRef<FSlateImageBrush> NewBrush = MakeShared<FSlateImageBrush>(QuestionImage, FVector2D(QuestionImage->GetSizeX(), QuestionImage->GetSizeY()));
        ImageBrush = NewBrush;
    }
    return ImageBrush.Get();
}

FReply SQuizWindow::OnSubmitClicked()
{
    // 채점

    if (AnswerTextBox.IsValid())
    {
        FText AnswerInput = AnswerTextBox->GetText();
        HandleAnswerString(AnswerInput);
    }
    return FReply::Handled();
}

void SQuizWindow::ShowQuizModal(EQuizType InQuizType, const FText& QuestionText, UTexture2D* QuestionImage, const TArray<FText>& Options, FText Answer, float TimeLimitSeconds, TFunction<void(bool)> OnClosed)
{
    TSharedRef<SWindow> Window = SNew(SWindow)
        .Title(FText::FromString(TEXT("깜짝 퀴즈")))
        .SupportsMaximize(false)
        .SupportsMinimize(false)
        .SizingRule(ESizingRule::Autosized);

    Window->SetOnWindowClosed(FOnWindowClosed::CreateLambda([OnClosed](const TSharedRef<SWindow>&)
        {
            if (OnClosed) OnClosed(false);
        }));

    Window->SetContent(
        SNew(SQuizWindow)
        .QuizType(InQuizType)
        .Question(QuestionText)
        .QuestionImage(QuestionImage)
        .Options(Options)
        .Answer(Answer)
        .TimeLimitSeconds(TimeLimitSeconds)
        .OnClosed([WindowPtr = TWeakPtr<SWindow>(Window), OnClosed](bool bCorrect)
            {
                if (TSharedPtr<SWindow> W = WindowPtr.Pin())
                {
                    W->RequestDestroyWindow();
                }
                if (OnClosed) OnClosed(bCorrect);
            })
    );

    FSlateApplication::Get().AddModalWindow(Window, nullptr);
}

void SQuizWindow::CloseResultWindowIfOpen()
{
    if (TSharedPtr<SWindow> Window = ResultWindowWeak.Pin())
    {
        // 켜져 있으면 닫기
        if (Window->IsVisible())
        {
            // 둘 중 하나 사용 가능
            // 1) 창 자체에 요청
            Window->RequestDestroyWindow();

            // 2) 애플리케이션을 통해 요청
            // FSlateApplication::Get().RequestDestroyWindow(Window.ToSharedRef());
        }

        // 선택: 즉시 포인터 정리 (닫히는 시점은 틱에서 처리될 수 있음)
        ResultWindowWeak.Reset();
    }
}


