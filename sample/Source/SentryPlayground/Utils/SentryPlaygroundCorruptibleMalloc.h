// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "HAL/MemoryBase.h"

// Forwarding allocator that faults on matching allocations once armed, to
// reproduce crashes handled while the allocator is corrupted.
class FSentryCorruptibleMalloc final : public FMalloc
{
public:
	static void ArmForNextCrash();

	explicit FSentryCorruptibleMalloc(FMalloc* InInner)
		: Inner(InInner)
	{
	}

	virtual void* Malloc(SIZE_T Count, uint32 Alignment) override
	{
		FaultIfArmed(Count);
		return Inner->Malloc(Count, Alignment);
	}

	virtual void* Realloc(void* Original, SIZE_T Count, uint32 Alignment) override
	{
		FaultIfArmed(Count);
		return Inner->Realloc(Original, Count, Alignment);
	}

	virtual void Free(void* Original) override { Inner->Free(Original); }

	virtual bool GetAllocationSize(void* Original, SIZE_T& SizeOut) override
	{
		return Inner->GetAllocationSize(Original, SizeOut);
	}

	virtual SIZE_T QuantizeSize(SIZE_T Count, uint32 Alignment) override
	{
		return Inner->QuantizeSize(Count, Alignment);
	}

	virtual bool IsInternallyThreadSafe() const override { return Inner->IsInternallyThreadSafe(); }

	virtual void Trim(bool bTrimThreadCaches) override { Inner->Trim(bTrimThreadCaches); }

	virtual const TCHAR* GetDescriptiveName() override { return TEXT("SentryCorruptibleMalloc"); }

private:
	void FaultIfArmed(SIZE_T Count)
	{
		if (bArmed && Count >= 128 && Count <= 1024)
		{
			*reinterpret_cast<volatile int*>(0) = 0xDEAD;
		}
	}

	FMalloc* Inner;
	bool bArmed = false;
};
