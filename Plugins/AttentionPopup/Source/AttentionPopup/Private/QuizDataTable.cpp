#include "QuizDataTable.h"
#include "Engine/Engine.h"

UDataTable* FQuizDataTable::CachedDataTable = nullptr;
EQuizMode FQuizDataTable::LastQuizMode = EQuizMode::Simple;
TArray<UDataTable*> FQuizDataTable::LoadedDataTables;

UDataTable* FQuizDataTable::LoadQuizDataTable(const EQuizMode InMode)
{
    if (InMode != LastQuizMode)
    {
        const int32 mode = static_cast<int32>(InMode);
        ensure(LoadedDataTables.IsValidIndex(mode));
        ensure(IsValid(LoadedDataTables[mode]));
        CachedDataTable = LoadedDataTables[mode];
        LastQuizMode = InMode;
    }
    else if (IsValid(CachedDataTable))
    {
        return CachedDataTable;
    }
    return CachedDataTable;
}

const FQuizRow* FQuizDataTable::GetRandomQuiz(UDataTable* Table)
{
    if (!Table || !IsValid(Table))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid DataTable passed to GetRandomQuiz"));
        return nullptr;
    }

    const TArray<FName> RowNames = Table->GetRowNames();
    if (RowNames.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("DataTable has no rows"));
        return nullptr;
    }

    const int32 RandomIndex = FMath::RandRange(0, RowNames.Num() - 1);
    const FName RandomRowName = RowNames[RandomIndex];

    const FQuizRow* SelectedQuiz = Table->FindRow<FQuizRow>(RandomRowName, TEXT("GetRandomQuiz"));

    if (!SelectedQuiz)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find row: %s"), *RandomRowName.ToString());
    }

    return SelectedQuiz;
}

void FQuizDataTable::ResetLastMode(const EQuizMode InMode)
{
    LastQuizMode = InMode;
    const int32 mode = static_cast<int32>(InMode);
    ensure(LoadedDataTables.IsValidIndex(mode));
    ensure(IsValid(LoadedDataTables[mode]));
    CachedDataTable = LoadedDataTables[mode];
}

void FQuizDataTable::LoadAllDataTables()
{
    FString DataTablePath = TEXT("/Script/Engine.DataTable'/AttentionPopup/Simple.Simple'");
    UDataTable* LoadedDataTable = Cast<UDataTable>(
        StaticLoadObject(UDataTable::StaticClass(), nullptr, *DataTablePath)
    );

    if (!LoadedDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable from path: %s"), *DataTablePath);
        LoadedDataTable = CreateFallbackDataTable();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully loaded DataTable from: %s"), *DataTablePath);
    }
    LoadedDataTables.Add(LoadedDataTable);

    DataTablePath = TEXT("/Script/Engine.DataTable'/AttentionPopup/Developer.Developer'");
    LoadedDataTable = Cast<UDataTable>(
        StaticLoadObject(UDataTable::StaticClass(), nullptr, *DataTablePath)
    );

    if (!LoadedDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable from path: %s"), *DataTablePath);
        LoadedDataTable = CreateFallbackDataTable();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully loaded DataTable from: %s"), *DataTablePath);
    }

    LoadedDataTables.Add(LoadedDataTable);

    DataTablePath = TEXT("/AntiAFKEditor/DataTables/DT_ImageSimpleQuizData.DT_ImageSimpleQuizData");
    LoadedDataTable = Cast<UDataTable>(
        StaticLoadObject(UDataTable::StaticClass(), nullptr, *DataTablePath)
    );

    if (!LoadedDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable from path: %s"), *DataTablePath);
        LoadedDataTable = CreateFallbackDataTable();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully loaded DataTable from: %s"), *DataTablePath);
    }

    LoadedDataTables.Add(LoadedDataTable);
    DataTablePath = TEXT("/AntiAFKEditor/DataTables/DT_ImageDeveloperQuizData.DT_ImageDeveloperQuizData");
    LoadedDataTable = Cast<UDataTable>(
        StaticLoadObject(UDataTable::StaticClass(), nullptr, *DataTablePath)
    );

    if (!LoadedDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable from path: %s"), *DataTablePath);
        LoadedDataTable = CreateFallbackDataTable();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully loaded DataTable from: %s"), *DataTablePath);
    }

    LoadedDataTables.Add(LoadedDataTable);
}

void FQuizDataTable::UnloadAllDataTables()
{
    for (UDataTable*& DataTable : LoadedDataTables)
    {
        DataTable = nullptr;
    }
    LoadedDataTables.Empty();
}

UDataTable* FQuizDataTable::CreateFallbackDataTable()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating fallback DataTable with hardcoded data"));

    UDataTable* DataTable = NewObject<UDataTable>();
    if (!DataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create fallback DataTable"));
        return nullptr;
    }

    DataTable->RowStruct = FQuizRow::StaticStruct();

    // 예시 퀴즈 데이터 추가
    FQuizRow Quiz1;
    Quiz1.QuizType = EQuizType::FillInBlank;
    Quiz1.Question = FText::FromString(TEXT("2 + 2 = ?"));
    Quiz1.Answer = FText::FromString(TEXT("4"));
    DataTable->AddRow(FName("Quiz001"), Quiz1);

    FQuizRow Quiz2;
    Quiz2.QuizType = EQuizType::MultipleChoice;
    Quiz2.Question = FText::FromString(TEXT("What is the capital of France?"));
    Quiz2.Answer = FText::FromString(TEXT("Paris"));
    Quiz2.Choices.Add(FText::FromString(TEXT("Berlin")));
    Quiz2.Choices.Add(FText::FromString(TEXT("London")));
    Quiz2.Choices.Add(FText::FromString(TEXT("Paris")));
    Quiz2.Choices.Add(FText::FromString(TEXT("Rome")));
    DataTable->AddRow(FName("Quiz002"), Quiz2);

    return DataTable;
}
