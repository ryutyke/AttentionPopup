// Copyright Epic Games, Inc. All Rights Reserved.

#include "AttentionPopup.h"
#include "QuizPopManager.h"
#include "QuizPopCommands.h"

#include "LevelEditor.h"
#include "ToolMenus.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/SWindow.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/AppStyle.h"
#include "QuizPopSettings.h"
#include "Styling/SlateStyleRegistry.h"
#include "QuizDataTable.h"

#define LOCTEXT_NAMESPACE "FAttentionPopupModule"

void FAttentionPopupModule::StartupModule()
{
    // 커맨드 등록
    FQuizPopCommands::Register();
    PluginCommands = MakeShared<FUICommandList>();
    PluginCommands->MapAction(
        FQuizPopCommands::Get().ToggleFocusMode,
        FExecuteAction::CreateRaw(this, &FAttentionPopupModule::ExecuteToggleFocusMode),
        FCanExecuteAction::CreateRaw(this, &FAttentionPopupModule::CanToggleFocusMode),
        FIsActionChecked::CreateRaw(this, &FAttentionPopupModule::IsFocusModeChecked)
    );

    // 메뉴/툴바 등록
    if (!UToolMenus::IsToolMenuUIEnabled())
    {
        ToolMenusCBHandle = UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FAttentionPopupModule::RegisterMenus));
    }
    else
    {
        RegisterMenus();
    }

    FQuizDataTable::LoadAllDataTables();
    const UQuizPopSettings* Settings = GetDefault<UQuizPopSettings>();
    FQuizDataTable::ResetLastMode(Settings->QuizMode);

    // 매니저
    Manager = MakeUnique<FQuizPopManager>();
    Manager->Initialize();
}

void FAttentionPopupModule::ShutdownModule()
{
    if (UToolMenus::IsToolMenuUIEnabled())
    {
        UToolMenus::UnRegisterStartupCallback((void*)this);
        if (ToolMenusCBHandle.IsValid())
        {
            UToolMenus::UnregisterOwner(this);
            ToolMenusCBHandle.Reset();
        }
    }

    FQuizPopCommands::Unregister();

    if (Manager)
    {
        Manager->Shutdown();
        Manager.Reset();
    }

    if (IconStyleSet.IsValid())
    {
        FSlateStyleRegistry::UnRegisterSlateStyle(*IconStyleSet);
        IconStyleSet.Reset();
    }

    FQuizDataTable::UnloadAllDataTables();
}

FAttentionPopupModule& FAttentionPopupModule::Get()
{
    return FModuleManager::LoadModuleChecked<FAttentionPopupModule>("AttentionPopup");
}

void FAttentionPopupModule::RegisterMenus()
{
    FToolMenuOwnerScoped OwnerScoped(this);

    if (UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar"))
    {
        FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("AttentionPopupSection");
        
        FString IconPath = TEXT("/AttentionPopup/Icon");
        UTexture2D* IconTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *IconPath));

        FSlateIcon CustomIcon;

        IconStyleSet = MakeShareable(new FSlateStyleSet("AttentionPopupIcons"));
        FSlateStyleRegistry::RegisterSlateStyle(*IconStyleSet);

        if (IconTexture && IconStyleSet.IsValid())
        {
            // FSlateBrush를 스타일셋에 추가
            IconStyleSet->Set("CustomFocusIcon",
                new FSlateImageBrush(IconTexture, FVector2D(512.0f, 512.0f)));

            // 커스텀 스타일셋의 아이콘 사용
            CustomIcon = FSlateIcon("AttentionPopupIcons", "CustomFocusIcon");
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to load icon texture: %s"), *IconPath);
            CustomIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Plus");  // fallback
        }

        FToolMenuEntry Entry = FToolMenuEntry::InitToolBarButton(
            FQuizPopCommands::Get().ToggleFocusMode,
            LOCTEXT("FocusMode_Label", "Focus Mode"),
            LOCTEXT("FocusMode_Tooltip", "Start/Stop Focus Mode (schedule surprise quizzes)."),
            CustomIcon
        );

    
        // 커맨드 리스트 연결
        Entry.SetCommandList(PluginCommands);
        Section.AddEntry(Entry);
    }
}

void FAttentionPopupModule::ExecuteToggleFocusMode()
{
    if (!Manager) return;

    if (Manager->IsFocusModeActive())
    {
        Manager->StopFocusMode();
    }
    else
    {
        Manager->StartFocusMode();
    }

    // 툴바 상태 즉시 갱신
    UToolMenus::Get()->RefreshAllWidgets();
    //UToolMenus::RefreshAllWidgets();
}

bool FAttentionPopupModule::IsFocusModeChecked() const
{
    return Manager && Manager->IsFocusModeActive();
}

bool FAttentionPopupModule::CanToggleFocusMode() const
{
    // 필요 시 조건 추가(예: DataTable 미지정 시 비활성화 등)
    return true;
}

void FAttentionPopupModule::OnToolbarButtonClicked()
{
    // 토글 다이얼로그 띄우기
    TSharedRef<SWindow> Dialog = SNew(SWindow)
        .Title(LOCTEXT("FocusModeDialogTitle", "Focus Mode"))
        .ClientSize(FVector2D(360, 160))
        .SupportsMaximize(false)
        .SupportsMinimize(false);

    FQuizPopManager* Mgr = GetManager();
    const bool bActive = Mgr && Mgr->IsFocusModeActive();

    TSharedRef<SVerticalBox> Root = SNew(SVerticalBox);

    Root->AddSlot().AutoHeight().Padding(12, 12, 12, 8)
        [
            SNew(STextBlock)
                .Text(bActive
                    ? LOCTEXT("FocusModeActiveText", "Focus Mode is currently ON.")
                    : LOCTEXT("FocusModeInactiveText", "Focus Mode is currently OFF.")
                )
                .AutoWrapText(true)
        ];

    Root->AddSlot().AutoHeight().Padding(12, 0, 12, 12)
        [
            SNew(STextBlock)
                .Text(LOCTEXT("FocusModeDesc", "When ON, a quiz will pop up every interval (Project Settings > Quiz Pop)."))
                .AutoWrapText(true)
                .ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
        ];

    Root->AddSlot().AutoHeight().HAlign(HAlign_Left).Padding(12, 0, 12, 12)
        [
            SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
                [
                    SNew(SButton)
                        .ButtonColorAndOpacity(FLinearColor(0.15f, 0.5f, 0.15f))
                        .OnClicked_Lambda([Dialog, Mgr]()
                            {
                                if (Mgr) Mgr->StartFocusMode();
                                Dialog->RequestDestroyWindow();
                                return FReply::Handled();
                            })
                        [
                            SNew(STextBlock).Text(LOCTEXT("StartFocusModeBtn", "Start Focus Mode"))
                        ]
                ]
                + SHorizontalBox::Slot().AutoWidth()
                [
                    SNew(SButton)
                        .ButtonColorAndOpacity(FLinearColor(0.5f, 0.15f, 0.15f))
                        .OnClicked_Lambda([Dialog, Mgr]()
                            {
                                if (Mgr) Mgr->StopFocusMode();
                                Dialog->RequestDestroyWindow();
                                return FReply::Handled();
                            })
                        [
                            SNew(STextBlock).Text(LOCTEXT("StopFocusModeBtn", "Stop Focus Mode"))
                        ]
                ]
        ];

    Dialog->SetContent(
        SNew(SBorder).Padding(8)[Root]
    );

    FSlateApplication::Get().AddWindow(Dialog);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAttentionPopupModule, AttentionPopup)