// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryImplWrapper.h"

#include "SentryAttachment.generated.h"

class ISentryAttachment;

/**
 * Additional file to store alongside an event or transaction.
 */
UCLASS(BlueprintType)
class SENTRY_API USentryAttachment : public UObject, public TSentryImplWrapper<ISentryAttachment, USentryAttachment>
{
	GENERATED_BODY()

public:
	/** Gets the bytes of the attachment. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	TArray<uint8> GetData() const;

	/** Gets the path string of the attachment. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetPath() const;

	/** Gets the name of the attachment to display in Sentry. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetFilename() const;

	/** Gets the content type of the attachment. Default is "application/octet-stream". */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetContentType() const;
};
