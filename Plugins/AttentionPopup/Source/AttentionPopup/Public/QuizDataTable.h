#pragma once

#include "CoreMinimal.h"
#include "QuizTypes.h"
#include "QuizPopSettings.h"

class UDataTable;

class FQuizDataTable
{
public:
    static UDataTable* LoadQuizDataTable(const EQuizMode InIsSimpleVersion);
    static const FQuizRow* GetRandomQuiz(UDataTable* Table);

    static void ResetLastMode(const EQuizMode InWasVersion);

    static void LoadAllDataTables();
    static void UnloadAllDataTables();

private:
    static UDataTable* CachedDataTable;
    static UDataTable* CreateFallbackDataTable();


    // ���������� ĳ���ߴ� ���.
    static EQuizMode LastQuizMode;
    static TArray<UDataTable*> LoadedDataTables;

};
