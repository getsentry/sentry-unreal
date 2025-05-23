// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryImplWrapper.h"

#include "SentryAttachment.generated.h"

class ISentryAttachment;

/**
 * Additional file to store alongside an event or transaction.
 */
UCLASS(BlueprintType, NotBlueprintable, HideDropdown)
class SENTRY_API USentryAttachment : public UObject, public TSentryImplWrapper<ISentryAttachment, USentryAttachment>
{
	GENERATED_BODY()

public:
	/**
	 * Initializes an attachment with bytes and a filename.
	 *
	 * @param Data The data for the attachment.
	 * @param Filename The name of the attachment to display in Sentry.
	 * @param ContentType The content type of the attachment. Default is "application/octet-stream".
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void InitializeWithData(const TArray<uint8>& Data, const FString& Filename, const FString& ContentType = FString(TEXT("application/octet-stream")));

	/**
	 * Initializes an attachment with a path and a filename.
	 *
	 * @param Path The path string of the file to upload as an attachment.
	 * @param Filename The name of the attachment to display in Sentry.
	 * @param ContentType The content type of the attachment. Default is "application/octet-stream".
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void InitializeWithPath(const FString& Path, const FString& Filename, const FString& ContentType = FString(TEXT("application/octet-stream")));

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
