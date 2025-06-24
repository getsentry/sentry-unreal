// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "SentryDataTypes.h"
#include "SentryVariant.h"

#include "SentryLibrary.generated.h"

class USentryTransactionContext;
class USentryEvent;
class USentryBreadcrumb;
class USentryUser;
class USentryUserFeedback;
class USentryAttachment;

/**
 * Utility functions for Sentry.
 */
UCLASS()
class SENTRY_API USentryLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Creates the event.
	 *
	 * @param Message Message to sent.
	 * @param Level Level of the event.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static USentryEvent* CreateSentryEvent(const FString& Message, ESentryLevel Level);

	/**
	 * Creates user.
	 *
	 * @param Email Email address of the user.
	 * @param Id Id of the user.
	 * @param Username Username of the user.
	 * @param IpAddress IP address of the user.
	 * @param Data Additional arbitrary fields related to the user.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry", Meta = (AutoCreateRefTerm = "Data"))
	static USentryUser* CreateSentryUser(const FString& Email, const FString& Id, const FString& Username, const FString& IpAddress,
		const TMap<FString, FString>& Data);

	/**
	 * Creates user feedback for the event.
	 *
	 * @param EventId Id of the event to which user feedback is associated.
	 * @param Name Name of the user.
	 * @param Email Email of the user.
	 * @param Comments Comments of the user about what happened.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static USentryUserFeedback* CreateSentryUserFeedback(const FString& EventId, const FString& Name, const FString& Email, const FString& Comments);

	/**
	 * Creates breadcrumb.
	 *
	 * @param Message Message of the breadcrumb.
	 * @param Type Type of the breadcrumb.
	 * @param Category Category of the breadcrumb.
	 * @param Data Data associated with the breadcrumb.
	 * @param Level Level of the breadcrumb.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry", Meta = (AutoCreateRefTerm = "Data"))
	static USentryBreadcrumb* CreateSentryBreadcrumb(const FString& Message, const FString& Type, const FString& Category,
		const TMap<FString, FSentryVariant>& Data, ESentryLevel Level = ESentryLevel::Info);

	/**
	 * Creates attachment with bytes and a filename.
	 *
	 * @param Data The data for the attachment.
	 * @param Filename The name of the attachment to display in Sentry.
	 * @param ContentType The content type of the attachment. Default is "application/octet-stream".
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static USentryAttachment* CreateSentryAttachmentWithData(const TArray<uint8>& Data, const FString& Filename,
		const FString& ContentType = FString(TEXT("application/octet-stream")));

	/**
	 * Creates attachment with a path and a filename.
	 *
	 * @param Path The path string of the file to upload as an attachment.
	 * @param Filename The name of the attachment to display in Sentry.
	 * @param ContentType The content type of the attachment. Default is "application/octet-stream".
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static USentryAttachment* CreateSentryAttachmentWithPath(const FString& Path, const FString& Filename,
		const FString& ContentType = FString(TEXT("application/octet-stream")));

	/**
	 * Creates transaction context.
	 *
	 * @param Name Transaction name.
	 * @param Operation Transaction operation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static USentryTransactionContext* CreateSentryTransactionContext(const FString& Name, const FString& Operation);
};
