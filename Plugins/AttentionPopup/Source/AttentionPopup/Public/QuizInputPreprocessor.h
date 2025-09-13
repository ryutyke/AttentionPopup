#pragma once
#include "CoreMinimal.h"
#include "Framework/Application/IInputProcessor.h"

class FQuizInputPreprocessor : public IInputProcessor
{
public:
    explicit FQuizInputPreprocessor(TFunction<void()> InOnActivity)
        : OnActivity(MoveTemp(InOnActivity)) {
    }

    virtual const TCHAR* GetDebugName() const override { return TEXT("FQuizInputPreprocessor"); }

    virtual void Tick(const float /*DeltaTime*/, FSlateApplication& /*SlateApp*/, TSharedRef<ICursor> /*Cursor*/) override {}

    // Ű����
    virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override { Notify(); return false; }
    virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override { Notify(); return false; }
    virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InEvent) override { Notify(); return false; }

    // ���콺
    virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { Notify(); return false; }
    virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { Notify(); return false; }
    virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { Notify(); return false; }
    virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& WheelEvent, const FPointerEvent* GestureEvent) override { Notify(); return false; }

private:
    void Notify()
    {
        // ������ ȣ�� ����(��ٿ)
        const double Now = FPlatformTime::Seconds();
        if (Now - LastFireTime >= 0.5) // 0.5�� ��ٿ
        {
            LastFireTime = Now;
            if (OnActivity) OnActivity();
        }
    }

private:
    TFunction<void()> OnActivity;
    double LastFireTime = 0.0;
};