// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"

class IPropertyHandle;
class FSlateHyperlinkRun;
class SWidget;

class FSentrySettingsCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization implementation */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	void DrawDebugSymbolsNotice(IDetailLayoutBuilder& DetailBuilder);

	void SetPropertiesUpdateHandler(IDetailLayoutBuilder& DetailBuilder);

	TSharedRef<SWidget> MakeSentryCliDownloadStatusRow(FName IconName, FText Message, FText ButtonMessage);

	void UpdateProjectName();
	void UpdateOrganizationName();
	void UpdateAuthToken();

	void UpdatePropertiesFile(const FString& PropertyName, const FString& PropertyValue);
	void UpdateCrcConfig(const FString& Url);

	void DownloadSentryCli();

	// Gets path to CRC's DefaultEngine.ini in engine directory
	FString GetCrcConfigPath() const;
	// Gets path to Sentry CLI in plugin's ThirdParty sources directory
	FString GetSentryCliPath() const;

	FString GetSentryCliExecName() const;
	int32 GetSentryCliDownloadStatusAsInt() const;

	TSharedPtr<IPropertyHandle> ProjectNameHandle;
	TSharedPtr<IPropertyHandle> OrganizationNameHandle;
	TSharedPtr<IPropertyHandle> AuthTokenHandle;

	static const FString DefaultCrcEndpoint;
};
