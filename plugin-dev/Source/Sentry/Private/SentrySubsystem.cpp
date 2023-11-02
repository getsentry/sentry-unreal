// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystem.h"
#include "SentryEngineSubsystem.h"

#include "SentrySettings.h"
#include "SentryBreadcrumb.h"
#include "SentryEvent.h"
#include "SentryId.h"
#include "SentryUserFeedback.h"

#include "Engine/Engine.h"

void USentrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void USentrySubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void USentrySubsystem::Initialize()
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return;

	SentryEngineSubsystem->Initialize();
}

void USentrySubsystem::InitializeWithSettings(const FConfigureSettingsDelegate& OnConfigureSettings)
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return;

	SentryEngineSubsystem->InitializeWithSettings(OnConfigureSettings);
}

void USentrySubsystem::Close()
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return;

	SentryEngineSubsystem->Close();
}

bool USentrySubsystem::IsEnabled()
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return false;

	return SentryEngineSubsystem->IsEnabled();
}

void USentrySubsystem::AddBreadcrumb(USentryBreadcrumb* Breadcrumb)
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return;

	SentryEngineSubsystem->AddBreadcrumb(Breadcrumb);
}

void USentrySubsystem::AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FString>& Data, ESentryLevel Level)
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return;

	SentryEngineSubsystem->AddBreadcrumbWithParams(Message, Category, Type, Data, Level);
}

void USentrySubsystem::ClearBreadcrumbs()
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return;

	SentryEngineSubsystem->ClearBreadcrumbs();
}

USentryId* USentrySubsystem::CaptureMessage(const FString& Message, ESentryLevel Level)
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return nullptr;

	return SentryEngineSubsystem->CaptureMessage(Message, Level);
}

USentryId* USentrySubsystem::CaptureMessageWithScope(const FString& Message, const FConfigureScopeDelegate& OnConfigureScope, ESentryLevel Level)
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return nullptr;

	return SentryEngineSubsystem->CaptureMessageWithScope(Message, OnConfigureScope, Level);
}

USentryId* USentrySubsystem::CaptureEvent(USentryEvent* Event)
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return nullptr;

	return SentryEngineSubsystem->CaptureEvent(Event);
}

USentryId* USentrySubsystem::CaptureEventWithScope(USentryEvent* Event, const FConfigureScopeDelegate& OnConfigureScope)
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return nullptr;

	return SentryEngineSubsystem->CaptureEventWithScope(Event, OnConfigureScope);
}

void USentrySubsystem::CaptureUserFeedback(USentryUserFeedback* UserFeedback)
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return;

	SentryEngineSubsystem->CaptureUserFeedback(UserFeedback);
}

void USentrySubsystem::CaptureUserFeedbackWithParams(USentryId* EventId, const FString& Email, const FString& Comments, const FString& Name)
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return;

	SentryEngineSubsystem->CaptureUserFeedbackWithParams(EventId, Email, Comments, Name);
}

void USentrySubsystem::SetUser(USentryUser* User)
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return;

	SentryEngineSubsystem->SetUser(User);
}

void USentrySubsystem::RemoveUser()
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return;

	SentryEngineSubsystem->RemoveUser();
}

void USentrySubsystem::ConfigureScope(const FConfigureScopeDelegate& OnConfigureScope)
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return;

	SentryEngineSubsystem->ConfigureScope(OnConfigureScope);
}

void USentrySubsystem::SetContext(const FString& Key, const TMap<FString, FString>& Values)
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return;

	SentryEngineSubsystem->SetContext(Key, Values);
}

void USentrySubsystem::SetTag(const FString& Key, const FString& Value)
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return;

	SentryEngineSubsystem->SetTag(Key, Value);
}

void USentrySubsystem::RemoveTag(const FString& Key)
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return;

	SentryEngineSubsystem->RemoveTag(Key);
}

void USentrySubsystem::SetLevel(ESentryLevel Level)
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return;

	SentryEngineSubsystem->SetLevel(Level);
}

void USentrySubsystem::StartSession()
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return;

	SentryEngineSubsystem->StartSession();
}

void USentrySubsystem::EndSession()
{
	USentryEngineSubsystem* SentryEngineSubsystem = GEngine->GetEngineSubsystem<USentryEngineSubsystem>();
	if (!SentryEngineSubsystem)
		return;

	SentryEngineSubsystem->EndSession();
}