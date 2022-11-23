// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"

class IPropertyHandle;
class FSlateHyperlinkRun;

class FSentrySettingsCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization implementation */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	void DrawDebugSymbolsNotice(IDetailLayoutBuilder& DetailBuilder);

	void SetPropertiesUpdateHandler(IDetailLayoutBuilder& DetailBuilder);

	void UpdateProjectName();
	void UpdateOrganizationName();
	void UpdateAuthToken();

	void UpdatePropertiesFile(const FString& PropertyName, const FString& PropertyValue);
	void UpdateCrcConfig(const FString& Url);

	// Gets path to CRC's DefaultEngine.ini in engine directory
	FString GetCrcConfigPath();

	TSharedPtr<IPropertyHandle> ProjectNameHandle;
	TSharedPtr<IPropertyHandle> OrganizationNameHandle;
	TSharedPtr<IPropertyHandle> AuthTokenHandle;

	static const FString DefaultCrcEndpoint;
};
