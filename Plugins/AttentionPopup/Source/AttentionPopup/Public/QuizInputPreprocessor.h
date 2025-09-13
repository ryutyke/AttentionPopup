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

    // 키보드
    virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override { Notify(); return false; }
    virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override { Notify(); return false; }
    virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InEvent) override { Notify(); return false; }

    // 마우스
    virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { Notify(); return false; }
    virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { Notify(); return false; }
    virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { Notify(); return false; }
    virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& WheelEvent, const FPointerEvent* GestureEvent) override { Notify(); return false; }

private:
    void Notify()
    {
        // 과도한 호출 방지(디바운스)
        const double Now = FPlatformTime::Seconds();
        if (Now - LastFireTime >= 0.5) // 0.5초 디바운스
        {
            LastFireTime = Now;
            if (OnActivity) OnActivity();
        }
    }

private:
    TFunction<void()> OnActivity;
    double LastFireTime = 0.0;
};