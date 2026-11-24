// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryPlaygroundTestUtils.h"

#include "SentryPlayground/SentryPlayground.h"

#include "HAL/PlatformMemory.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#endif

void USentryPlaygroundTestUtils::SetMemoryLimit(int32 HeadroomMB)
{
#if PLATFORM_WINDOWS
	const uint64 CurrentCommitBytes = FPlatformMemory::GetStats().UsedVirtual;
	const uint64 LimitBytes = CurrentCommitBytes + (uint64)HeadroomMB * 1024 * 1024;

	// Unnamed to avoid collisions with stale job objects from previous runs
	HANDLE JobObject = ::CreateJobObject(nullptr, nullptr);
	if (!JobObject)
	{
		UE_LOG(LogSentrySample, Warning, TEXT("SetMemoryLimit: CreateJobObject failed (error %u)"), static_cast<uint32>(::GetLastError()));
		return;
	}

	JOBOBJECT_EXTENDED_LIMIT_INFORMATION JobLimitInfo;
	FMemory::Memzero(JobLimitInfo);
	JobLimitInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_PROCESS_MEMORY;
	JobLimitInfo.ProcessMemoryLimit = (SIZE_T)LimitBytes;

	if (!::SetInformationJobObject(JobObject, JobObjectExtendedLimitInformation, &JobLimitInfo, sizeof(JobLimitInfo)))
	{
		UE_LOG(LogSentrySample, Warning, TEXT("SetMemoryLimit: SetInformationJobObject failed (error %u)"), static_cast<uint32>(::GetLastError()));
		::CloseHandle(JobObject);
		return;
	}

	if (!::AssignProcessToJobObject(JobObject, ::GetCurrentProcess()))
	{
		UE_LOG(LogSentrySample, Warning, TEXT("SetMemoryLimit: AssignProcessToJobObject failed (error %u)"), static_cast<uint32>(::GetLastError()));
		::CloseHandle(JobObject);
		return;
	}

	// The job handle is intentionally left open so the limit persists for the process lifetime
	UE_LOG(LogSentrySample, Log, TEXT("SetMemoryLimit: process commit capped at %llu MB (current usage %llu MB + %d MB headroom)"),
		LimitBytes / (1024 * 1024), CurrentCommitBytes / (1024 * 1024), HeadroomMB);
#endif
}
