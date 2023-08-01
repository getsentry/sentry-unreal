#include "SentryCrashContext.h"

#include "Desktop/SentryScopeDesktop.h"

FSentryCrashContext::FSentryCrashContext(const FSharedCrashContext& Context)
	: FGenericCrashContext(Context.CrashType, Context.ErrorMessage)
	, CrashContext(Context)
{
}

void FSentryCrashContext::Apply(TSharedPtr<SentryScopeDesktop> Scope)
{
	const FSessionContext& SessionContext = CrashContext.SessionContext;

	Scope->SetExtraValue("Crash Type", GetCrashTypeString(CrashContext.CrashType));
	Scope->SetExtraValue("Crashing Thread Id", FString::FromInt(CrashContext.CrashingThreadId));
	Scope->SetExtraValue("App Default Locale", SessionContext.DefaultLocale);
	Scope->SetExtraValue("Language LCID", FString::FromInt(SessionContext.LanguageLCID));
	Scope->SetExtraValue("Base Dir", SessionContext.BaseDir);
	Scope->SetExtraValue("Is Source Distribution", SessionContext.bIsSourceDistribution ? TEXT("true") : TEXT("false"));
	Scope->SetExtraValue("Crash GUID", SessionContext.CrashGUIDRoot);
	Scope->SetExtraValue("Epic Account Id", SessionContext.EpicAccountId);
	Scope->SetExtraValue("Login Id", SessionContext.LoginIdStr);
	Scope->SetExtraValue("Executable Name", SessionContext.ExecutableName);
	Scope->SetExtraValue("Game Name", SessionContext.GameName);
	Scope->SetExtraValue("CPU Brand", SessionContext.CPUBrand);
	Scope->SetExtraValue("CPU Vendor", SessionContext.CPUVendor);
	Scope->SetExtraValue("Number of Cores", FString::FromInt(SessionContext.NumberOfCores));
	Scope->SetExtraValue("Number of Cores including Hyperthreads", FString::FromInt(SessionContext.NumberOfCoresIncludingHyperthreads));	
	Scope->SetExtraValue("Process Id", FString::FromInt(SessionContext.ProcessId));
	Scope->SetExtraValue("Seconds Since Start", FString::FromInt(SessionContext.SecondsSinceStart));
	Scope->SetExtraValue("Command Line", SessionContext.CommandLine);
	Scope->SetExtraValue("Memory Stats Page Size", FString::FromInt(SessionContext.MemoryStats.PageSize));
	Scope->SetExtraValue("Memory Stats Total Physical GB", FString::FromInt(SessionContext.MemoryStats.TotalPhysicalGB));
	Scope->SetExtraValue("Memory Stats Total Virtual", FString::Printf(TEXT("%lld"), SessionContext.MemoryStats.TotalVirtual));

	TMap<FString, FString> GpuContext;
	GpuContext.Add(TEXT("name"), *GetEngineData(TEXT("RHI.AdapterName")));
	GpuContext.Add(TEXT("vendor_name"), *GetEngineData(TEXT("RHI.GPUVendor")));
	GpuContext.Add(TEXT("graphics_shader_level"), *GetEngineData(TEXT("RHI.FeatureLevel")));

	Scope->SetContext(TEXT("gpu"), GpuContext);
}