// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"

class FSentrySymToolsDownloader;
class IPropertyHandle;
class FSlateHyperlinkRun;
class SWidget;

enum class ESentrySettingsStatus : uint8
{
	DsnMissing = 0,
	Modified,
	Configured
};

class FSentrySettingsCustomization : public IDetailCustomization
{
public:
	FSentrySettingsCustomization();
	virtual ~FSentrySettingsCustomization();

	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization implementation */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	void DrawGeneralNotice(IDetailLayoutBuilder& DetailBuilder);
	void DrawCrashReporterNotice(IDetailLayoutBuilder& DetailBuilder);
	void DrawDebugSymbolsNotice(IDetailLayoutBuilder& DetailBuilder);

	void SetPropertiesUpdateHandler(IDetailLayoutBuilder& DetailBuilder);

	TSharedRef<SWidget> MakeGeneralSettingsStatusRow(FName IconName, FText Message, FText ButtonMessage);
	TSharedRef<SWidget> MakeSentryCliStatusRow(FName IconName, FText Message, FText ButtonMessage);

	void UpdateProjectName();
	void UpdateOrganizationName();
	void UpdateAuthToken();

	void UpdatePropertiesFile(const FString& PropertyName, const FString& PropertyValue);
	void UpdateCrcConfig(const FString& Url);

	// Gets path to CRC's DefaultEngine.ini in engine directory
	FString GetCrcConfigPath() const;

	int32 GetGeneralSettingsStatusAsInt() const;
	int32 GetSentryCliStatusAsInt() const;

	TSharedPtr<IPropertyHandle> ProjectNameHandle;
	TSharedPtr<IPropertyHandle> OrganizationNameHandle;
	TSharedPtr<IPropertyHandle> AuthTokenHandle;

	TSharedPtr<FSentrySymToolsDownloader> CliDownloader;

	static const FString DefaultCrcEndpoint;
};
