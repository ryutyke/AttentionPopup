#pragma once
#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "Styling/AppStyle.h"

class FQuizPopCommands : public TCommands<FQuizPopCommands>
{
public:
    FQuizPopCommands()
        : TCommands<FQuizPopCommands>(
            TEXT("QuizPop"),
            NSLOCTEXT("QuizPop", "QuizPopCommands", "Quiz Pop"),
            NAME_None,
            FAppStyle::GetAppStyleSetName()
        )
    {
    }

    virtual void RegisterCommands() override;

public:
    // 툴바 토글 버튼 커맨드
    TSharedPtr<FUICommandInfo> ToggleFocusMode;
};