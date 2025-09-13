#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "QuizTypes.generated.h"

UENUM(BlueprintType)
enum class EQuizType : uint8
{
    MultipleChoice UMETA(DisplayName = "Multiple Choice"),
    FillInBlank UMETA(DisplayName = "Fill in the Blank"),
};

//USTRUCT(BlueprintType)
//struct FQuizRow : public FTableRowBase
//{
//    GENERATED_BODY()
//
//    // �ؽ�Ʈ ����(����)
//    UPROPERTY(EditAnywhere, BlueprintReadOnly)
//    FText Question;
//
//    // �̹��� ����(����)
//    UPROPERTY(EditAnywhere, BlueprintReadOnly)
//    UTexture2D* QuestionImage = nullptr;
//
//    // ���� 4�� (�ؽ�Ʈ)
//    UPROPERTY(EditAnywhere, BlueprintReadOnly)
//    TArray<FText> Options;
//
//    // ���� �ε��� (0~3)
//    UPROPERTY(EditAnywhere, BlueprintReadOnly)
//    int32 CorrectIndex = 0;
//};

// ===================== �翵 ���� ==================
USTRUCT(BlueprintType)
struct FQuizRow : public FTableRowBase
{
    GENERATED_BODY();

    FQuizRow()
        : QuizType(EQuizType::MultipleChoice) // �⺻ �ʱ�ȭ �߰�
    {
    }

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EQuizType QuizType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Question;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UTexture2D* Image;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FText> Choices;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Answer;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float TimeLimit;
};