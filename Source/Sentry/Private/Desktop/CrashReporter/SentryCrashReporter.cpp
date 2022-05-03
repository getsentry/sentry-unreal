// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryCrashReporter.h"

#include "SentryUser.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"

#include "GenericPlatform/GenericPlatformCrashContext.h"

SentryCrashReporter::SentryCrashReporter()
{
	crashReporterConfig = MakeShareable(new FJsonObject);
}

void SentryCrashReporter::SetUser(USentryUser* user)
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

void SentryCrashReporter::RemoveUser()
{
	crashReporterConfig->RemoveField(TEXT("user"));

	UpdateCrashReporterConfig();
}

void SentryCrashReporter::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	TSharedPtr<FJsonObject> valuesConfig = MakeShareable(new FJsonObject);

	for (auto it = values.CreateConstIterator(); it; ++it)
	{
		valuesConfig->SetStringField(it.Key(), it.Value());
	}

	TSharedPtr<FJsonObject> contextConfig = MakeShareable(new FJsonObject);
	contextConfig->SetObjectField(key, valuesConfig);

	crashReporterConfig->SetObjectField(TEXT("contexts"), contextConfig);

	UpdateCrashReporterConfig();
}

void SentryCrashReporter::SetTag(const FString& key, const FString& value)
{
	TSharedPtr<FJsonObject> tagsConfig;

	if(crashReporterConfig->HasField(TEXT("tags")))
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

void SentryCrashReporter::RemoveTag(const FString& key)
{
	TSharedPtr<FJsonObject> tagsConfig;

	if(crashReporterConfig->HasField(TEXT("tags")))
	{
		tagsConfig = crashReporterConfig->GetObjectField(TEXT("tags"));
		tagsConfig->RemoveField(key);
	}

	UpdateCrashReporterConfig();
}

void SentryCrashReporter::UpdateCrashReporterConfig()
{
	FString config;
	TSharedRef<TJsonWriter<>> jsonWriter = TJsonWriterFactory<>::Create(&config);
	FJsonSerializer::Serialize(crashReporterConfig.ToSharedRef(), jsonWriter);

	FGenericCrashContext::SetGameData(TEXT("__sentry"), config);
}
