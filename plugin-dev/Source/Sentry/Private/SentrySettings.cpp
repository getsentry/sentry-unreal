// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySettings.h"

USentrySettings::USentrySettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InitAutomatically(false)
	, UploadSymbolsAutomatically(false)
{
	DsnUrl = TEXT("");
	Release = TEXT("");
}

#if WITH_EDITOR
void USentrySettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName Name = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (Name == FName(TEXT("PropertiesFilePath")) && !PropertiesFilePath.FilePath.IsEmpty())
	{
		PropertiesFilePath.FilePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*PropertiesFilePath.FilePath);
		SaveConfig(CPF_Config, *GetDefaultConfigFilename());
	}
}
#endif
