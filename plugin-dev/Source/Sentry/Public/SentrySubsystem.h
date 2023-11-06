// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "SentryDataTypes.h"
#include "SentryDefines.h"
#include "SentryScope.h"

#include "SentrySubsystem.generated.h"

class USentrySettings;
class USentryBreadcrumb;
class USentryEvent;
class USentryId;
class USentryUserFeedback;
class USentryUser;
class ISentrySubsystem;
class USentryBeforeSendHandler;

class [[deprecated("`USentrySubsystem` was deprecated and proxying calls to `USentryEngineSubsystem` which should be now used instead.")]] USentrySubsystem;

UCLASS()
class SENTRY_API USentrySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (DeprecatedFunction, DeprecationMessage="SentrySubsystem was deprecated. Please use SentryEngineSubsystem instead."))
	void Initialize();

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (DeprecatedFunction, DeprecationMessage="SentrySubsystem was deprecated. Please use SentryEngineSubsystem instead."))
	void InitializeWithSettings(const FConfigureSettingsDelegate& OnConfigureSettings);

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (DeprecatedFunction, DeprecationMessage="SentrySubsystem was deprecated. Please use SentryEngineSubsystem instead."))
	void Close();

	UFUNCTION(BlueprintPure, Category = "Sentry",
		Meta = (DeprecatedFunction, DeprecationMessage="Please use USentryEngineSubsystem"))
	bool IsEnabled();

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (DeprecatedFunction, DeprecationMessage="Please use USentryEngineSubsystem"))
	void AddBreadcrumb(USentryBreadcrumb* Breadcrumb);

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (AutoCreateRefTerm = "Data", DeprecatedFunction, DeprecationMessage="Please use USentryEngineSubsystem"))
	void AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FString>& Data,
		ESentryLevel Level = ESentryLevel::Info);

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (DeprecatedFunction, DeprecationMessage="Please use USentryEngineSubsystem"))
	void ClearBreadcrumbs();

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (DeprecatedFunction, DeprecationMessage="Please use USentryEngineSubsystem"))
	USentryId* CaptureMessage(const FString& Message, ESentryLevel Level = ESentryLevel::Info);

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (AutoCreateRefTerm = "OnCofigureScope", DeprecatedFunction, DeprecationMessage="Please use USentryEngineSubsystem"))
	USentryId* CaptureMessageWithScope(const FString& Message, const FConfigureScopeDelegate& OnConfigureScope, ESentryLevel Level = ESentryLevel::Info);

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (DeprecatedFunction, DeprecationMessage="Please use USentryEngineSubsystem"))
	USentryId* CaptureEvent(USentryEvent* Event);

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (DeprecatedFunction, DeprecationMessage="Please use USentryEngineSubsystem"))
	USentryId* CaptureEventWithScope(USentryEvent* Event, const FConfigureScopeDelegate& OnConfigureScope);

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (DeprecatedFunction, DeprecationMessage="Please use USentryEngineSubsystem"))
	void CaptureUserFeedback(USentryUserFeedback* UserFeedback);

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (DeprecatedFunction, DeprecationMessage="Please use USentryEngineSubsystem"))
	void CaptureUserFeedbackWithParams(USentryId* EventId, const FString& Email, const FString& Comments, const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (DeprecatedFunction, DeprecationMessage="Please use USentryEngineSubsystem"))
	void SetUser(USentryUser* User);

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (DeprecatedFunction, DeprecationMessage="Please use USentryEngineSubsystem"))
	void RemoveUser();

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (AutoCreateRefTerm = "OnCofigureScope", DeprecatedFunction, DeprecationMessage="Please use USentryEngineSubsystem"))
	void ConfigureScope(const FConfigureScopeDelegate& OnConfigureScope);

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (DeprecatedFunction, DeprecationMessage="Please use USentryEngineSubsystem"))
	void SetContext(const FString& Key, const TMap<FString, FString>& Values);

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (DeprecatedFunction, DeprecationMessage="Please use USentryEngineSubsystem"))
	void SetTag(const FString& Key, const FString& Value);

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (DeprecatedFunction, DeprecationMessage="Please use USentryEngineSubsystem"))
	void RemoveTag(const FString& Key);

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (DeprecatedFunction, DeprecationMessage="Please use USentryEngineSubsystem"))
	void SetLevel(ESentryLevel Level);

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (DeprecatedFunction, DeprecationMessage="Please use USentryEngineSubsystem"))
	void StartSession();

	UFUNCTION(BlueprintCallable, Category = "Sentry",
		Meta = (DeprecatedFunction, DeprecationMessage="Please use USentryEngineSubsystem"))
	void EndSession();
};
