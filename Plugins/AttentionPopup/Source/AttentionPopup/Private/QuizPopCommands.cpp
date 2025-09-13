#include "QuizPopCommands.h"

#define LOCTEXT_NAMESPACE "FQuizPopCommands"

void FQuizPopCommands::RegisterCommands()
{
    UI_COMMAND(
        ToggleFocusMode,
        "Focus Mode",
        "Toggle Focus Mode (schedule surprise quizzes).",
        EUserInterfaceActionType::ToggleButton,
        FInputChord()
    );
}

#undef LOCTEXT_NAMESPACE