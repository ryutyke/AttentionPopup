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
//    // 텍스트 문제(선택)
//    UPROPERTY(EditAnywhere, BlueprintReadOnly)
//    FText Question;
//
//    // 이미지 문제(선택)
//    UPROPERTY(EditAnywhere, BlueprintReadOnly)
//    UTexture2D* QuestionImage = nullptr;
//
//    // 보기 4개 (텍스트)
//    UPROPERTY(EditAnywhere, BlueprintReadOnly)
//    TArray<FText> Options;
//
//    // 정답 인덱스 (0~3)
//    UPROPERTY(EditAnywhere, BlueprintReadOnly)
//    int32 CorrectIndex = 0;
//};

// ===================== 재영 민주 ==================
USTRUCT(BlueprintType)
struct FQuizRow : public FTableRowBase
{
    GENERATED_BODY();

    FQuizRow()
        : QuizType(EQuizType::MultipleChoice) // 기본 초기화 추가
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