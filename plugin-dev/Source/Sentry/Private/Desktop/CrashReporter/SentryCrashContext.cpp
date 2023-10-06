// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryCrashContext.h"

#include "SentrySettings.h"
#include "SentryModule.h"

#include "Desktop/SentryScopeDesktop.h"

#if USE_SENTRY_NATIVE

FSentryCrashContext FSentryCrashContext::Get()
{
	FSharedCrashContext SharedCrashContext;
	FGenericCrashContext::CopySharedCrashContext(SharedCrashContext);

	FSentryCrashContext SentryCrashContext;
	SentryCrashContext.CrashContext = SharedCrashContext;

	return SentryCrashContext;
}

void FSentryCrashContext::Apply(TSharedPtr<SentryScopeDesktop> Scope)
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	const FSessionContext& SessionContext = CrashContext.SessionContext;

	Scope->SetExtraValue("Crash Type", FGenericCrashContext::GetCrashTypeString(CrashContext.CrashType));
	Scope->SetExtraValue("IsEnsure", CrashContext.CrashType == ECrashContextType::Ensure ? TEXT("true") : TEXT("false"));
#if ENGINE_MAJOR_VERSION >= 5
	Scope->SetExtraValue("IsStall", CrashContext.CrashType == ECrashContextType::Stall ? TEXT("true") : TEXT("false"));
#endif
	Scope->SetExtraValue("IsAssert", CrashContext.CrashType == ECrashContextType::Assert ? TEXT("true") : TEXT("false"));
	Scope->SetExtraValue("Crashing Thread Id", FString::FromInt(CrashContext.CrashingThreadId));
	Scope->SetExtraValue("App Default Locale", SessionContext.DefaultLocale);
	Scope->SetExtraValue("Language LCID", FString::FromInt(SessionContext.LanguageLCID));
	Scope->SetExtraValue("Base Dir", SessionContext.BaseDir);
	Scope->SetExtraValue("Is Source Distribution", SessionContext.bIsSourceDistribution ? TEXT("true") : TEXT("false"));
	Scope->SetExtraValue("Crash GUID", SessionContext.CrashGUIDRoot);
	Scope->SetExtraValue("Executable Name", SessionContext.ExecutableName);
	Scope->SetExtraValue("Game Name", SessionContext.GameName);
	Scope->SetExtraValue("Process Id", FString::FromInt(SessionContext.ProcessId));
	Scope->SetExtraValue("Seconds Since Start", FString::FromInt(SessionContext.SecondsSinceStart));
	Scope->SetExtraValue("Command Line", SessionContext.CommandLine);
	Scope->SetExtraValue("Memory Stats Page Size", FString::FromInt(SessionContext.MemoryStats.PageSize));
	Scope->SetExtraValue("Memory Stats Total Virtual", FString::Printf(TEXT("%lld"), SessionContext.MemoryStats.TotalVirtual));

	if(Settings->SendDefaultPii)
	{
		Scope->SetExtraValue("Epic Account Id", SessionContext.EpicAccountId);
		Scope->SetExtraValue("Login Id", SessionContext.LoginIdStr);
	}
}

FString FSentryCrashContext::GetGameData(const FString& Key)
{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
	const FString* GameDataItem = FGenericCrashContext::GetGameData().Find(Key);
	return GameDataItem != nullptr ? *GameDataItem : FString();
#else
	return CrashContext.GetGameData(Key);
#endif
}

#endif
