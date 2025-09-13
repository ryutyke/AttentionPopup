// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FQuizPopManager;

class FAttentionPopupModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

    static FAttentionPopupModule& Get();
    FQuizPopManager* GetManager() const { return Manager.Get(); }

private:
    void RegisterMenus();

    // 토글 동작
    void ExecuteToggleFocusMode();
    bool IsFocusModeChecked() const;
    bool CanToggleFocusMode() const;

    void OnToolbarButtonClicked();

private:
    TUniquePtr<FQuizPopManager> Manager;
    TSharedPtr<class FUICommandList> PluginCommands;
    FDelegateHandle ToolMenusCBHandle;

    TSharedPtr<FSlateStyleSet> IconStyleSet;
};
