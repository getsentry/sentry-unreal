// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentryCrashReporter.h"

#include "GenericPlatform/CrashReporter/GenericPlatformSentryCrashContext.h"
#include "GenericPlatform/GenericPlatformSentryUser.h"
#include "GenericPlatform/Infrastructure/GenericPlatformSentryConverters.h"

#include "SentryDefines.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"

#if USE_SENTRY_NATIVE

FGenericPlatformSentryCrashReporter::FGenericPlatformSentryCrashReporter()
{
	crashReporterConfig = MakeShareable(new FJsonObject);

	const FString sentryData = FGenericPlatformSentryCrashContext::Get()->GetGameData(TEXT("__sentry"));
	if (!sentryData.IsEmpty())
	{
		const TSharedRef<TJsonReader<>> jsonReader = TJsonReaderFactory<>::Create(*sentryData);
		if (!FJsonSerializer::Deserialize(jsonReader, crashReporterConfig) && crashReporterConfig.IsValid())
		{
			UE_LOG(LogSentrySdk, Log, TEXT("Faield to deserialize `__sentry` data stored in crash context object: %s"), *FString(sentryData));
		}
	}
}

void FGenericPlatformSentryCrashReporter::SetRelease(const FString& release)
{
	if (!release.IsEmpty())
		crashReporterConfig->SetStringField(TEXT("release"), release);

	UpdateCrashReporterConfig();
}

void FGenericPlatformSentryCrashReporter::SetEnvironment(const FString& environment)
{
	if (!environment.IsEmpty())
		crashReporterConfig->SetStringField(TEXT("environment"), environment);

	UpdateCrashReporterConfig();
}

void FGenericPlatformSentryCrashReporter::SetUser(TSharedPtr<FGenericPlatformSentryUser> user)
{
	TSharedPtr<FJsonObject> userConfig = MakeShareable(new FJsonObject);
	userConfig->SetStringField(TEXT("email"), user->GetEmail());
	userConfig->SetStringField(TEXT("username"), user->GetUsername());
	userConfig->SetStringField(TEXT("id"), user->GetId());
	userConfig->SetStringField(TEXT("ip_address"), user->GetIpAddress());

	const TMap<FString, FString>& userData = user->GetData();

	for (auto it = userData.CreateConstIterator(); it; ++it)
	{
		userConfig->SetStringField(it.Key(), it.Value());
	}

	crashReporterConfig->SetObjectField(TEXT("user"), userConfig);

	UpdateCrashReporterConfig();
}

void FGenericPlatformSentryCrashReporter::RemoveUser()
{
	crashReporterConfig->RemoveField(TEXT("user"));

	UpdateCrashReporterConfig();
}

void FGenericPlatformSentryCrashReporter::SetContext(const FString& key, const TMap<FString, FSentryVariant>& values)
{
	TSharedPtr<FJsonObject> valuesConfig = MakeShareable(new FJsonObject);

	for (auto it = values.CreateConstIterator(); it; ++it)
	{
		valuesConfig->Values.Add(it.Key(), FGenericPlatformSentryConverters::VariantToJsonValue(it.Value()));
	}

	TSharedPtr<FJsonObject> contextConfig;

	if (crashReporterConfig->HasField(TEXT("contexts")))
	{
		contextConfig = crashReporterConfig->GetObjectField(TEXT("contexts"));
		contextConfig->SetObjectField(key, valuesConfig);
	}
	else
	{
		contextConfig = MakeShareable(new FJsonObject);
		contextConfig->SetObjectField(key, valuesConfig);
		crashReporterConfig->SetObjectField(TEXT("contexts"), contextConfig);
	}

	crashReporterConfig->SetObjectField(TEXT("contexts"), contextConfig);

	UpdateCrashReporterConfig();
}

void FGenericPlatformSentryCrashReporter::SetTag(const FString& key, const FString& value)
{
	TSharedPtr<FJsonObject> tagsConfig;

	if (crashReporterConfig->HasField(TEXT("tags")))
	{
		tagsConfig = crashReporterConfig->GetObjectField(TEXT("tags"));
		tagsConfig->SetStringField(key, value);
	}
	else
	{
		tagsConfig = MakeShareable(new FJsonObject);
		tagsConfig->SetStringField(key, value);
		crashReporterConfig->SetObjectField(TEXT("tags"), tagsConfig);
	}

	UpdateCrashReporterConfig();
}

void FGenericPlatformSentryCrashReporter::RemoveTag(const FString& key)
{
	TSharedPtr<FJsonObject> tagsConfig;

	if (crashReporterConfig->HasField(TEXT("tags")))
	{
		tagsConfig = crashReporterConfig->GetObjectField(TEXT("tags"));
		tagsConfig->RemoveField(key);
	}

	UpdateCrashReporterConfig();
}

void FGenericPlatformSentryCrashReporter::UpdateCrashReporterConfig()
{
	FString config;
	TSharedRef<TJsonWriter<>> jsonWriter = TJsonWriterFactory<>::Create(&config);
	FJsonSerializer::Serialize(crashReporterConfig.ToSharedRef(), jsonWriter);

	FGenericCrashContext::SetGameData(TEXT("__sentry"), config);
}

#endif