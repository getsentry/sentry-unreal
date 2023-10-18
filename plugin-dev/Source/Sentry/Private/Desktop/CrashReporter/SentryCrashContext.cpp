﻿// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryCrashContext.h"

#include "SentrySettings.h"
#include "SentryModule.h"

#include "Desktop/SentryScopeDesktop.h"

#if USE_SENTRY_NATIVE

FSentryCrashContext::FSentryCrashContext(TSharedPtr<FSharedCrashContext> Context)
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
	: CrashContext(Context)
#else
	: FGenericCrashContext(Context->CrashType, Context->ErrorMessage)
	, CrashContext(Context)
#endif
{
}

FSentryCrashContext FSentryCrashContext::Get()
{
	TSharedPtr<FSharedCrashContext> SharedCrashContext = MakeShareable(new FSharedCrashContext);
	FGenericCrashContext::CopySharedCrashContext(*SharedCrashContext);

	return FSentryCrashContext(SharedCrashContext);
}

void FSentryCrashContext::Apply(TSharedPtr<SentryScopeDesktop> Scope)
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	const FSessionContext& SessionContext = CrashContext->SessionContext;

	TMap<FString, FString> ContextValues;

	ContextValues.Add("Crash Type", FGenericCrashContext::GetCrashTypeString(CrashContext->CrashType));
	ContextValues.Add("IsEnsure", CrashContext->CrashType == ECrashContextType::Ensure ? TEXT("true") : TEXT("false"));
#if ENGINE_MAJOR_VERSION >= 5
	ContextValues.Add("IsStall", CrashContext->CrashType == ECrashContextType::Stall ? TEXT("true") : TEXT("false"));
#endif
	ContextValues.Add("IsAssert", CrashContext->CrashType == ECrashContextType::Assert ? TEXT("true") : TEXT("false"));
	ContextValues.Add("Crashing Thread Id", FString::FromInt(CrashContext->CrashingThreadId));
	ContextValues.Add("App Default Locale", SessionContext.DefaultLocale);
	ContextValues.Add("Language LCID", FString::FromInt(SessionContext.LanguageLCID));
	ContextValues.Add("Base Dir", SessionContext.BaseDir);
	ContextValues.Add("Is Source Distribution", SessionContext.bIsSourceDistribution ? TEXT("true") : TEXT("false"));
	ContextValues.Add("Crash GUID", SessionContext.CrashGUIDRoot);
	ContextValues.Add("Executable Name", SessionContext.ExecutableName);
	ContextValues.Add("Game Name", SessionContext.GameName);
	ContextValues.Add("Process Id", FString::FromInt(SessionContext.ProcessId));
	ContextValues.Add("Seconds Since Start", FString::FromInt(SessionContext.SecondsSinceStart));
	ContextValues.Add("Command Line", SessionContext.CommandLine);
	ContextValues.Add("Memory Stats Page Size", FString::FromInt(SessionContext.MemoryStats.PageSize));
	ContextValues.Add("Memory Stats Total Virtual", FString::Printf(TEXT("%lld"), SessionContext.MemoryStats.TotalVirtual));

	if(Settings->SendDefaultPii)
	{
		ContextValues.Add("Epic Account Id", SessionContext.EpicAccountId);
		ContextValues.Add("Login Id", SessionContext.LoginIdStr);
	}

	Scope->SetContext(TEXT("Additional Crash Info"), ContextValues);
}

FString FSentryCrashContext::GetGameData(const FString& Key)
{
	const FString* GameDataItem = nullptr;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
	GameDataItem = FGenericCrashContext::GetGameData().Find(Key);
#else
	GameDataItem = FGenericCrashContext::GetGameData(Key);
#endif

	return GameDataItem != nullptr ? *GameDataItem : FString();
}

#endif
