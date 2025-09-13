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
    // ���� ��� ��ư Ŀ�ǵ�
    TSharedPtr<FUICommandInfo> ToggleFocusMode;
};