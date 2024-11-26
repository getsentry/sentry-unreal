// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryCrashContext.h"

#include "SentrySettings.h"
#include "SentryModule.h"

#include "Desktop/SentryScopeDesktop.h"

#if USE_SENTRY_NATIVE

FSentryCrashContext::FSentryCrashContext(TSharedPtr<FSharedCrashContext> Context)
#if UE_VERSION_OLDER_THAN(5, 3, 0)
	: FGenericCrashContext(Context->CrashType, Context->ErrorMessage)
	, CrashContext(Context)
#else
	: CrashContext(Context)
#endif
{
}

TSharedPtr<FSentryCrashContext> FSentryCrashContext::Get()
{
	TSharedPtr<FSharedCrashContext> SharedCrashContext = MakeShareable(new FSharedCrashContext);
	FGenericCrashContext::CopySharedCrashContext(*SharedCrashContext);

	return MakeShareable(new FSentryCrashContext(SharedCrashContext));
}

void FSentryCrashContext::Apply(TSharedPtr<SentryScopeDesktop> Scope)
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	const FSessionContext& SessionContext = CrashContext->SessionContext;

	TMap<FString, FString> ContextValues;

	ContextValues.Add("Crash Type", FGenericCrashContext::GetCrashTypeString(CrashContext->CrashType));
	ContextValues.Add("IsEnsure", CrashContext->CrashType == ECrashContextType::Ensure ? TEXT("true") : TEXT("false"));
#if !UE_VERSION_OLDER_THAN(5, 0, 0)
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
	ContextValues.Add("Command Line", FString(SessionContext.CommandLine).TrimStart());
	ContextValues.Add("Memory Stats Page Size", FString::FromInt(SessionContext.MemoryStats.PageSize));
	ContextValues.Add("Memory Stats Total Virtual", FString::Printf(TEXT("%lld"), SessionContext.MemoryStats.TotalVirtual));

	if(Settings->SendDefaultPii)
	{
		ContextValues.Add("Epic Account Id", SessionContext.EpicAccountId);
		ContextValues.Add("Login Id", SessionContext.LoginIdStr);
	}

	Scope->SetContext(TEXT("Crash Info"), ContextValues);
}

FString FSentryCrashContext::GetGameData(const FString& Key)
{
	const FString* GameDataItem;

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	GameDataItem = nullptr;
#elif UE_VERSION_OLDER_THAN(5, 3, 0)
	GameDataItem = FGenericCrashContext::GetGameData(Key);
#else
	GameDataItem = FGenericCrashContext::GetGameData().Find(Key);
#endif

	return GameDataItem != nullptr ? *GameDataItem : FString();
}

#endif
