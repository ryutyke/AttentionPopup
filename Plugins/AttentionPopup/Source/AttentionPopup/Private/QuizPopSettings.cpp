#include "QuizPopSettings.h"

const UQuizPopSettings* UQuizPopSettings::GetSettings()
{
	return GetDefault<UQuizPopSettings>();
}